# `__fastcall` v1 — compiler-side implementation plan

Companion to `register-param-passing-brief.md` (ABI + annotation, cross-signed) and
`future-static-alloc-and-fastcall-design.md` (deeper background). This is the compiler-internal
build plan, grounded in a front-end/back-end map of the LCC 1.9 source
(`compiler/sources`, `compiler/includes`). **Design only — nothing built yet.**

## The v1-shrinking insight: call-site only

A `__fastcall` function has two halves — the **caller** packs args into registers, the **callee**
reads them. v1's marked functions are the **hand-asm library routines** (`puts`/`putchar`/`_putc`),
whose callee side *we write by hand* (and mostly by shim-deletion — see the brief). So **v1 needs
only the caller half in the compiler**: emit register-packed calls to `__fastcall` targets. The
compiler does *not* need to generate `__fastcall` callee prologues until someone marks a
*C-defined* function — that's the fast-follow. This roughly halves v1.

## ABI (from the brief)
Opt-in `__fastcall` per function. Params packed little-endian across **A, then X, then Y**
(char→A, int/ptr→A:X). `__fastcall` with **>3 param bytes** or **variadic** ⇒ **hard compile error**
(explicit marker ⇒ fail loud). Returns unchanged (X:A).

## Front-end: the keyword (cheap)

1. **Token** — `includes/token.h`: reuse a RESERVED slot (token numbering is positional/array-indexed;
   do NOT append past `EOI=127` or insert mid-list). Change e.g. `yy(0, 17, ...)` →
   `zz(FASTCALL, 17, 0,0,0, CHAR, "__fastcall")`. `kind==CHAR` classifies it as a
   declaration-specifier token (same as `const`/`volatile`), which is exactly what `decl.c` keys on.
   Also add `#define FASTCALL 17` in the `#ifndef __STDC__` fallback block (token.h:145-200).
2. **Lexer match** — `includes/keywords.h`: the bare `case '_':` (line ~342) currently has no body and
   falls through to the identifier handler. Add a body matching `_fastcall` (leading `_` already
   consumed by the switch): `rcp[0]=='_'&&rcp[1]=='f'&&…&&!(map[rcp[9]]&(DIGIT|LETTER))` → `return FASTCALL`.
3. **Specifier parse** — `sources/decl.c` `type()` (~line 1034): add `case FASTCALL:` with its own
   accumulator flag (mirror the `const`/`volatile` `cons`/`vol` pattern), duplicate ⇒ error.
4. **Carry it on the function type** — `includes/c.h` `struct tynode` has **no spare flag field**;
   const/volatile are encoded arithmetically in `op`. Chosen approach: **add `unsigned fastcall:1`
   to `struct tynode`** and **include it in the `tynode()` interner key** (`types.c:626-647`,
   the hash-cons at :633-636) — otherwise fastcall and non-fastcall function types collide into one
   interned node. Smallest blast radius vs. a distinct `FUNCTION_FAST` Typeop (which would touch
   every `isfunc()`/`case FUNCTION`).
   - **The one non-trivial bit:** the specifier is parsed in `type()` (base type) but the FUNCTION
     type is built later in the declarator (`func()`, `types.c:344`). Thread the parsed flag from
     `type()` down to where `func()` is constructed and set the bit there. Bounded, but it's the
     part to get right. Validate with `isfunc` + the interner.
5. **Errors** — at the point the function type is finalized with a prototype: if `fastcall` and
   (total param bytes > 3 || variadic) ⇒ `error(...)`.

## Back-end: caller half (`gen.c`)

**Seeing the callee's fastcall-ness at the ARG.** Today the ARG/CALL nodes consult only
`optype`/`argoffset`/arg-size; the fastcall bit lives on the callee's function type, reachable from
the CALL node's callee child. Use a **pre-pass** (before `tmpalloc`, exactly like the existing
`mark_inplace_rmw`/`mark_fuse_addk` pre-passes, whose Xnode flags survive `tmpalloc`):
- Walk the forest; group ARG nodes to their CALL (args precede their CALL, same accounting
  `tmpalloc` already does with `argoffset`).
