# Dead-Local Elimination — Design Brief

**Status:** SHIPPED (2026-07-25) — implemented as `eliminate_dead_locals()` in `dag.c`, one
pass at the top of `gencode()`, gated on `optimizelevel>0 && !glevel`. Test: `t_deadlocal.c`
(8 checks, green O1/O2/O3). Byte-identical output verified for functions with no eliminable
dead local. Grounded in a front/back-end feasibility map (2026-07-21); one new pass, no
dataflow framework needed.
**Author:** OSDK-Claude · **Date:** 2026-07-21
**Motivation:** the biggest remaining *size* lever for real programs — the `enter`/`leave` frame
overhead Mike flagged repeatedly (the `char a = getchar()` case).

---

## 1. TL;DR / the ask

A function goes *frameless* (no `ENTER`/`LEAVE`, and it doesn't pull the `enter`/`leave` runtime
in) only when it has no stack locals and no call-crossing register variables. A **local variable
that is written but never read** forces a stack slot for nothing. Detect such a local, **drop the
dead store** (keeping the right-hand side if it has side effects, e.g. a call), and **remove the
local from the frame**. If that was the only thing forcing a frame, the function becomes frameless.

This is the narrow, high-ROI slice of the frame-omission lever — it needs a whole-function
"is this local ever read?" check, **not** full control-flow liveness.

## 2. Problem — the observed frame overhead

`omit_frame` in `gen.c function()` is true only when **`localsize == 6 && nbregs == 0`**; otherwise
`ENTER(nbregs, localsize)` is emitted and the program links the `enter`/`leave` routines.

Measured (Opus session 2026-07-21):

| source | codegen |
|---|---|
| `getchar();` (result discarded) | `CALLVF_C(_getchar)` + `RET` — **frameless** |
| `char a; a = getchar();` (a unused) | `ENTER(0,7)` + `CALLWF_CD(...)` — **framed** |
| `char a; a = 5;` (a unused, no call at all) | `ENTER(0,7)` — **framed** |

So the unused `a` is the *sole* cause of the frame. The compiler assigns every auto local a stack
slot and never notices `a` is dead — even a dead store with no call pulls in the frame. If it
recognised `a` is never read, `a = getchar()` would compile like `getchar();` → frameless, and
`a = 5;` would vanish entirely.

## 3. The optimization

For each local `L` in a function:
1. **Classify** `L` as *read* (ever the value of a load) or *write-only* (only ever the target of
   a store). Write-only ⇒ dead.
2. For a dead `L`:
   - Each store `L = rhs` becomes just `rhs` evaluated **in void/side-effect context** (the RHS
     is kept — it may be a call or other side-effecting expression — but its value is discarded,
     exactly how a bare `f();` statement already compiles).
   - **Remove `L` from the frame** (don't allocate its slot; `localsize` doesn't grow for it).
3. If removing all dead locals brings `localsize` back to its base and `nbregs == 0`, the function
   is now frameless — no `ENTER`, no `enter`/`leave` link.

## 4. Why this slice (and not more)

- **Not full liveness.** "Is `L` read *anywhere* in the function" is a whole-function membership
  test, not a per-point live/dead dataflow. No CFG needed. LCC deliberately avoids dataflow; this
  stays within that.
- **Not partial dead stores.** `L = a; …; L = b; use(L)` (the first store is dead) needs real
  liveness — out of scope. v1 only kills locals that are *never* read at all.
- **Not register promotion of used locals.** Putting a *used* small local in `reg0-7` instead of
  the stack is the bigger, separate lever (real register allocation). Out of scope here.

## 5. Compiler-internal design

Grounded in a front/back-end feasibility map. The whole optimization is one new pass over the
already-built function code list; **no dataflow framework, no new IR.**

### 5.1 Insertion point
A new `eliminate_dead_locals()` called at the **top of `gencode()` (dag.c:154)**, before its
`Blockbeg`/`Local`/`Gen` dispatch loop. At that moment:
- the entire `codehead` list is built and untouched — every source local (in `Blockbeg.locals[]`
  and `Local` code nodes) and every statement dag (`cp->u.node`) is reachable in one place;
- it runs **before** `local()`/`allocreg()` assign any frame offset or zero-page register — which
  is what makes removal clean (see 5.4).

### 5.2 Classify reads vs writes (whole-function membership, not liveness)
Walk every `Gen/Jump/Label` node's dag forest (`cp->u.node`). For each node:
- a local is **read** when its `ADDRL` node (`node->syms[0]` is the local `Symbol`) is the child
  of an `INDIR` — i.e. `INDIR(ADDRL L)`. Build a `read` set of such locals.
