# Design notes: byte-granular zero-page slot allocator & 32-bit `long` support

**Status: DEFERRED — post-1.24.** Companion to `future-static-alloc-and-fastcall-design.md`;
this allocator is the foundation piece that document's param-bank/leaf-locals ideas also want.
Do not start until OSDK 1.24 is released and field-tested. Captures the design discussion of
2026-07-19 (Mike + Claude) so it can be picked up cleanly.

---

## Motivation

Two forces meet in the same zero-page bytes:

1. **OSDK has no 32-bit integer type.** `long` is 16-bit (`compiler/sources/types.c:44` maps
   `longtype` to `INT_METRICS`; there are no L-macro families in MACROS.H and no 32-bit lib
   routines). Exposed publicly by the ISS MOS6502 benchmark v2: the pi and mandelbrot samples
   compute garbage on OSDK (identical garbage on 1.23 — a long-standing limitation, not a 1.24
   regression), confirmed FAIL by the known-answer host-reference gate
   (`TestSuite/compiler/benchtable-1.24.md`, 19/22 PASS, both FAILs are 16-bit-`long` victims).
2. **Zero page is scarce and must not grow.** Hard requirement from Mike: no new zp addresses.
   The CRT block is 44 bytes (`lib/zp_crt.inc`): `ap/fp/sp` (6) + `tmp0-7` (16) + `op1/op2`
   (4) + `tmp` (2) + `reg0-7` (16).

Today's allocator hands out whole 2-byte slots regardless of width: a `char` temp burns 2
bytes (and the 1.24 8-bit codegen made char temps *common*), and a 32-bit value has no home at
all. Meanwhile user assembler already hand-packs the same bytes (see below). The answer to
both: keep the same 32 bytes, allocate them at **byte granularity**.

## The register file is ABI — names must survive

User assembler modules use the CRT registers *by name* as their local register file.
`D:\Git\Encounter\code\display.s:183` is the canonical example:

```
baseLinePtr   = tmp0   ; @ptr16
messagePtr    = tmp1   ; @ptr16
x_position    = tmp2
y_position    = tmp2+1          ; <- hand-rolled BYTE packing, in user space, today
...
fontPtr       = tmp7   ; @ptr16
scanlinePtr   = reg0   ; @ptr16 <- a reg drafted as a ninth scratch slot
```

Hard rules this imposes:

- **`tmp0-7`, `reg0-7`, `op1`, `op2`, `tmp`, `ap/fp/sp` remain defined equates** with their
  current meanings. User code references names (never absolute addresses — repo convention),
  so a *reorder* of `zp_crt.inc` is fine, but the names and their clobber semantics are frozen.
- **The tmp bank stays freely clobberable by asm called from C.** Compiled code must never
  keep a call-crossing value in tmp bytes (true today by the caller-saved convention; the new
  scheme pins it as the "floor" rule below).
- **The reg-as-scratch habit** (`scanlinePtr = reg0` without save/restore) works today only
  because no C caller up the chain happens to hold an enregistered value in `reg0` across the
  call. The new allocator must keep that *exact* risk level: reg-side bytes are allocated
  **strictly to call-crossing values** — never spill short-lived temps there "because there is
  room", which would raise the odds a reg byte is live when user asm clobbers it.

## Why a per-function floating tmp/reg boundary is unsound

