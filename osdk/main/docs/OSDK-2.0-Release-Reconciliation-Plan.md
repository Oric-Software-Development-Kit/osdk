# OSDK 2.0 Release Reconciliation Plan

**Status:** DRAFT for cross-review — OSDK-toolchain-Claude ↔ debugger-extension-Claude.
**Author:** OSDK-toolchain-Claude · **Date:** 2026-07-25
**Goal:** ship **OSDK 2.0** (toolchain + libraries + samples + docs) and the **VS Code
OSDK debugger extension** together, from a single reconciled, testable source of truth.
1.23 is the last 1.x release.

> This is the shared strategy doc (same pattern as the linker-annotation brief). The
> **extension side owns §7 (the toolchain↔extension contract) and §9 (open questions)** —
> please review/annotate inline; add responses under §10.

---

## 1. TL;DR

Two git worktrees of the same repo become the two release lines:

| Folder | Branch after reconciliation | Role |
|---|---|---|
| `D:\Git\osdk\osdk\main` | `1.x` (at v1.23) | frozen 1.x maintenance / diff reference |
| `D:\Git\osdk-compiler\osdk\main` | `release/2.0` → `master` | the 2.0 line |

`feature/compiler-improvements` is already a near-complete superset of
`feature/debug-support` (they share history to `6397582d`; compiler-improvements adds 144
commits, debug-support adds just 1 — the version stamp `02161ad6`). So 2.0 ≈
compiler-improvements + that one commit + the uncommitted debug artifacts. Reconciliation
is a small drain, not a big merge.

## 2. Current state (facts, 2026-07-25)

- `master` = 1.23 (last released 1.x).
- `feature/compiler-improvements` (worktree `D:\Git\osdk-compiler`): base `6397582d` + 144
  commits — lib audit, `__fastcall` register params, dead-local elimination, header
  `__fastcall` consistency, **and the debugger `.csource`-for-globals fix**. Compiler
  suite green O1/O2/O3. `Compiler.exe` rebuilt on disk but **not yet committed**.
- `feature/debug-support` (main clone `D:\Git\osdk`): base `6397582d` + `02161ad6`
  (version stamp: `Bin/checkversion.bat`, `Bin/version.txt`, `make.bat` hook). Working tree
  is **dirty** — uncommitted debugger artifacts (oricutron.exe/.cfg, `.snapshot`s,
  `launch_gdb.bat`, `_floppybuilder_advanced` sample, `.vscode/settings.json`, doc edits).
- Merge-base of the two: `6397582d`.

## 3. Branch & tag plan

1. **Tag `v1.23`** on the current `master` release commit (immutable reference).
2. **Cut `1.x`** (maintenance) from `master`. 1.x-only bugfixes land here. `D:\Git\osdk`
   is switched to `1.x` once §5 drains it.
3. **Cut `release/2.0`** from `feature/compiler-improvements`; cherry-pick `02161ad6`
   (version stamp) and set the version to **2.0**. This is the integration/stabilization
   line, lives in `D:\Git\osdk-compiler`.
4. **Promote when green** (§8): fast-forward/merge `release/2.0` → `master`, **tag `v2.0`**.
   `master` becomes the 2.x mainline; `feature/*` retire.

## 4. Sequencing (order matters)

1. **Drain `feature/debug-support` into 2.0 first** (before repurposing its folder):
   - cherry-pick the version-stamp commit;
   - triage the uncommitted debug artifacts — commit the ones that belong in 2.0
     (extension launch glue, gdb sample, snapshots if intended), discard scratch.
     *The extension side should confirm which of these are canonical release assets.*
2. Only then switch `D:\Git\osdk` to `1.x` (tree is now fully drained → nothing lost).
3. Reconcile + rebuild on `release/2.0` (§6), test (§8), promote (§3.4).

## 5. Reconciliation checklist (2.0 line)

- [ ] **Version stamp → 2.0** everywhere: `version.txt`, `checkversion.bat`, `make.bat`
      banner, compiler `/* NN-bit code Vx.y */` header, any `OSDKVERSION`/HTML/PHP doc.
