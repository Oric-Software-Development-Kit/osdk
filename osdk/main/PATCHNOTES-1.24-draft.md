# OSDK 1.24 — draft release notes (feature/compiler-improvements)

Working document for the official patch notes (`doc_historic.htm`, 1.24 section), organised
**new-vs-fixed** the way the published changelog should read. A "Fixed" item is something that
was broken for users of the *released* 1.23 (or a long-standing latent bug); a "New" item is a
1.24 feature that didn't exist before (so incomplete corners of it that were finished during
development are *part of the feature*, not fixes). Kept the Found/Problem/Fix detail per entry
for reference; the published notes will condense. Items marked `[documented]` are already in
doc_historic.htm; commit hashes in parentheses.

---

# NEW IN 1.24

## The optimizing C compiler (compiler.exe → V1.41)

The 1.24 compiler gains a real optimising codegen path. None of the following existed in 1.23.

### Native 8-bit arithmetic for `char` operands (Phases 1–4: `+ - & | ^`, `~ - <<1`, comparisons, `>>1`)
- The compiler always promotes `char` to `int` (per C rules), so even fully-8-bit code like
  `c = a + b` (all `unsigned char`) was widen-both-to-16-bit, a 16-bit ADD, then a narrow-store —
  roughly twice the code and cycles a byte op needs. New at -O2/-O3: a pre-pass
  (`mark_byte_narrowing`) recognises the promoted `CVUC/CVIC( <arith>( CVCU-widen-of-char | int-const, … ) )`
  shape where the result is only ever used as a char, and emits a byte-width op (new
  `ADDB/SUBB/ANDB/ORB/XORB` etc.). Exact incl. 8-bit wraparound; guarded to single-use so any
  value also used at 16-bit keeps its full-width path; `int`/`float` code byte-for-byte unchanged.
- **Dead-widen elision:** the operands' `CZBW` zero-extend is dropped when the widened temp is
  proven consumed only by byte-narrowed ops before its next redefinition (liveness on the
  *physical* temps after allocation, so `while(i--)` — where the loaded `i` feeds both the byte
  decrement and the 16-bit `!=0` test — correctly keeps its widen). -O3 emits a byte-only load
  (`CWB`), making -O3 no slower than -O2 on char-heavy code (aes256 58.1M→50.9M cyc).
- **Phase 2:** `~x`(`COMB`), unary `-x`(`NEGB`), `x<<1`(`LSH1B`). **Phase 3:** `== != < > <= >=`
  uchar-vs-uchar/const emit unsigned byte compares (`EQB/NEB/LTUB/GTUB/LEUB/GEUB`) — unsigned so
  ≥128 compares correctly. **Phase 4:** `x>>1` uchar → single `lsr` (`RSH1B`, logical); in-place
  `x<<=1`/`x>>=1` → single `asl`/`lsr` RMW. `*` deliberately stays on the word path (no 6502 mul).
- **Result:** `t_char8` (39 checks) green -O1/-O2/-O3; aes256 -O2 14840→13122 B (−11.6%).

### 16-bit shift by exactly 8 → byte move, not an 8-step loop
- `x<<8`/`x>>8` (and `x*256`, `x/256`, `x%256` after strength reduction) used the generic
  variable-count loop (~100 cyc). Now a constant count of 8 emits a byte-move (`LSHW8`/`RSHW8`
  logical/`ASRW8` arith, ~12 cyc). New `t_arith` checks pass -O1/-O2/-O3; shrank many samples.

### Small constant left shifts unrolled (`x<<2`, `x<<3`)
- `x<<2`/`x<<3` (`x*4`/`x*8`) went to the variable-count loop (~10 bytes of scaffold). Now one
  `LSH1W` + `n-1` in-place `LSH1W_D` doublings — both smaller AND faster (`x<<2` 22→14 B), so
  unconditional. Counts ≥4 (which trade size for speed) deferred to the size/speed axis.

### In-place `char` increment/decrement → single `inc`/`dec`
- A narrowed `c++`/`c--` (byte ±1, operand==result, ZP/direct) now emits `INCB_D`/`DECB_D`
  (one `inc mem`/`dec mem`) instead of the five-instruction `clc/lda/adc/sta` path.

