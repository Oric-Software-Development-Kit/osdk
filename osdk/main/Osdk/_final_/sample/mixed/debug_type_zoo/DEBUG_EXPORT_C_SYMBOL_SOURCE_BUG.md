# Bug report: debug export attributes C symbols to the TMP intermediate, not the `.c` source

---

## ✅ RESOLVED (compiler side, 2026-07-25)

**Root cause (confirmed):** the compiler emitted its `.csource "<file>" <line>`
directive only for **functions and executable statements** (function prologue +
each execution point). `world.c` is **data-only** (globals, no functions), so it
emitted **no `.csource` at all** — its real name never entered the assembly, and
xa's `-S` fell back to the physical intermediate it read (`%OSDKT%\world`).
`main.c` has functions, so it emitted `.csource "...main.c"` and was registered.

**Fix:** `defglobal()` (compiler `init.c`) now emits `.csource "<src>" <line>` for
each file-scope global when `-g` is on — the same directive functions use — so a
data-only TU registers its source too. Guarded by `glevel`, so non-debug builds
are byte-for-byte unchanged.

**Verified** on `debug_type_zoo` `build/symbols_ext`:
- `#FILES` now lists `3  …\world.c` (a real file, no longer `TMP\world`).
- `#LINES` now maps each global's address to `world.c`, e.g. `_g_entities` at
  `0e47` → `3:10` (file 3 = world.c, line 10) — was `7:3` (TMP\world).

**One clarification for the debugger side:** the `#SYM` section's source string
uses the **physical assembled file for _every_ C symbol** — `_main` itself shows
`…\TMP\main:423`, not `main.c`. That is why `#SYM` was never the navigable
signal; the extension already resolves source via `#LINES` → `#FILES` (which is
how `main.c`'s functions were clickable despite `TMP\main` in `#SYM`). The fix
targets `#LINES`/`#FILES`, so `g_entities` now resolves to `world.c:10` and
becomes clickable. `#SYM` showing `TMP\world` is cosmetic and identical to how
`main.c` symbols already appear; making `#SYM` print the `.csource` name instead
would be a separate xa `-S` change, not required for navigation.

---


**For:** whoever owns the OSDK C compiler / `-S` extended-symbol export (this
`osdk-compiler` tree, `feature/compiler-improvements`).
**From:** the VS Code / Oricutron debug side. This is a **toolchain export** issue, not
a debugger bug — the extension faithfully renders whatever the export says.
**Reproducer:** `sample/mixed/debug_type_zoo` (`world.c`, `main.c`, `asm_data.s`).

---

## Symptom

In the VS Code debugger's **Oric Symbols** panel, an assembly global like
`_g_asm_ticks` is underlined/clickable and opens its real source
(`asm_data.s:14`). A **C global** like `g_entities` (defined in `world.c:10`) is **not
clickable** — the extension has no source location for it.

The extension makes a symbol navigable only when the export gives it a **real source
file**. For the affected C symbols the export gives a *build artifact* path instead.

## Root cause (from `build/symbols_ext`)

The extended symbol export records the affected C symbols against the compiler's
**throw-away intermediate** `%OSDKT%\world` (the generated `.s` from `world.c`, no
extension, deleted after the build) instead of `world.c`.

`#SYM` (address, name, source):

```
1261 _g_asm_ticks  ...\debug_type_zoo\asm_data.s:14     <- real .s source          OK
11ea _g_entities   ...\TMP\world:2                       <- TMP intermediate        BUG
123d _g_score      ...\TMP\world:60                      <- TMP intermediate        BUG
123f _g_seed       ...\TMP\world:62                      <- TMP intermediate        BUG
1241 _g_total_xp   ...\TMP\world:64                      <- TMP intermediate        BUG
123a _g_current    ...\TMP\world:56                      <- TMP intermediate        BUG
```

`#FILES` (file index -> path) — note **`main.c` is present as a real `.c`, but
`world.c` is NOT: it appears only as `TMP\world`**:

```
2  ...\debug_type_zoo\main.c        <- real C source (correct)
7  ...\TMP\world                    <- world.c's intermediate (WRONG; should be world.c)
8  ...\debug_type_zoo\asm_data.s    <- real asm source (correct)
9  ...\debug_type_zoo\asm_helpers.s
```

`#LINES` (address -> file:line) confirms it for the data address too:

```
11ea 7:3      <- g_entities's address -> file 7 (TMP\world) line 3, not world.c:10
```

The **type** info is correct — only the **source location** is wrong:

```
var _g_entities Entity[4] 80
```

## The key lead: `main.c` is fine, `world.c` is not

Both are C files in the same project, compiled by the same `make.bat` pipeline
(`cpp -> compiler.exe -> cpp -> macrosplitter -> %OSDKT%\<name>`), yet:

- `main.c` keeps its identity — it appears in `#FILES` as the real `main.c`.
- `world.c` loses it — it appears only as `%OSDKT%\world` (the intermediate).

So the defect is **inconsistent between translation units**. That strongly suggests
the `#file` / source-name directive that xa's `-S` export reads is emitted correctly
for one C file but not the other (e.g. correct for the first/primary C module, wrong
or missing for subsequent ones — or it uses the *output/temp* name instead of the
original `.c` for some modules).

## Where to look

1. **Generated intermediate `.s`.** Compare `TMP\main` vs `TMP\world` for their
   `#file "…"` / line directives. (TMP is wiped each build — preserve it: comment out
   the `RMDIR /s /q %OSDKT%` in `bin/make.bat`, or run the C→asm steps by hand.)
   Expectation: `TMP\main` carries `#file "…main.c"` while `TMP\world` carries the
   wrong name (or none, so xa falls back to the physical filename `world`).
2. **Compiler emit** (`compiler.exe` / `gen.c`) — how it writes the source-file
   directive / `__FILE__` for each module. A module that emits the temp/output name
   instead of the original `.c` would produce exactly this.
3. **Preprocess step** (`cpp`) — the `# <line> "file"` line markers it emits; if the
   original filename is lost/rewritten for `world.c`, xa records the temp file.
4. **xa `-S` export** — how it resolves a symbol/line's source file (physical file it
   assembled vs a `#file` directive). It already honours `#file` for `.s`/library code
   (see `linked.s` `#file` markers), so the fix is likely to make the C path feed it a
   correct `#file "<orig>.c"`.

## What "correct" looks like

- `#SYM`: `_g_entities  …\world.c:10` (like `_g_asm_ticks  …\asm_data.s:14`).
- `#FILES`: contains `…\world.c` (not `…\TMP\world`).
- `#LINES`: `g_entities`'s address maps to `world.c:<line>`.

## How to verify a fix

Rebuild `debug_type_zoo`, then in `build/symbols_ext`:
- `grep world _final_/…/build/symbols_ext` should show `world.c`, never `TMP\world`.
- Every `_g_*` from `world.c` should end in `…\world.c:<line>`.

In VS Code (Oric Symbols panel): `g_entities` should become underlined/clickable and
open `world.c` at its definition, exactly like `_g_asm_ticks` opens `asm_data.s:14`.

## Notes / scope

- Evidence above is from a build whose `%OSDK%` was `D:\Git\osdk\…\_final_` (paths show
  that tree + its `TMP`); the defect is in the C-compiler/export pipeline, so it
  reproduces wherever these tools build a multi-`.c` project.
- The debugger side needs **no** change once the export is correct — the extension
  already renders real `.c:line` locations as clickable (proven by `asm_data.s` and by
  C source-line breakpoints). No debugger-side workaround is clean here because the
  export currently contains **no** `world.c` reference to map back to.