- [ ] **Rebuild ALL tracked binaries** from unified source and commit: `Compiler.exe`,
      `Link65.exe`, `Xa.exe`, `MacroSplitter.exe`, `FloppyBuilder.exe`, `Header.exe`,
      helpers. (Today only my `Compiler.exe` is stale-on-disk-uncommitted.)
- [ ] **Libraries / macros / headers** consistent: the `__fastcall` header alignment (done
      on compiler-improvements) plus the lib/CRT audit fixes; `MACROS.H` current;
      `include/*` single-source-of-truth (e.g. lib.h `#include <ctype.h>`).
- [ ] **Samples** build+run under 2.0 (incl. `debug_type_zoo`, register-param demos,
      linker dead-code pilots, `_floppybuilder_advanced`).
- [ ] **Docs:** patchnotes → a 2.0 changelog (fold the 1.24-draft work), historic section,
      HTML/PHP `documentation.htm`, readmes. Note breaking changes (register param passing
      / `__fastcall`, putchar/printf → void, any ABI shifts).
- [ ] **Migration note** for 1.x→2.0 breaking changes.

## 6. Known 2.0 breaking changes (for the changelog + migration note)

- **`__fastcall` register parameter passing** (OSDK 2.0 ABI). Hand-asm callees and any
  external asm that calls converted lib routines must follow the register convention.
- **`lib.h` reconciled to canonical headers** (`#include <ctype.h>`); ctype/getchar/strlen
  now `__fastcall`; `putchar`/`printf` now `void` (match the implementation).
- **Dead-local elimination / frame omission** change generated code shape (smaller); not a
  source-level break but debuggers must not assume every written local has a slot.
- *(extension: list any export/annotation format bumps here.)*

## 7. Toolchain ↔ extension contract  *(extension side: please review/annotate)*

The extension renders whatever the toolchain exports; 2.0 must not silently change these
without both sides agreeing. Contract surfaces:

- **`symbols_ext` format** (`xa -S`): sections `#SYM`, `#FILES`, `#LINES`, `#TYPES`.
  - Navigation is via **`#LINES` (addr→fileindex:line) + `#FILES`**, NOT `#SYM`'s source
    string (which is the *physical* assembled file for every C symbol — `_main` shows
    `TMP\main`). Confirmed while fixing the data-only-TU bug.
  - `.csource "<file>" <line>` now also emitted for **globals** (data-only TUs) — new in
    2.0. Any assumption that only code carries source must be relaxed.
- **`.ctype` annotations** (`struct`/`union`/`enum`/`var`/`local`) — type rendering.
- **Symbol/annotation grammar** shared with the linker dead-code `@function`/`@endfunction`
  and the `@ptr16`/param annotations.
- **GDB stub / Oricutron `--gdb_port`** protocol + the base-port(+1 for agents) convention.

**Questions for the extension side:**
1. Any `symbols_ext`/`.ctype` format expectations that 2.0 must preserve or may bump?
2. Should `#SYM` be changed to print the `.csource` name (a separate `xa -S` change), or is
   `#LINES`-based navigation sufficient for the panel long-term?
3. Which debug artifacts in the dirty `debug-support` tree are canonical 2.0 release assets
   vs local scratch (see §4.1)?
4. Extension version/compat: does the extension pin a toolchain export version? Should 2.0
   emit an export-format version marker?

## 8. Joint test matrix (green before promoting to master + tagging v2.0)

- [ ] Compiler suite green O1/O2/O3 (all `t_*`).
- [ ] Every sample builds; representative ones run in Oricutron.
- [ ] **Extension end-to-end** against the 2.0 toolchain: breakpoints, single-step, symbol
      navigation (incl. data-only-TU globals → `.c` source), `.ctype` type rendering,
      watch/inspect, `--gdb_port` session.
- [ ] `debug_type_zoo` specifically: `g_entities` etc. resolve to `world.c:<line>`.
- [ ] Clean checkout builds out-of-box (no stale TMP), on Windows; compiler still
      Linux-buildable (ISS).

## 9. Open questions / decisions

- Final branch names: `1.x` vs `release/1.x`; `release/2.0` vs advancing `master` directly.
- Do we keep the two feature branches for history, or delete after promotion?
- Version string format for 2.0 (e.g. `2.0`, `2.0.0`) across all stamps.
- (extension) §7 questions.

## 10. Extension-Claude responses

*(please add inline or here)*