### Byte compare against zero drops the redundant `cmp #0`
- A narrowed `char == 0`/`!= 0` now routes through `compare0` → `EQ0B_D`/`NE0B_D`
  (`lda c : beq/bne …`), since `lda` already sets Z/N. Equality only (ordered compares need the
  carry `cmp` sets). −2 bytes/compare on the very common `while(c)` / `if(!c)` patterns.

### `-Os` optimize-for-size flag
- The compiler only accepted `-O0..-O3`. `-Os` (and `-OS`) now select optimize-for-size — maps
  to `-O3`-level codegen today (byte-identical; on the 6502 nearly all optimisation is Pareto),
  and sets an internal size-axis flag for future transforms that trade size for speed. First half
  of the size-vs-speed axis (per-function `#pragma optimize(size|speed)` still to come). Encounter's
  C-heavy modules −~11%. (`2300ed44`)

### `0b` binary integer literals `[documented]`
- lex.c accepts `0b`/`0B` literals (e.g. `0b101 == 5`). (`b301e483`)

### CRT slimming — −396 bytes for a minimal program
- The 256-byte software stack is no longer emitted in the tap (moved to `.bss` above the image);
  `enter`/`leave` moved to `lib/frame.s` (linked on demand); `cif`/`cfi` to `lib/float.s`
  (−33 B for every non-float program). (`6ade0460`, `32f86eff`)

## New 6502 peephole optimizer (MacroSplitter 0.3, `OSDKMACRO=-O`)

Entirely new in 1.24 — a fixpoint optimiser over the expanded assembly. **It only ever runs on
compiler-generated code; hand-written `.s` (libraries, user asm) is copied through untouched.**
Everything below is part of introducing this feature (including corners finished during
development — they were never in a shipped product).

- **Built-in macro expansion is the default** (`OSDKMACROEXPAND=1`): compiler output expanded by
  macrosplitter `-M` instead of a second cpp pass; `=0` restores the old path. Byte-identical taps
  verified. (`2ee28af7`)
- **Base passes:** self-store, load-after-store (flag-guarded), dead-store, tail-call `jsr+rts→jmp`,
  dead `rts` after `jmp`, cross-register transfer, same-immediate reload.
- **Dead-load elimination:** drop a `lda/ldx/ldy` whose register is overwritten by the next
  instruction without being read (I/O-page reads excepted). 30 in aes256 alone. (`edd89078`)
- **Copy propagation:** forward `lda X : sta Y` into later reads of `Y` and delete the copy when
  `Y` is confined to the block — since `tmpN`/`regN` are all equal-cost zero page, the copy buys
  nothing. Guarded on a pointer's high byte (`(P),y` uses `P` and `P+1`). aes256 -O3 11137→10645 B.
  (`94dc1336`)
- **Optimize across macro-annotation comments:** the `; === MACRO ===` markers are now a
  transparent token type, so patterns can match across a macro boundary (previously the peephole
  was effectively per-macro). Big win: aes256 -O1 16274→14031 B. (`6e7eaf97`)
- **Redundant index-reload elimination + `iny`/`dey` rewrites:** delete a `ld_ #N` whose register
  already holds `N`; rewrite `ld_ #N±1` to `iny`/`dey`/`inx`/`dex` (same N/Z flags). aes256 -O3
  redundant `ldy` 25→0. (`1428ee84`)
- **Branch relaxation:** collapse the range-safe `.( b<cc> skip : jmp TARGET : skip .)` idiom to a
  single inverted branch `b<!cc> TARGET` when TARGET is within a conservative reach (MAXSPAN 120 <
  the true ±127); runs in the fixpoint so each collapse can bring more into range. Two-branch
  `>`/`<=` idioms relax correctly. −3 bytes at nearly every comparison; aes256 -O3 10605→10512 B.
  (`f09b4d47`)
  - **Orphaned-scope dissolve:** after relaxation, the now-unreferenced local `skip` label + its
    `.( .)` are removed — de-clutter *and* removes an optimizer barrier, exposing more adjacency.
    (`4c96bc94`)
- **Fold a word return routed through a scratch temp:** `lda Vlo : sta T : lda Vhi : sta T+1 :
  ldx T : lda T+1 : (jmp leave|rts)` → `ldx Vlo : lda Vhi : …` (byte case `ldx Vlo : lda #0`).
  −8 bytes per byte-returning function. aes256 -O3 10509→10477 B. (`0f3d9e89`)