tmp vs reg is not an address split, it is a **contract across calls**: callees clobber tmps
freely; callees save/restore any reg they touch. The contract works because every function
agrees where the boundary is. If F places a live-across-call value at byte 18 ("reg side" by
F's boundary) and calls G whose boundary is lower, G clobbers byte 18 as scratch — corruption.
G cannot know what F's ancestors hold live without whole-program analysis, which separate
compilation forbids. So "tmps from one end, regs from the other, meet in the middle" cannot be
a per-function secret.

## The sound version: fixed scratch floor + callee-saved-on-use above it

Replace the full partition with **one global constant**:

- **Floor = the tmp bank (16 bytes, pinned by the asm-interop rule).** Always scratch: any
  function (C or asm) clobbers freely, never saved. Only lifetimes that do not cross a call
  are allocated here.
- **Above the floor (the reg bank, 16 bytes): callee-saved-on-use.** Any function may use any
  of these bytes, but must save/restore the ones it touches — the existing `enter`/`leave`
  contract, at byte granularity.

The caller may then park a call-crossing value in *any* byte above the floor: whichever callee
touches that byte restores it. Mike's two-ended picture becomes the **allocation heuristic**
(not the contract): non-call-crossing lifetimes bottom-up into the floor (zero save cost),
call-crossing lifetimes top-down (each function's saved bytes form one contiguous top run, so
`enter`/`leave` stay a single "save top N bytes" count — `frame.s` extends from slot count to
byte count).

Elasticity falls out: a scratch-hungry expression overflows the floor by *saving* the extra
bytes it borrows from the upper region (cheap prologue/epilogue cost) instead of dying with
"expression too complex"; a call-heavy function fills the top with preserved values. No
cross-function coordination, no new zp, and with floor = 16 the scheme is a **strict superset
of today** — existing costs unchanged, new freedoms purely additive.

## The allocator

- Linear-scan over lifetime intervals (the existing tmpalloc discipline) against a 32-byte
  bitmap instead of an 8+8 slot list. Widths: char = 1, int/pointer = 2, long = 4, float = 5
  (if float temps ever move here — out of scope). **No alignment constraints** (the 6502 does
  not care; odd addresses fine) — confirmed intent.
- Operands are emitted as **base+offset expressions** (`tmp0+13`-style, or per-function
  equates). XA accepts expressions in operands and MACROS.H bodies already do `%1+1`
  arithmetic, so `sta %1+1` with `%1 = tmp0+13` expands correctly. (Pick ONE canonical base
  symbol per bank for emission, e.g. `tmp0+n` / `reg0+n`, so hashes/diffs stay readable.)
- **Prerequisite reorder:** `op1/op2/tmp` currently sit between the tmp and reg banks. Move
  them after `reg7` in `zp_crt.inc` so `tmp0..reg7` become one contiguous 32-byte run (pure
  rename-safe reorder; everything is `.dsb`-allocated and name-referenced). Not strictly
  required (the allocator can treat two disjoint ranges) but it simplifies overflow-with-save
  and the top-run save convention.
- **Sub-slot aliasing (later stage):** conversions become renames when lifetimes allow — a
  long→int truncate re-aliases the long's low 2 bytes; the byte-narrowing `CWB` re-aliases the
  low byte (the compiler-side equivalent of `y_position = tmp2+1`). Zero-copy, zero-cycle.
- `op1/op2/tmp` stay **runtime-private** (helpers need fixed addresses). Folding them into
  the pool via per-helper clobber sets remains a later idea (see the fastcall doc).

## 32-bit `long` on top of the allocator

The metrics change is one line (`types.c`: `longtype`/`unsignedlong` → `4,1,0`); everything
else is consequences:

1. **Backend width dispatch.** This lcc 1.9 IR does not encode size in the op name (`ADDI`
   serves int and long alike); every integer emit site in gen.c keys on the node type's
   `size == 4` and routes to the L path. Mechanical, touches many sites.
2. **Inline vs runtime split** (avoid a 764-macro-style explosion for L):
   - Inline: `INDIRL`/`ASGNL`, 32-bit constants, add/sub/and/or/xor/neg/com as 4-byte chains.
   - Runtime: `mul32`, `udiv32`/`umod32` + signed wrappers, variable shifts, compares —
     classic 6502 routines. **Zero-new-zp operand convention:** operand A in `op1:op2` (4
     contiguous bytes already), operand B *by pointer* in `tmp`, result in `op1:op2`.
3. **Conversions:** CVIL/CVLI etc. = zero/sign-extend and truncate (trivial; truncate becomes
   a rename once sub-slot aliasing lands).
4. **Constant handling:** the 1.24 "fold to 16-bit target width" fix becomes width-aware
   (longs fold at 32); lexer already carries 32-bit values on the host; define a macro-arg
   convention for 32-bit immediates (lo16/hi16 pair).
5. **Library:** `printf %ld/%lu` (uses `udiv32`); long↔float is the nasty corner — the ROM
   converts 16-bit only, so long→float composes two 16-bit converts (`hi*65536.0 + lo`);
   float→long is harder and may ship documented-limited in v1.
6. **Struct/sizeof impact:** any existing code using `long` changes meaning (4-byte fields,
   bigger arrays, slower ops — but *correct* now). Grep sweep of lib + samples needed; the pi
   sample's `long arr[561]` grows from 1122 to 2244 bytes — fine at $0800 load.

## Staging & validation (each stage gated before the next)

Assets already in place: the execution test suite (36 tests × -O1/2/3), the **host-reference
known-answer harness** (`scratchpad/hostref/`, rebuildable anywhere: MSVC `/D__HOST_C__` +
VM-array peek/poke), the ISS benchmark pipeline, Encounter as the real-world canary.

1. **Allocator rewrite in compat mode.** Byte-granular machinery forced to 2-byte widths and
   today's exact placement order. **Gate: the entire suite + benchmark + Encounter build
   BYTE-IDENTICAL.** Proves the rewrite alone changed nothing. (This gate is the whole reason
   to stage; the allocator replaces code every function flows through — the borrowed-temp bug
   family lived exactly here.)
2. **Widths on** (char=1, long=4) + the L emit paths + 32-bit runtime. Gates: suite green;
   known-answer gate — **pi and mandelbrot flip to PASS** (the acceptance test); cc65 suite
   long tests; Encounter plays; zp usage unchanged (same 44 bytes).
3. **Save-on-use overflow** above the floor (retires temp-pressure "expression too complex").
   Gate: suite + a new stress test with deep long expressions.
4. **Sub-slot aliasing** (zero-copy narrows/truncates). Gate: byte-diff shows only removals.

Effort honestly: stages 1+2 together are comparable to the whole 8-bit-char project plus the
lib audit; stage 2 is the long-pole (runtime routines + width dispatch). But every deferred
feature (fastcall param bank, static leaf locals, op consolidation) wants this allocator, so
it is built once, first.

## Open decisions

- Reorder `zp_crt.inc` (op island after reg7) — recommended yes, in stage 1.
- Emission spelling: `tmp0+n`/`reg0+n` expressions vs per-function equates (debuggability).
- `enter`/`leave` byte-count encoding (X = byte count of the top run seems free).
- v1 scope of `%ld` and float↔long.
- Whether 32-bit `unsigned long` division by constant gets strength reduction (later).
