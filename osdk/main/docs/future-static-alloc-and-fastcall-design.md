# Design notes: static local allocation & a `__fastcall`-style calling convention

**Status: DEFERRED — post-1.24.** Do not start until the OSDK 1.24 compiler is released and
field-tested. This is an ABI-touching feature; it must not destabilise a freshly-shipped
compiler. This doc captures the design discussion (2026-07-18) so it can be picked up cleanly.

---

## Motivation

On the 6502 the C software stack frame is *expensive*, in a way it isn't on a real CPU:

- A framed local is `(fp),N` — indirect-indexed, ~5 cycles, two of them for a 16-bit value,
  plus `enter`/`leave` prologue/epilogue.
- A statically-placed local is `lda var` — 4 cyc / 3 bytes absolute, or 3 cyc / 2 bytes in
  zero page.

So static allocation is roughly half the cycles and bytes *and* deletes `enter`/`leave` — a
**Pareto win** (smaller AND faster), the class we said should always be on. The C stack frame
exists only for **reentrancy** (recursion, or being called from an interrupt); a provably
non-reentrant function doesn't need it. Encounter already proves the payoff by hand — see its
`common.h`, where API functions are macros that stuff a fixed zero-page `param0/param1/param2`
bank and `jsr` an asm routine, with no frame at all; that made the code dramatically more compact.

## Key safety insight (specific to this runtime)

The C runtime's `tmp*`/`reg*`/`ap`/`fp`/`sp` zero-page pseudo-registers are **non-reentrant**.
Calling a C function from an interrupt would trash the interrupted C code's machine state — so
it simply isn't done (Encounter's whole IRQ path is asm using its own scratch). **Therefore no
C function is ever interrupt-reachable**, which means the *only* reentrancy hazard for
static-allocating a C function's frame is **recursion**. The interrupt-reentrancy half of the
usual analysis is off the table — a real simplification.

- **Leaf functions** (no `CALL` in their DAG) are trivially non-recursive and detectable
  **per-function** (no whole-program call graph). Safe starting set.
- Non-leaf non-recursion needs a real call graph + function-pointer conservatism (harder,
  cross-module).

## The chosen approach: an explicit calling-convention marker (like `__fastcall`/`__stdcall`)

Rather than have the compiler *prove* non-recursion (undecidable in general; function pointers
force conservatism; needs whole-program analysis), let the **programmer assert it** via a
per-function convention marker — exactly how `__fastcall`/`__stdcall`/`__pascal` work. This is
the primary design because it dissolves the hard part:

- **No analysis required** — the marker is the programmer's promise the function is non-reentrant.
  (Compiler MAY warn on obvious direct self-recursion as a courtesy; misuse is a user bug, like
  misusing `__fastcall`.)
- **Cross-module ABI solved for free** — the convention is part of the function *type*, so it
  travels with the prototype; every call site that sees the declaration knows to use the
  static-param convention. No whole-program coordination.
- **Opt-in ⇒ low risk** — unmarked functions keep the current VAX-frame convention untouched;
  adopt the marker function-by-function.
- **It formalises Encounter's `param0-2` macros into a type-checked feature** — write
  `__fastcall void Foo(char x, char y);`, call `Foo(x,y)`, and the compiler lays the args into
  fixed slots, generates caller writes + a frameless callee, *and type-checks the call*.

### Semantics
- **Callee:** params + locals at fixed addresses (static block), no `ENTER`/frame.
- **Caller:** write args to those fixed addresses, `jsr`. (The marker-in-the-type is what lets
  the call site know the layout.)

### Capacity — must fail LOUD, never silently
A fixed param bank has a hard byte capacity. If a `__fastcall` function's total param bytes
exceed it → **hard compile error** (model it on the existing "expression too complex" error,
which is the analogous fixed-resource-exhausted diagnostic). Do NOT silently fall back to the
stack — a function you *think* is frameless quietly not being so is exactly the silent failure
to avoid. (A loud warning + documented fallback is the only acceptable alternative, but hard
error is the default recommendation.) Bank size = a build-time constant; keep it modest
(zero page is scarce — see the ROM-coexistence note); Encounter's hand-rolled bank was 6 bytes
and covered its whole API, a good calibration point. Capacity is measured in **bytes**, not
param count (char=1, int/ptr=2, …).

### Shared bank vs per-function slots
- **One shared bank** (like `param0-2`): minimal zp. Safe for nested calls *only if* the
  compiler evaluates all args into temps first and writes the bank **immediately before the
  `jsr`** — then `Foo(Bar(x))` is fine (Bar consumes the bank and returns before Foo's args are
  written). Matches how LCC already evaluates args into temps. Capacity check = "largest single
  `__fastcall`'s params must fit." **Recommended starting point.**
- **Per-function slots:** nesting trivially safe regardless of codegen order, but more zp
  (unless overlay-colored across non-coexisting functions). Later refinement.