- **`.csource`/`.ctype` debug directives kept whole:** the `-g1` debug directives carry `:` as
  data (`.csource "C:/path" 12`, `.ctype …name:char[8]:0:8…`); `TokenizeLine` now keeps them whole
  (plus a quote-aware `:` scan) instead of splitting them into garbage. This finished the
  peephole+`-g1` pairing (neither the peephole nor `-g1` existed in 1.23, so this is part of the
  feature, not a regression fix). (`9548e167`)
- **Safety hardening (4 classes):** `*+N`/`*-N` self-mod regions frozen; I/O-page `$300-$3FF`
  loads never eliminated; load-after-store requires live N/Z semantics; comment/label/directive
  barriers respected. (`62e3ccc8`) Also: the immediate normaliser now parses with `strtoul` and
  truncates to 16 bits so `#>($fffff0ff)`=`$f0` (was mis-clamped) — a bug in the new normaliser,
  fixed within development. (`7b85903b`)

## Debug support (new; merged from feature/debug-support)

New in 1.24: `-g1` emits `.csource` source-line and `.ctype` struct/enum/type directives; the
extended symbol export carries source locations + type info; Oricutron gains a GDB-compatible
debug stub (TCP breakpoints/step/mem/regs) + a screen/keyboard visualization stream; `#pragma
optimize(push/pop/n)` for per-function optimization level. (merge `2b8b0104`; details in
doc_historic.htm's debug section.)

## C library additions (lib/*.s, headers)

- **`atoi`, `fprintf`** (+ minimal `FILE`/`stdin`/`stdout`/`stderr`); **`stdlib.h` `EXIT_SUCCESS`/
  `EXIT_FAILURE`** (their absence was the single most common failure across the imported cc65
  suite — 55 of 313 tests); const-correct `string.h`. (`d8f87385`, `d85d0f9b`)
- **SEDORIC file API + zero-page shelter:** new `sed_savefile(name,buf,len)` / `sed_loadfile(name,
  buf,&len)` via the documented DOS entry points, and a reusable `lib/zeropage.s` `_zp_swap`
  (self-inverse, IRQ-safe) that shelters the C zero-page block across ROM/DOS calls. Module
  ~366 B, pulled only when referenced. Runtime-validated on a generated SEDORIC disk. New
  `sample/c/sedoric`. (`ec84caed` + follow-ups) *(the `sedoric()` corruption fix is under Fixed.)*

## Preprocessor, build & test infrastructure

- **mcpp 2.7.2 is the default C preprocessor** (C99-conforming, `file:line` diagnostics, hard
  errors on missing includes); legacy `cpp.exe` bundled, `OSDKCPP` overrides. Byte-identical output
  verified vs cpp.exe on the suite + ~175 cc65 pairs. (`a5eb0c19`, `497e3a3d`, `2ee28af7`)
- Automated execution **test suite** (tests × -O1/-O2/-O3 via emulated printer, CSV diffs), a
  cc65/SDCC regression importer (~313 upstream tests run unmodified), the ISS benchmark runner with
  exact cycle counts, sandboxed/turbo/headless emulator. Not user-facing. (`eca90bd1`, `31cefb3f`,
  `5ab5a85a`)

---

# FIXED (broken in released 1.23 / long-standing latent bugs)

## Compiler codegen

### `-O3`: call result stored through a call-clobbered pointer / at the wrong width
- The long-standing "aes256 fails at -O3" report. The `-O3` ASGN-into-call fold fired in two
  unsound cases: for `dst[i] = f(...)` it stored through the destination-address temporary, stale
  after the call clobbered scratch; and it inherited the CALL's width, so an int-returning call
  folded into a char assignment stored a word over the neighbouring byte. Now suppressed when the
  call width differs or the destination lives in a clobbered temp. aes256 -O3 runs correctly for
  the first time. (First of the "borrowed-temporary" family.)
### `-O3`: dereference lost when passed as a differently-typed argument
- `f(*(arr+i))` passed the ADDRESS not the value — conversion-elision compared operands by name
  only (post-INDIR-fold `(tmp0),0` vs `tmp0` looked identical), and a borrowed temp was released
  too early. Fix: elision also requires equal addressing modes; borrowed slot held until the parent
  allocates. (`4f860d3a`)
### `-O3`: borrowed temporary freed while still shared
- In `p->a==a && p->b==b && p->c==c` the pointer in `tmp0` was overwritten. A folded INDIR only
  owns its child's temp when it consumed the last reference. Fix: ownership via reference count;
  t_struct 12/12 at -O3. (`b20215ef`)
### Constant folding used 32-bit host arithmetic
- `0xFF50 + 0x100` folded to `0x10050` (compile time) but wraps to `0x0050` (run time), so
  compares against runtime values failed. Fix: all folds wrap to the 16-bit target width; literals
  classify with target limits; emitters normalise leftover constant operands. (`7dbf6572`)
### Signed `>>` was a logical shift
- `-16 >> 2` gave 16380 not −4. `RSHI`/`RSHU` both emitted the logical family. Fix: new `ASRW`
  arithmetic-shift family (`cmp #$80 / ror`); `RSHI` emits it, `RSHU` unchanged. (`b0cb1c7a`)
### Constant-first `==`/`!=` stopped the build
- `0 == -x` (x unsigned) reached the emitter and produced macro names the library doesn't provide.
  Fix: `==`/`!=` are symmetric → swap the constant second; `0 == x` now reaches the `EQ0W`/`NE0W`
  forms too. (`44b69536`)
### Float expressions always failed — "expression too complex" (regression shipped in 1.23) `[documented]`
- The 1.40 temporary-register change marked all 32 float temporaries permanently busy. Fix:
  restored the per-function temporary reset; float temps frame-allocated on demand. (`b301e483`)

## Macro library (MACROS.H)

### Six `-O3` macro bodies emitted invalid assembly `[documented]`
- Any `-O3` build with `a = a - b` on stack-resident variables broke. `INDIRW_DY, SUBW_YYY,
  COMW_YY, ANDW_YYY, XORW_YYY, ORW_YYY` stored with a literal index / direct label instead of `,y`.
  Fixed; 8 missing `ASGNS` struct-copy variants added (`-O3 *ps=*pt` had died on an undefined
  macro). (`1a671d72`)
### `#HIGH(cte)+1` — wrong page for `$FF`-low constants
- `<`/`>` bind the whole trailing expression, so `#HIGH(cte)+1` = `HIGH(cte+1)`: high byte one too
  large when the low byte is `$FF` (wrong-page pointer). Affected `MOVW_CY` + 6 shift macros.
  Removed the `+1`. (`0c941568`)
### `ASGNW_YD` always miscompiled the low byte
- `lda ptr1,y` then `lda #0` discarded the loaded low byte and left Y wrong for the store.
  `lda #0` → `ldy #0`. (`0c941568`)

## Linker (Link65 → 1.5)

### Statement scanner corrupted multi-statement lines
- `-O3` struct code failed at assembly with "Label 'mul16i' not defined": `strtok` poked NULs into
  the shared line buffer past the current `:`-separated statement, hiding trailing statements (the
  `jsr mul16i` in an expanded MULI). Fix: each statement parsed in an isolated copy; preprocessor
  lines kept whole (drive-letter `:` in `#include` paths still work). (`7e890bdc`)

## C library (lib/*.s)

### Dynamic allocator repaired: malloc / free / realloc
- **malloc:** the "no next block" case overwrote its length limit with 0 → heap-end bound never
  enforced (fired on the very first allocation). **free:** null-checked the C stack instead of the
  passed pointer; 16-bit descriptor decrement corrupted the high byte at ≥129 live blocks.
  **realloc:** broken in every path (mis-routed shrinks, wrong descriptor offset, no real resize).
  All rewritten; 39-check `t_malloc.c`. (`03ebd774`)
### String functions: strchr, strstr, strcspn, strspn, strrchr, strtok, memccpy
- **strchr(s,'\0')** returned NULL (tested end before comparing; C89 says the terminator is part of
  the string). **strstr** never rewound the haystack after a partial match (missed overlaps) and
  returned garbage for `""`. **strcspn/strspn** jumped into the inner loop on a page boundary,
  skipping the first char of every 256-byte page. **strrchr** returned an address too high by the
  pages crossed. **strtok** produced empty tokens on consecutive delimiters. **memccpy** patched a
  byte that (post strncpy-rewrite) was an opcode and jumped to a deleted label — *any* program using
  it failed to link; prototype was missing from `string.h`. All C89-correct now; 24-check
  `t_string.c` (page-crossing). (`8fc57a6e`, `c29fcea1`, and the string-audit commit)
### sprintf missing NUL terminator `[documented]`
- Never wrote the terminating NUL (buffer reuse showed stale tails). (`d8f87385`)
### Runtime int→float conversion used the wrong ROM entry `[documented]`
- `cif` pointed at `$DF24` (tail of ROM SGN, 8-bit only), so `(float)some_int` was wrong for
  anything outside −128..127; unnoticed because positive-literal casts are constant-folded. Now
  uses the 16-bit ROM entry `$DF40` with sign handling. (`8c74528d`)
### file_unpack_raw: missing `clc` in the C wrapper
- The C wrapper added the unpacked size to the destination with the carry in whatever state the
  caller left it (sibling `_file_unpack` does `clc`); if C=1 at entry, one extra byte past the end.
  Added the `clc`.
### `sedoric()` corrupted the C runtime / clobbered TXTPTR
- The old `sedoric()` copied its command string *unbounded* into the `$35` line buffer (overlaps
  the C zero page from `$50`) — >26 chars corrupted `ap/fp/sp` — and clobbered `TXTPTR` without
  restore (the reported "syntax error on exit"). Rewritten to shelter the C block (`_zp_swap`),
  bound the command, and restore `TXTPTR`. (`ec84caed`)

## Build system

### INCLUDE / C_INCLUDE_PATH leaked MSVC headers
- mcpp honoured them, so building from a VS developer prompt silently pulled in MSVC system
  headers. Now cleared around the preprocessing pass. (`6ade0460`)
### Project `-I` now searched before the bundled include
- so a project can override a bundled header. (`29433d4e`)

---

# PERFORMANCE (ISS oricCompilerBenchmark, 22 samples, vs 1.23 -O2)

Two gates. (1) Agreement: every size/cycle figure reported only after the sample's computed output
is byte-identical across all five configs — proves no regression vs 1.23, NOT absolute correctness.
(2) Known-answer (2026-07-19): outputs also diffed against MSVC-host reference builds (32-bit long):
**19 of 22 PASS byte-for-byte; 10-pi and 08-mandelbrot FAIL because OSDK's `long` is 16-bit** —
a pre-existing limitation (types.c maps longtype to INT_METRICS, identical wrong output on 1.23),
surfaced by the ISS benchmark v2 report. **Code size** new-O3pp −20..31% typical
(−30.9% frogmove), aes256 −19.7%; total 98184 → 77983 B (−20.8%). **Speed** frogmove −70.4%,
memcopy −44.0%, aes256 −42.2% (-O3), selection-sort −35.0%, bubble-sort −30.2%. The peephole never
changes output; its win scales with macro density — ~1-3% on tight scalar code but up to −20.8%
cycles / −12% size on char-heavy code (aes256 -O3 → -O3pp), as the comment-transparency +
copy-propagation passes clean up across macro seams. (The former aes256 "-O2 faster than -O3"
anomaly is resolved by the -O3 dead-char-widen elision.) Full table + correctness matrix in
`TestSuite/compiler/benchtable-1.24.md` (regenerated 2026-07-18).

---

# INTERNAL / MAINTENANCE (not for user-facing notes)

- **FRASM-era `*+N` self-mod → symbolic labels** (52 sites): robustness/readability; proven
  byte-identical. (`0c941568`)
- **FRASM-era `LOW()`/`HIGH()` aliases → native XA `<`/`>`** (249 uses): expansion-neutral, hash
  byte-identical. (`b0cb1c7a`)
- ASRW naming glossary in MACROS.H/gen.c.
- Optimizer *exonerated* for the aes256 `-O3` crash (all 54 eliminations proven individually safe);
  the crash was the codegen bug above, not the peephole.

---

# STILL OPEN (not for 1.24 notes)

- aes256 `-O3` latent layout-sensitive bug (any 2-byte shift trips it; peephole exonerated).
- `tap2dsk -m` flag to emit MFM directly (backlog; backward compatible). Long-wished by Mike.
- Deferred design docs: static local allocation + `__fastcall` convention; memory-access tracer.