- For a CALL whose callee type has `fastcall`: assign each ARG a **target register** per the packing
  rule and stamp it on a new **Xnode field `fastreg`** (0 = stack, else A/X/Y code). Compute the
  A/X/Y layout from the arg sizes.

**Emit (BOTH emitters — `emitdag0` unopt @ ~1348 AND `emitdag` opt @ ~1714; patch both or -O0/-O3
diverge).** In the ARG cases: if `p->x.fastreg` is set, emit a register load of the arg value
instead of the `(sp),y` store. New MACROS.H macros, addressing-mode-parameterized like the existing
`ARGB_`/`ARGI_` family:
- `ARGRA_<am>(value)` → load value low byte into A (char / low half)
- `ARGRX_<am>(value)`, `ARGRY_<am>(value)` → into X / Y (high half / later bytes)
(names TBD; one per target register, `<am>` = source addressing mode: immediate/direct/frame.)
The CALL emits `jsr` as today with **no arg-frame offset** for the register args.

**v1 arg-eval simplification:** only handle param lists that are **a single ≤3-byte arg OR multiple
args whose evaluation can't clobber an already-loaded arg register**. The general multi-arg
clobber-ordering problem (evaluating arg 2 trashes A holding arg 1) is **deferred**; the console
routines are all single-arg so v1 sidesteps it. Guard: if a `__fastcall` call's args would need a
clobber-safe ordering v1 doesn't do yet, fall back to stack for that call (or defer marking).

## Back-end: callee half (FAST-FOLLOW, not v1)

For a C-defined `__fastcall` function, `function()` (`gen.c:389`) param-binding loop
(`gen.c:410-421`) would bind the first A/X/Y params and the prologue would **stash A/X/Y into the
param frame slots** before use (or keep in-register for leaves). Not needed in v1 (library callees
are hand-asm). Documented here so the type flag we add already supports it.

## Library conversion (v1)
- `conio.h`: mark `_putc`/`puts`/`putchar` (and `_puts`) `__fastcall`.
- `lib/gpchar.s` + `lib/conio.s`: delete the `(sp),y` read shims (the register entry becomes the
  public entry); `__puts` reads the pointer from A:X (`sta tmp/stx tmp+1`). Annotate each with the
  frozen `@function <names…>` + `@param … in=…`.
- These are the same routines already annotated for dead-code stripping — the two features compose.

## Verifier (v1)
Assert each hand-asm callee's `@param in=` locations == the compiler's prototype-derived packing.
Cheapest home: a link65 or build-step check (it already parses `@param`); WARN on mismatch. This is
the anti-drift guard nfo-lang asked for.

## Gates (must all hold)
- Full known-answer suite build+run green.
- **Unmarked / stack-path code byte-for-byte identical** (fastcall is opt-in → zero change to
  anything not marked).
- `__fastcall` on >3 bytes / variadic ⇒ compile error (tested).
- hello-world `final.out` crosses under vbcc's 99.
- A `__fastcall` call round-trips correctly (known-answer: `putchar`/`puts` output).

## Gotchas (from the map)
- **Both emitters** must branch (easy to patch one).
- **Interner key** must include the fastcall bit or types alias.
- **Positional token numbering** — reuse a reserved slot only.
- **Specifier→func() threading** is the one fiddly front-end bit.
- CALL node must reach the callee function type (via callee child) for the pre-pass.

## Build order (each gated)
1. Front-end keyword end-to-end: `__fastcall` parses, sets the type bit, over-capacity/variadic
   errors — verified by compiling a marked prototype and dumping the type (no codegen change yet).
2. Caller pre-pass + emit + `ARGR*` macros; convert `putchar` (single char→A) first — smallest case.
3. `puts`/`_putc` + `__puts` (ptr→A:X); the shim deletions; annotations.
4. Verifier; full gates; measure hello-world.