- **Do not** classify by inspecting a single `ADDRL` node: `ADDRL` is hash-consed (`dag.c` `node()`),
  so one `ADDRL(L)` object is shared by a read and a write of `L`. Classify by **parent context**
  across the whole forest (any `INDIR` over `ADDRL L` ⇒ `L` is read).
- Treat `L->addressed` (address taken, `&L`), aggregates, and `volatile` as **read** (conservative).
- Consider only **source locals** (`sclass==AUTO`, in `Blockbeg.locals`/`Local`, `!temporary`) —
  never compiler temporaries, never parameters.

`ref`/`uses` are **useless here** — `ref` (set in `idnode`) is a weighted read+write frequency
count; a write-only local has `ref>0` and `initialized==1`, so neither `ref` nor the existing
`checkref` / `dag.c:190` gate detects a dead store. The `read` set is computed fresh by this scan.

### 5.3 Dead set (v1 restriction)
`L` is **dead** iff: source AUTO scalar local, **not** in the `read` set, not addressed/volatile,
**and** every store to it is a *statement-root* `ASGN(ADDRL L, rhs)` (the assignment's own result is
unused, `count==0`). This covers the target case — a local initializer `char a = getchar();`
lowers to a root `ASGN(a, CALL)`. **v1 skips** any `L` whose `ASGN` result is itself used
(`x = a = f()`): leave such `L` alone (correctness over completeness).

### 5.4 Transform
For each dead `L`:
1. **Drop the store, keep the side effects.** Replace each root `ASGN(ADDRL L, rhs)` (i.e. set
   `cp->u.node`) with `rhs` in **void context**:
   - side-effecting `rhs` (a `CALL`, or contains stores) → keep it as the new root with the
     top node's `count == 0`, so `needtmp()` (gen.c:635) rewrites a `CALL` to the void `CALLV`
     and allocates no result temp — exactly how a bare `f();` already compiles.
   - pure `rhs` (`a = b+1`, no side effects) → drop the whole sub-dag (mirror `root()`'s
     value-stripping, tree.c:199-201).
2. **Keep `L` off the frame.** Extend the existing `dag.c:190` gate
   `if ((*p)->ref>0 || (*p)->initialized || glevel) local(*p);` with a `&& !is_dead(*p)` predicate
   (and the same for `Local` nodes), so `local()` — the *only* thing that grows `offset`/`localsize`
   (gen.c:449-452) — is never called for `L`. Running before `allocreg()` also stops a hot dead
   local from claiming a `regN` and bumping `nbregs` (which independently forces `ENTER`).

Net: if `L` was the sole frame-forcing feature, `localsize` stays 6 and `nbregs` stays 0 →
`omit_frame` holds → no `ENTER`, and `enter`/`leave` isn't linked.

### 5.5 Why removal is safe/clean (timing)
Frame offsets are assigned **late** (backend `local()` during `gencode`), through a single gated
call site, and `localsize` is a pure running max. There is no earlier offset assignment to undo.
So the pass — inserted just before that gate — has full freedom to exclude a local.

## 6. Safety — the correctness argument

Dead-store elimination that is even slightly wrong = **silently dropped code**, so the bar is
"prove it, don't assume it":

- **Keep every side effect.** A store's RHS is kept and evaluated; only the *store* is dropped.
  `a = f();` → `f();`. Never drop the RHS.
- **Truly never read.** Only eliminate a local that is *never* the value of a load anywhere in the
  function (conservative membership test). Any read at all ⇒ keep.
- **Address-taken locals are off-limits.** If `&L` is taken (`Symbol.addressed`), `L` may be read
  through the pointer — treat as read, never eliminate.
- **`volatile` locals are off-limits.** Reads/writes have observable semantics.
- **Aggregates / structs** — conservative: v1 only touches scalar locals (or skip anything whose
  address escapes).
- **Parameters are not locals** — never touch incoming parameters (they're the caller's values);
  this is about *auto* locals only.

## 7. Non-goals (v1)

Partial/positional dead stores; register promotion of used locals; aggregates or address-taken
locals; `volatile`; parameters; anything requiring control-flow liveness.

## 8. Gate

- **Byte-for-byte identical** output for every function that has no eliminable dead local (the
  common case) — the pass must be a no-op there.
- A **known-answer test**: a function with a dead local that today emits `ENTER` becomes frameless
  (assert no `enter` reference / smaller output) **and** still produces the right result — the
  side-effecting RHS must still run (e.g. `a = getchar();` with `a` unused still consumes the key).
- Full compiler suite green.
- No silent drop: if v1 can't safely eliminate (address-taken, volatile, etc.), it leaves the code
  exactly as today.