## The register-file question (verified against frame.s / MACROS.H)

Current zero-page pseudo-registers: `tmp0-7`, `reg0-7`, `op1`, `op2`, lone `tmp`, plus `ap`/`fp`/`sp`
(≈19 16-bit slots + 3 pointers). Their *real* roles:

- **`tmp0-7` = caller-saved** expression temporaries (a call clobbers them).
- **`reg0-7` = callee-saved** — `enter`/`leave` save/restore them (count = `nbregs`). This is a
  **genuine optimization**: a value live across a call sits in a `reg` and survives for free,
  instead of spilling to memory around every call. Not a VAX artifact — fundamental ABI.
- **`op1`/`op2`** = fixed **operand slots for the 16-bit math library** (mul/div/mod expect their
  operands there) + `enter`/`leave` scratch.
- **lone `tmp`** = scratch for the **shift routines** and `enter`/`leave`.
- `op1`/`op2`/`tmp` are **runtime-private** — kept separate so hand-written helper routines don't
  collide with the compiler's `tmp*`/`reg*` (which hold live values while a `MULI`/`LSHW`/frame
  setup runs).

**Crucial 6502 point:** all of these are zero-page bytes → **equal cost**. So the distinctions
are about **lifetime** (survives-a-call?) and **ownership** (compiler-allocated vs runtime-private),
NOT speed. Consolidation options, cheapest first:
1. **Fold `op1`/`op2`/`tmp` into the general pool** by teaching the compiler each helper's
   **clobber set** — reclaims ~3 slots (6 bytes of scarce zp). Moderate rework, real payoff.
2. **Merge `tmp*`+`reg*` into one bank** with an allocator-managed caller/callee boundary —
   functionally re-derives the same split (you still need callee-saved regs), so marginal unless
   the goal is a **smaller total count** (8+8=16 is generous; fewer = more free zp, more
   "expression too complex").
3. Treat the total register count as a **tunable knob**, validated against the suite's
   "expression too complex" rate.

## The overriding constraint: ROM coexistence (for the *generic* compiler)

Encounter/Nova own the whole machine (all zp + page 2 free), so none of this binds them. But the
generic Oric C compiler must **coexist with a live system ROM and call ROM services without
clobbering system variables.** Then the usable zero page is only the **complement of what the ROM
touches** under the allowed calls — a tight budget that *bounds* the register file + param bank.

- From the ROM disassembly: `$35-$84` is the **BASIC input buffer** (79 bytes); the `$35-$48`
  "CLOAD name", `$49-$5D`, tape-address slots are all "(V1.0 only)". So on the Atmos those tape
  roles are dead, but the **input-buffer role is live** — ROM number/string/float routines
  scribble there. So `$35-$48` is **not** safe for a ROM-coexisting register file.
- Note: the OSDK's current runtime base `$50` is **itself inside the input buffer `$35-$84`** —
  safe today only because OSDK programs (games) never invoke a ROM input/string routine. For the
  ROM-coexisting generic target, **the base itself needs re-deriving**, not just the extension.
- So the design is a **fit-to-budget** problem: (1) measure the ROM-safe zp window for a chosen
  set of callable ROM services, (2) fit `ap/fp/sp` + a lean (consolidated) register pool + a
  small param bank inside it. The win is having *any* zero-page register file + frameless leaf
  calls in that tight budget — not a luxuriously large one.

**Measure it, don't guess:** the `MEM_ACCESS_TRACER_SPEC.md` (in D:\Git\Encounter) proposes the
exact tool — arm read+write tracking on `$00-$4F`, run a program exercising the allowed ROM
calls, and the untouched set is the empirical safe budget.

## Incremental plan (each gated on: full known-answer suite build+execute + a real project build+run)

1. **Step 1 — leaf locals, no ABI change (safe, self-contained).** Leaf functions → static-allocate
   their `(fp),N` locals into an overlaid static block (only one leaf is live at a time → they can
   share one block sized to the largest leaf frame), drop `fp`. Params still via `(ap)`. Builds the
   leaf-detection + static-block machinery; zero caller-side change.
2. **Step 2 — `__fastcall` marker + static parameter passing (the ABI change, the big win).**
   Marked functions: params in the shared bank (write-before-jsr), frameless callee, hard error on
   over-capacity. This is your `param0-2` scheme, automated + type-checked.
3. **Step 3 — extend beyond leaves** (call-graph non-recursion) and **overlay coloring** to bound
   RAM/zp across the wider set; optional `reg*`/`tmp*`/`op*` consolidation.

## Open decisions to settle before coding
- Marker spelling: keyword (`__fastcall`), `__attribute__`, or `#pragma`.
- Param bank: shared (recommended) vs per-function; size constant; hard-error vs warn-fallback
  (recommended: hard error).
- Whether to consolidate `op1`/`op2`/`tmp` and/or shrink the register count as part of this.
- The ROM-safe zp map (measure with the tracer) → the concrete budget.
