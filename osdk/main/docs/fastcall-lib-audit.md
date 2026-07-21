# `__fastcall` library audit (2026-07-21)

Which OSDK library functions can be improved by register parameter passing.
121 public prototypes surveyed across the headers + `library.ndx` + `lib/*.s`.
Companion to `register-param-passing-brief.md` / `fastcall-v1-implementation-plan.md`.

## Buckets
- **ZERO** (no params): 25 implemented. A call still emits a dead `ldy #0`. Marking these
  `__fastcall` is *free* — no `.s` change (0 args to read) — once the lean call (no `ldy`) is
  extended to the value-returning paths (`CALLW`/`CALLI`/`CALLB`; only `CALLV` done in step 2).
  K&R `()`-declared ones (clock, heapinit, cls, lores0/1, is_overlay_enabled, cread, cwritehdr,
  joystick_*, uninstall_irq_handler) need `(void)` added to satisfy the prototype requirement.
  Examples: getchar, rand, random, clock, key, get, cls, hires, text, ping/shoot/zap/explode,
  kbdclick1/2, lores0/1, cread.
- **REG1** (one ≤2-byte param → char in A, int/ptr in A:X): 36 convertible (+ `_puts` already done).
  Each needs its callee to read the register instead of `(sp),y`, then the prototype marked
  `__fastcall`. **Cheapest = those that already have a register internal entry behind a stack
  shim** (just delete the shim):
    - `putchar`/`_putc` (gpchar.s `putchar`, char in **A**) — but 3-way alias `__putc`/`_putchar`
      shares the shim; mark all together (see hazards).
    - `toupper`/`tolower`/`toascii` (tstring.s `_touppermc`, char in **X**).
    - `malloc`/`free` (malloc.s `_mallocmc` / free.s `_freemc`, size/ptr in **A:X**).
  High-value simple ones without a pre-made reg entry but trivial bodies: the 11 ctype predicates
  (isalpha/isupper/islower/isdigit/isspace/ispunct/isprint/iscntrl/isascii → char, `(sp),y`→X→
  ctype[x]); atoi, strlen, itoa, srandom; cwrite, call, pattern; exit.
- **BIG** (>1 param, or >2-byte param, or variadic): 56. Deferred to the multi-arg step (A/X/Y
  packing) or never (variadic printf/scanf family; the 5-byte-`double` math.s functions).

## Cheapest conversions (existing register entry — delete the `(sp),y` shim)
| module | stack entry | register entry | reg |
|---|---|---|---|
| gpchar.s | `_putchar`/`__putc` | `putchar` | A (char) |
| gpchar.s | `_gets` | `gets` | ptr in tmp (not A/X — needs a small adapter) |
| tstring.s | `_toupper`/`_tolower`/`_toascii` | `_touppermc` etc. | X (char) |
| malloc.s | `_malloc` | `_mallocmc` | A:X |
| free.s | `_free` | `_freemc` | A:X |
| strcpy.s | `_strcpy` | `_strcpymc` | op1/op2 (zp, BIG — 2 ptrs) |
`_mallocmc`/`_freemc`/`putchar` are already called register-style internally (by strdup/realloc/
_puts), proving the path.

## Hazards (must handle before marking `__fastcall`)
- **Shared/aliased entry points**: `__putc`/`_putchar`/`putchar` sit at one address with a single
  `(sp),y` shim. Converting `_putc` forces converting `putchar` too (mark both `__fastcall`, delete
  the one shim). Same idea for `rand`/`random` (aliased, but 0-arg so trivial).
- **Cross-header prototype conflicts** — reconcile before marking, or a mismatched redeclaration
  bites: `putchar` (void in stdio.h vs `int` in lib.h), `getchar`/`printf` return types differ
  (lib.h vs stdio.h), `strcmpi` declared `int*`, `strncmpi` declared `char*`, `lprintf` no return
  type in sys/oric.h.
- **Register-in-X, not A**: toupper/tolower/toascii + the ctype predicates end up with the char in
  X (via `(sp),y`→tax). A `char`→A `__fastcall` would hand them the char in A; the callee body
  then needs `tax` (or rewrite to read A). Cheap but not literally "delete the shim".
- **`_gets` / itoa / strcpy** register entries use zp pseudo-regs (tmp/op1/op2), not A/X — need a
  tiny A/X→zp adapter, or stay stack for now.

## Separate lever (not `__fastcall`): caller frame omission
Adding any call/local turns a leaf caller into a framed one (`ENTER` + pulls in enter/leave from
frame.s). `omit_frame` is only true when `localsize==6 && nbregs==0` (gen.c function()). So:
- a stack local (`char a`) → localsize>6 → frame;
- register vars live across a call → `nbregs>0` → frame.
A future "callee preserves reg0-7" contract (which hand-asm leaves like getchar already honor)
could let a caller that only calls such functions keep `nbregs==0` and stay frameless — a large
win, but its own design.

## Recommended order
1. Extend the lean call (no `ldy`) to `CALLW`/`CALLI`/`CALLB` (value-returning). Unlocks the ZERO
   bucket's saving.
2. Mark the ZERO functions `__fastcall` (free; add `(void)` to the K&R ones). ~25 funcs, −2 B/call.
3. Convert the cheapest REG1 (putchar family, malloc/free, toupper family, ctype predicates).
4. Later: the frame-omission lever; multi-arg (A/X/Y); C-defined `__fastcall` callee codegen.
