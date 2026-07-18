# Teaching an old 8-bit C compiler new tricks

### How the OSDK C toolchain was overhauled for version 1.24 — the optimisations, the philosophy, and the bugs found along the way

---

The [OSDK](https://osdk.org) (Oric Software Development Kit) lets you write C for the
Oric, an 8-bit home computer built around the MOS 6502. Its C compiler descends from
the venerable **LCC**, wired to a 6502 backend, a macro-based assembler layer, and a
small C runtime. Some of that code is thirty years old. Much of it had never been
exercised hard — most real Oric programs allocate memory statically, avoid floating
point, and lean on hand-written assembler for the hot paths, so entire corners of the
toolchain had quietly rotted without anyone noticing.

Version 1.24 is the result of a long push to change that: to make the compiler generate
genuinely tight 6502 code, to repair library functions that had been subtly broken for
years, and — crucially — to do all of it *without ever shipping a program that computes
the wrong answer*. This article walks through what changed and, just as importantly, the
bugs that were dragged into the light on the way.

If there is one theme, it is this: **on a machine this small, correctness and size are
the same discipline.** A byte saved is a byte you don't have; a wrong byte is a crash.

---

## The pipeline, so the rest makes sense

A `.c` file becomes an Oric `.tap` through a chain of tools, and it helps to know which
tool owns which problem:

```
  file.c
    │  mcpp                 C preprocessor (macros, #include)
    ▼
  compiler.exe (LCC)        parses C, emits 6502 code in "macro form"
    │                       (ADDW, CZBW, ASGNW_DD … — one macro per operation)
    ▼
  macrosplitter            expands the macros to real 6502, then runs a
    │  (-M expand, -O peep) peephole optimiser over the result
    ▼
  link65                    resolves labels, pulls in referenced library code
    │
    ▼
  xa                        6502 assembler → machine code
    │
    ▼
  file.tap
```

Two design decisions in this chain matter for everything below:

1. **The compiler doesn't emit raw 6502. It emits *macros*** — an intermediate form
   where `a = b + c` on 16-bit ints becomes something like `ADDW_DDD(c,b,a)`. The
   `macrosplitter` then expands each macro to actual instructions. This is why there are
   two places to make code smaller: teach the *compiler* to pick a cheaper macro, or
   teach the *macrosplitter* to clean up the expanded result.

2. **The peephole optimiser only ever runs on compiler-generated code.** Hand-written
   assembler — the libraries, and any `.s` file a user writes — is copied through
   verbatim and never touched. This is a hard rule, and a deliberate one: hand assembler
   can contain self-modifying code, hand-tuned templates, and hardware I/O sequences
   whose "redundant" loads are anything but. The optimiser has no business second-guessing
   a human who wrote 6502 on purpose. (A *reporting* mode that flags possible improvements
   in hand code without applying them is a plausible future feature — but it would only
   ever suggest, never rewrite.)

The 6502-only focus is worth stating too. LCC is a *retargetable* compiler built around
an abstract machine; the OSDK fork abandons that abstraction entirely. It targets the
6502 (and possibly the 65816), full stop. That freedom is what licenses many of the
optimisations below — they'd be unsound on some hypothetical architecture, but the OSDK
compiler will never run on one.

---

## Part 1 — Making the compiler count in bytes

### Native 8-bit arithmetic: the big one

C has a rule that bites hard on an 8-bit machine: **the *integer promotions*.** In an
expression, a `char` is promoted to `int` before almost any operation. So this:

```c
unsigned char a, b, c;
c = a + b;
```

is, by the letter of the standard, "widen `a` to 16 bits, widen `b` to 16 bits, add them
as 16-bit integers, then store the low 8 bits into `c`." The old compiler did exactly
that, faithfully and wastefully: two zero-extensions, a 16-bit `ADDW`, and a narrowing
store — roughly twice the code and cycles that a single-byte add needs. And byte
arithmetic is *everywhere* in real Oric code.

The fix is a codegen pre-pass, `mark_byte_narrowing`, that recognises the tell-tale
shape the front end produces — a byte operation wrapped in "widen the operands, then
narrow the result back to char" — and, when the result is only ever used as a char,
flags the operation to emit a **byte-width macro** instead. A new family of macros
(`ADDB`, `SUBB`, `ANDB`, `ORB`, `XORB` — the low-byte halves of the existing word
families) does the work in a single pass over one byte each.

This is *exact*, not approximate. For `+ - & | ^`, the low byte of the result depends
only on the low bytes of the operands, so 8-bit wraparound comes out identical to the
16-bit-then-truncate version: `200 + 100` gives `44`, `0 - 1` gives `255`, just as C
requires. And detection is conservative — if a value is *also* used somewhere at full
16-bit width, it keeps its widening path. Pure `int` code (`k = i + j`) is byte-for-byte
unchanged.

The work landed in four phases:

- **Phase 1** — `+ - & | ^`, and `++`/`--` (which are just `c = c ± 1`).
- **Phase 2** — `~x` (`COMB`), unary `-x` (`NEGB`), and `x << 1` (`LSH1B`).
- **Phase 3** — comparisons: `== != < > <= >=`.
- **Phase 4** — `x >> 1` (`RSH1B`).

Two of those phases hide a genuine correctness subtlety worth calling out, because they
are exactly the kind of thing a naive "just use the byte version" would get wrong:

**Comparisons must stay unsigned.** A `unsigned char` is always in `0..255`. After
promotion, comparing two of them is an *unsigned* 16-bit compare. The byte forms
(`LTUB`, `GTUB`, …) are therefore the unsigned byte compares — because a *signed* byte
compare would treat `0x80..0xFF` as negative and get the wrong answer for exactly half
the range. Only unsigned-char widenings qualify for narrowing; signed and mixed-width
compares stay on the word path. The big winner is the ubiquitous
`for (unsigned char i = 0; i < n; i++)`.

**Right shift must stay logical.** `0x80 >> 1` is `0x40`, not `0xC0`. Emitting `lsr`
(logical) is correct precisely *because* only unsigned operands are narrowed — the top
bit is data, never a sign bit. (Signed right shift is a separate story; see below.)

#### Eliding the dead widen

Narrowing the *operation* is only half the win. The operands still arrive with their
zero-extension (`CZBW`, "clear the high byte") attached — and once the op is a byte op,
that high byte is dead. So a companion analysis drops the zero-extend whenever the
widened temporary is proven to be consumed *only* by byte operations before it's next
overwritten.

That liveness test is deliberately run on the *physical* temporaries, after register
allocation, because several logical values can share one physical slot. The canonical
trap is `while (i--)`: the loaded `i` feeds both the byte decrement *and* a 16-bit
"is it zero?" test. Its widen must be **kept** — dropping it leaves a garbage high byte
and the loop never terminates. (It briefly did exactly that during development, which is
how the importance of physical-slot liveness got learned the hard way.)

At `-O3` the analysis goes one step further and fuses the now-byte-only load into the
widen slot, emitting a byte-only load (`CWB`, the word→byte family). The net effect is
that char-heavy code at `-O3` is finally no slower than at `-O2` — on the AES-256
benchmark, `-O3` dropped from 58.1M cycles to 50.9M, at last landing *below* its own
`-O2`.

### Shift by 8 is a byte move, not eight shifts

Here's one that looks like nothing and mattered a lot. `x << 8`, `x >> 8` — and, after
strength reduction, `x * 256`, `x / 256`, `x % 256` — compiled to the *generic
variable-count shift routine*, which loops eight times doing `asl/rol` (or `lsr/ror`) one
bit per iteration. That's 100-plus cycles to accomplish what is really a **one-byte
move**: the low byte becomes the high byte (or vice versa) and the other byte is zeroed.

These patterns are everywhere in 8-bit code — splitting a 16-bit address into its high
and low bytes, or grabbing the integer part of an 8.8 fixed-point value. A constant shift
count of exactly 8 now emits a byte-move macro (`LSHW8`, `RSHW8` logical, `ASRW8`
arithmetic with a branchless sign fill). Roughly 10-14 cycles instead of ~100, and no
shared loop routine dragged into the binary. It fired across nearly every benchmark.

*(Why stop at 8 and not generalise to counts 9-15, or use rotate tricks for large
shifts? Because those are rare in practice and the complexity — especially the 6502's
9-bit rotate-through-carry — isn't worth it. The common, high-value case is the byte
move; that's what got built.)*

### In-place idioms: `inc`, `dec`, `asl`, `lsr`

The 6502 can increment, decrement and shift a memory location directly. When a byte
operation's result *is* its operand — `c++`, `c--`, `c <<= 1`, `c >>= 1` — the compiler
now emits the single read-modify-write instruction (`inc c`, `dec c`, `asl c`, `lsr c`)
instead of the general load / operate / store sequence. A `char` counter that used to
cost five instructions (`clc / lda c / adc #1 / sta c`) now costs one. It mirrors a
special case the word path already had (`INCW`/`DECW`), and because the byte-narrowing is
itself the gate, it applies at every optimisation level.

---

## Part 2 — The bug hunts

Optimisation work has a way of turning over rocks. Several of the nastiest bugs in this
release were *not* new — they were latent for years and only surfaced because the new
test suite finally exercised the code that tripped them.

### The AES-256 "-O3 crashes" saga, and the borrowed-temporary family

For a long time there was a standing report: the AES-256 sample crashed at `-O3`. A
multi-day hunt chased the wrong suspects first — the macro library, then the peephole
optimiser — and cleared both (an investigation that produced independent proof each of
the 54 peephole eliminations on that program was individually safe). Runtime tracing
finally pinned it to a single line, `buf[i] = rj_sbox(buf[i])`, where the C runtime's
zero-page pointers picked up a wrong high byte and cascaded into wild stores.

The culprit was an `-O3` optimisation called the **ASGN-into-call fold**: making a
function call store its result *directly* into the assignment's destination, skipping a
temporary. Clever, and usually a win — but it fired in two unsound situations:

1. **`dst[i] = f(...)`** — it stored through the *destination-address temporary*, but the
   call clobbers the scratch temporaries, so the address was stale by the time the folded
   store ran. (The compiler even emitted SAVE/RESTORE around the call — but the folded
   store executed *before* the RESTORE.)
2. **Width mismatch** — an `int`-returning call folded into a `char` assignment stored a
   *word*, clobbering the neighbouring byte. `somechar = somefunc()` to a global would
   quietly overwrite the next global in memory.

The fix suppresses the fold exactly when the call's result width differs from the
assignment's, or when the destination lives in a call-clobbered temporary — falling back
to the correct "result to a temp, RESTORE, properly-sized store" sequence. Matching-width,
stable-target folds (`int_global = int_func()`) are untouched, so there's no speed cost
where the fold was always safe.

This turned out to be the third bug in a *family*, all rooted in the same `-O3`
mechanism of one node "borrowing" another's temporary register:

- **Dereference lost across a type change** — `f(*(arr + i))` where `f` takes `unsigned`
  passed the *address* instead of the value. Two causes: a conversion-elision test that
  compared operands by *name* only (so after the INDIR fold, `(tmp0),0` and `tmp0` looked
  identical and the dereference was silently dropped), and a borrowed temporary released
  one step too early. Fixed by also requiring equal addressing modes for elision, and
  holding the borrowed slot until the parent has allocated its result.
- **Borrowed temporary freed while still shared** — in `p->a == a && p->b == b && …` the
  pointer in `tmp0` got overwritten by the next subexpression. A folded dereference only
  *owns* its child's temporary when it consumed the last reference to it; when the slot
  was still shared, releasing it freed something that wasn't ours. Fixed by tracking
  ownership through the reference count. `t_struct` now passes 12/12 at `-O3` for the
  first time.

### Constant folding at the wrong width

A subtle and dangerous one, imported straight from a known cc65 bug. The compiler folds
constant expressions at compile time — but it was folding them using the *host's* 32-bit
arithmetic. So `0xFF50 + 0x100` folded to `0x10050`, when on the 16-bit target it wraps
to `0x0050`. Compile-time and run-time disagreed, and comparisons against runtime values
came out wrong. It even corrupted a test's own failure counter — hundreds of folded
unsigned comparisons branching the wrong way.

The fix wraps *all* folds to the 16-bit target width, classifies literals with target
limits, and normalises any leftover constant operands the emitter didn't have a macro for
(swapping commutative operands, materialising through scratch). A whole class of "the
compiler computed a different answer than the program would" vanished.

### Signed `>>` was silently logical

`-16 >> 2` gave `16380` instead of `-4`. Both `RSHI` (signed) and `RSHU` (unsigned) were
emitting the *logical* shift family. C89 technically leaves signed right shift
implementation-defined — but every mainstream compiler sign-extends, and Oric code relies
on it. A new arithmetic-shift macro family (`ASRW`, built on `cmp #$80 / ror`) now backs
`RSHI`; `RSHU` stays logical.

### Constant-first comparisons stopped the build

`0 == -x` (with `x` unsigned) survived the folder and reached the emitter, which produced
macro names the library deliberately doesn't provide — a hard build failure. Since `==`
and `!=` are symmetric, the emitter now swaps to the constant-second variants, which also
means `0 == x` reaches the same optimised zero-compare forms as `x == 0`.

---

## Part 3 — The peephole optimiser grows up

The macrosplitter's peephole optimiser was an ad-hoc pass written months earlier and,
until this cycle, essentially untested. Bringing it into service meant both hardening it
and, honestly, making it smarter than the name "peephole" implies.

### From peephole to fixpoint + dataflow

A textbook peephole optimiser is a dumb, one-pass, look-at-the-previous-instruction
affair. This one has grown into something more like a small local optimiser that runs to
a **fixpoint** — it re-scans the whole instruction stream, applying every pattern, and
repeats until a full pass changes nothing. That matters because the transformations
*enable each other*: copy propagation removes a load, which exposes a dead store, which
the next iteration catches, and so on. Every transformation only ever shrinks the code, so
the loop is monotone and guaranteed to terminate.

The passes, roughly in order of sophistication:

- **Classic adjacency patterns** — self-store, load-after-store, dead store, tail-call
  `jsr`+`rts` → `jmp`, dead `rts` after `jmp`, cross-register transfers, same-immediate
  reload.
- **Dead-load elimination.** A recurring residue in the expanded output is a load whose
  value is overwritten by the *very next* instruction without being read — e.g. an
  in-place zero-extend expands to `lda t : sta t : lda #0 : sta t+1`, the self-store rule
  drops the `sta t`, and now the `lda t` is dead (both its value and its N/Z flags).
  There were 30 of these in AES-256 alone. The pass removes them — carefully excluding
  reads from the I/O page (see below).
- **Copy propagation.** The byte codegen threads values through `regN`/`tmpN` copies, and
  here the 6502-only focus pays off: `tmpN` and `regN` are *all* zero-page locations, with
  identical cost. A `lda X : sta Y` copy between two of them buys nothing. So the pass, for
  such a copy, scans the straight-line run that follows; if every later read of `Y` can be
  re-pointed at `X` (with `X` unmodified up to that read) and `Y` is overwritten before the
  block ends, it forwards the reads and deletes the copy. `lda reg1 : sta tmp0 : lda #27 :
  eor tmp0 : sta tmp0` collapses to `lda #27 : eor reg1 : sta tmp0`.
- **Redundant index-load elimination + `iny`/`dey` rewrites.** Argument passing reloads
  the index register with a value it already holds (`ldy #0 : … : ldy #0`). A
  block-scoped value tracker deletes the redundant load, and rewrites a `ldy #N` where the
  register already holds `N±1` into a one-byte `iny`/`dey` — which reproduces the identical
  N/Z flags, so it's unconditionally safe. Zero redundant `ldy`s remained in AES-256's
  compiler code afterward, down from 25.

### The comment that was a wall

One fix here is almost embarrassing in hindsight and gave one of the biggest single wins.
The macro expander inserts `; === MACRO ===` annotation lines between expanded macros. The
optimiser parsed these as *labels* — which are barriers. So the peephole was effectively
running *per macro*: it could never match a pattern that straddled a macro boundary, like
`sta tmp0` at the end of one macro followed by `lda tmp0` at the start of the next — a
textbook load-after-store it simply never saw.

Making annotation comments a *transparent* token type — kept in the output, but skipped
when scanning for adjacent instructions and not resetting value tracking — unlocked
cross-macro matching everywhere. AES-256 at `-O1` fell from 16274 to 14031 bytes on that
change alone. The only cost is cosmetic: once code moves across them, the `; === MACRO ===`
markers no longer line up perfectly with their instructions. On a heavily-optimised build
the output is interleaved and messy anyway — a fair trade.

### Hardening: four ways to be unsafe

Before trusting the optimiser more widely, it was audited against the specific ways a
6502 peephole pass can silently break a program:

1. **Self-modifying code.** Regions containing `*+N` / `*-N` operands are *frozen* — no
   elimination or size change is allowed to move a self-modification target.
2. **Hardware I/O.** Loads from the I/O page (`$0300-$03FF`, the VIA and friends) are never
   eliminated — reading a hardware register has side effects, even if the value is
   "unused."
3. **Flag semantics.** Load-after-store elimination is allowed only when the N/Z flags
   already reflect the register value, preserving the meaning of a following branch.
4. **Barriers.** Real labels, directives, and self-mod spans still stop the optimiser;
   only the annotation comments are transparent.

And a bug the hardening itself surfaced: the immediate-value normaliser parsed with
`strtol`, which clamps at `LONG_MAX`. The compiler emits 16-bit constants sign-extended to
32 hex digits, so a bitfield clear-mask like `#>($fffff0ff)` (which should be `$f0`) became
`#255`, and field stores stopped clearing bits. Every signed-bitfield test failed with the
optimiser on. Switched to `strtoul` with a truncation to 16 bits, matching how the
assembler actually evaluates.

### Coming next: branch relaxation

The comparison macros emit a safe-but-bulky idiom — `branch-if-false around a jmp` — so
the jump can reach any distance. But most targets are close. The next pass, currently in
development, is **branch relaxation**: measure the (function-local, all-compiler-generated)
distance to the target and, when it comfortably fits in a 6502 relative branch's ±127-byte
range, collapse the whole idiom to a single conditional branch. Because it only ever
*shrinks* code, it slots straight into the fixpoint loop — and each shrink can bring the
*next* branch into range, exactly the "rinse and repeat until it settles" cascade that a
one-shot pass would miss. Conservative distance estimates keep it safe: a wrong guess can
only make it skip a relaxation, never emit an out-of-range branch.

---

## Part 4 — The macro library and the assembler layer

### A high byte one page too high

The static audit that grew out of the AES hunt found a real page-boundary bug hiding in
the macros. `#define HIGH >` aliased XA's high-byte operator — but `<` and `>` bind the
*whole* trailing expression, so `#HIGH(cte1)+1` assembles as `HIGH(cte1+1)`. Whenever the
constant's low byte is `$FF`, that `+1` carries into the high byte: the pointer lands one
page too high. It affected `MOVW_CY` and six shift macros. The fix simply removed the
spurious `+1`, matching the correct sibling macros.

A second audit find: `ASGNW_YD` loaded the source low byte with `lda ptr1,y` and then
immediately did `lda #0`, discarding it *and* leaving Y at the wrong offset for the store.
A one-character fix (`lda #0` → `ldy #0`).

### Retiring 30-year-old self-modifying code

The library carried 52 `sta/stx *+N` operand patches — self-modifying code from the FRASM
era, before the current assembler existed. They worked, but their safety against the
optimiser depended on layout accident rather than any structural guarantee, and they gave
no hint what byte they patched. All 52 were rewritten to use symbolic labels (`src+1`,
`dst+2`, `jsrto+1`), which the optimiser treats as barriers by construction. Proven
byte-identical across every benchmark and test program. In the same spirit, the FRASM-era
`LOW()`/`HIGH()` aliases were replaced by native `<`/`>` across all 249 uses —
expansion-neutral, verified by hash compare.

### A `strtok` in the assembler's own line parser

`link65` failed to assemble some `-O3` struct code with "Label 'mul16i' not defined." The
cause was in its statement scanner: `strtok` pokes NUL bytes into a shared line buffer, and
the opcode scan could run past the current `:`-separated statement. A NUL landing in a
later statement hid the remaining separators, so trailing statements — like the
`jsr mul16i` inside an expanded macro — were never scanned and the library dependency went
missing. Fixed by parsing each statement in an isolated buffer copy (while keeping
preprocessor lines whole, so `#include` paths with a drive-letter `:` still work).

---

## Part 5 — The C library was quietly broken

The runtime library got the most sobering audit of all. Several standard functions had
been wrong for years, unnoticed because the code paths that used them were rare.

**The dynamic allocator was broken three ways.** `malloc` had a fall-through bug where the
"no next block" case overwrote its own length limit with zero — so on the *very first
allocation*, the heap-end bound was never enforced. `free` null-checked the C stack pointer
instead of the pointer actually passed in, and used a 16-bit decrement (`dec lo : bpl hi`)
that corrupts the high byte whenever there are ≥129 live blocks. `realloc` was broken in
*every* path — a signed comparison mis-routed shrinks as grows, the shrink path read and
wrote the descriptor's NEXT pointer instead of its length field (corrupting the free list),
and the grow path never actually resized anything. All three were rewritten and are now
covered by a 39-check validation test.

**The string functions had a page-crossing curse.** Several `string.h` routines worked fine
on short strings and failed silently past 256 bytes:

- `strchr(s, '\0')` returned NULL — it tested for end-of-string *before* comparing, but
  C89 says the terminator is part of the string.
- `strstr` never rewound the haystack after a partial match, so overlapping matches were
  missed (`strstr("aaab", "aab")` returned NULL instead of `s+1`).
- `strcspn`/`strspn` jumped into the *inner* loop on a page boundary, skipping the first
  character of every 256-byte page.
- `strrchr` computed its result with the scan-advanced high byte, returning an address too
  high by the number of pages crossed.
- `memccpy` was the worst: it patched a byte inside `strncpy` that — after `strncpy` was
  rewritten — had become an *opcode* rather than an operand, and jumped to a label that no
  longer existed. Any program merely *referencing* `memccpy` failed to link. Its prototype
  was missing from `string.h` entirely.
- `strtok` produced empty tokens on consecutive delimiters (no leading-delimiter skip) and
  mis-sequenced the final token.

All rewritten to C89 semantics and covered by a 24-check test that deliberately uses
buffers longer than 256 bytes.

**A runtime `(float)` cast of an int gave zero.** The int→float conversion routine pointed
at `$DF24` — the *tail* of the ROM's SGN routine, which only converts a signed 8-bit value.
So `(float)some_int_variable` was wrong for anything outside `-128..127`. It went unnoticed
because casts of positive *literals* are constant-folded at compile time. Now it uses the
proper unsigned 16-bit ROM entry with sign handling.

**SEDORIC support was rewritten from scratch.** The old `sedoric()` copied its command
string *unbounded* into a line buffer that overlaps the C runtime's zero page — any command
longer than 26 characters corrupted the runtime pointers — and clobbered the BASIC text
pointer without restoring it (the long-reported "syntax error on exit"). The rewrite adds a
reusable zero-page *shelter* module (`_zp_swap`, a self-inverse 20-byte routine that swaps
only the compiler's zero-page block out of the way during a DOS call), bounds the command
properly, restores the text pointer, and adds `sed_savefile`/`sed_loadfile` for reading and
writing SEDORIC data files. It was validated end-to-end in the emulator: booting a generated
SEDORIC disk, saving a file that physically appears in the disk image, loading a HIRES
picture to `$A000` and watching it display — with the C program still running afterward.

**And the runtime got smaller.** The 256-byte software stack is no longer emitted into the
`.tap` (it moved to uninitialised RAM above the image); the `enter`/`leave` frame routines
and the float conversion helpers moved out of the always-linked header into on-demand
library modules. A minimal program shrank by 396 bytes, and every non-float program saved
another 33.

---

## Part 6 — The preprocessor, and the discipline that held it together

The default C preprocessor is now **mcpp 2.7.2** — C99-conforming, with real `file:line`
diagnostics and hard errors on missing includes (the legacy `cpp.exe` stays bundled and can
be selected back). Its output was verified byte-identical to the old preprocessor across the
test suite and ~175 sample pairs before it was made the default. Two related papercuts were
fixed at the same time: a project's own `-I` include directory is now searched *before* the
bundled headers, and the `INCLUDE`/`C_INCLUDE_PATH` environment variables are cleared around
the preprocessing pass (mcpp honoured them, so building from a Visual Studio developer prompt
would silently pull in MSVC's system headers).

But the single most important piece of infrastructure isn't a compiler feature at all. It's
the rule that made all of the above safe to ship:

> **Metrics without correctness are worthless.** A compiler bug that emitted a program
> consisting of a single `RTS` would produce the smallest and fastest binary imaginable —
> and compute nothing. So *no size or speed number is ever reported until the program's
> actual output has been verified correct.*

That principle turned an **extensive suite of test programs into the instrument for
judging every change** — because "is this change an improvement?" is a three-axis question
(is it still *correct*, is it *smaller*, is it *faster*?), and the only honest way to answer
it is to measure all three, automatically, on every commit. A **known-answer test suite**
builds and *runs* every test program in the emulator at `-O1`, `-O2` and `-O3` — with the
peephole optimiser both on and off — reading results back through the emulated printer and
comparing them to expected values. Nothing is committed unless the entire suite passes, build
*and* execute, at every level. Alongside it, a **benchmark harness** over a 22-program corpus
records code size and exact cycle counts (via the emulator's internal cycle counter, IRQs
masked for determinism) — but only *after* confirming the sample's computed output is
byte-identical across all five compiler configurations. So each candidate change produces a
concrete verdict: green across the suite, and a size/speed delta on the benchmark that is
trustworthy precisely because a fast-but-wrong build was quarantined before its numbers were
ever read. A change that shrinks the code but breaks a single test is not a smaller program —
it's a bug, and the harness says so before it can be committed.

On top of that, roughly 313 upstream cc65/SDCC regression tests are imported and run
*unmodified*, with a small shim supplying the standard headers and documenting OSDK's real
type sizes — a constant external check against a mature 8-bit C implementation.

### Reading the output — the loop that found the optimisations

The measurement half of the loop tells you whether a change is good. The other half is where
the changes *came from*, and it is almost embarrassingly low-tech: **read the generated
assembly, by eye, looking for waste.** Nearly every optimisation in this release started as
someone scrolling through the expanded 6502 output of a real program — the AES-256 log-table
routine `_gf_alog` became the recurring specimen — and noticing a pattern that shouldn't be
there:

- *"This `lda tmp0` right after a `sta tmp0` is pointless — the value is already in the
  accumulator."*
- *"`lda reg1 : sta tmp0` then `eor tmp0` — on the 6502 these are both zero-page, so why copy
  at all? Just `eor reg1`."*
- *"This whole `lda / clc / adc #1 / sta` is a `char` increment. That's an `inc`."*
- *"That `cmp #0` after an `lda` does nothing — the load already set the Z flag."*

Each observation became a hypothesis — *this pattern is safe to rewrite* — that was then
implemented (in the compiler if it was a code-*selection* issue, in the peephole pass if it
was a *cleanup* issue), and immediately run back through the test-and-benchmark harness to
prove it changed the numbers in the right direction and nothing else. The output of the *new*
build was then read again, because each cleanup exposes the next: collapse the copies and a
redundant load appears; remove the load and a dead comparison surfaces. That feedback cycle —
**read the disassembly → spot a pattern → implement → measure → read the new disassembly** —
is the engine that drove the whole effort.

What made it productive was that it was genuinely **collaborative, human and AI together.**
Both parties read the same generated code and brought different eyes to it: the AI was good at
systematically enumerating a pattern's every occurrence and reasoning about the flag-liveness
and aliasing edge cases that make a rewrite safe or unsafe; the human brought the 6502
instinct — *"that's just a `dec`,"* *"if that jump is under 128 bytes away the whole idiom is
one branch"* — that names the destination an automated pass would take much longer to
discover. The in-place `inc`/`dec`, the copy propagation, the operator-folding across
temporaries, and the branch-relaxation pass now in progress all trace back to a specific
"wait, why is it doing *that*?" while staring at a listing. The test suite is what let that
loop run fast and fearlessly — you can act on a hunch about a listing precisely because, if
the hunch is wrong, the suite catches it in minutes rather than in a user's crashed program.

---

## The results

With everything above in place, on the ISS `oricCompilerBenchmark` corpus (22 samples,
measured against OSDK 1.23's `-O2` as the anchor, every figure correctness-gated):

- **Code size:** −20% to −27% on typical code, up to −30% on some samples; AES-256 −16.5%.
- **Speed:** the standouts — frogmove −67.8%, memcopy −44%, selection-sort −35%,
  AES-256 −31.3% at `-O3` (−37% at `-O2`, where the char widens are elided), bubble-sort
  −30%.
- The peephole optimiser adds a further ~1-4% on top, and — by construction and by test —
  never changes a program's output.

---

## What it adds up to

A few themes run through all of this.

**Old code hides.** The allocator, half of `string.h`, the int→float cast, the SEDORIC
interface, a page-boundary bug in the macros — none of these were regressions. They were
long-standing defects sitting in code paths that real programs happened to avoid. What
flushed them out was not cleverness but *coverage*: a test suite that actually runs the
code, and an audit that reads every macro looking for the shape of a known bug.

**On a small machine, the abstraction tax is the whole bill.** The single biggest code-size
win came from simply *not* promoting `char` to `int` when the result is a `char` — undoing an
abstraction the C standard mandates but the 6502 can't afford. Committing to one architecture
is what made that, and copy propagation between "equivalent" zero-page temporaries, and a
dozen other small wins, sound rather than reckless.

**Correctness and size are one discipline.** Every optimisation here was gated on a program
still computing the right answer, verified by execution, before its bytes were counted. That's
not bureaucracy — on hardware where a stray high byte is a crash, "smaller" and "correct" are
the same word, and the only way to pursue one without sacrificing the other is to measure both,
every time.

---

*This document accompanies the OSDK 1.24 patch notes. The full commit-by-commit fix list,
with the "Found / Problem / Fix" breakdown for each item, lives in
`PATCHNOTES-1.24-draft.md`.*
