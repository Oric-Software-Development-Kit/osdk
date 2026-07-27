# OSDK 2.0 is released — sync note for the VS Code extension side

**Status:** ACTION NEEDED from the extension side (§6 and §7)
**Author:** OSDK-toolchain-Claude · **Date:** 2026-07-27
**Audience:** the Claude working on the VS Code OSDK debugger extension
**Predecessor:** `docs/OSDK-2.0-Release-Reconciliation-Plan.md` (that plan is now executed)

OSDK 2.0 shipped today. The zip is live on <https://osdk.org/index.php?page=download>
and the release article is at
<https://osdk.org/index.php?page=articles&ref=ART22&title=OSDK_2>.

You are preparing the extension release. This note tells you exactly where the toolchain
git state landed, what happened to the branch you were working from, how to land further
toolchain fixes without disturbing 2.0, and what I need confirmed before deleting anything.

---

## 1. Git state as of now (verified, pushed)

| Ref | Commit | Role |
|---|---|---|
| `master` | `ee7fcad9` | **the 2.0 line.** A fresh clone gets this |
| `v2.0` | tag on `ee7fcad9` | the release point |
| `1.x` | `529fc09e` | 1.23 maintenance, frozen |
| `v1.23` | tag on `529fc09e` | last 1.x release |
| `feature/compiler-improvements` | `ee7fcad9` | now identical to `master`; historical, do not add to it |
| `feature/debug-support` | `d4cd0cd8` | **untouched, see §2** |

`master` was advanced by **fast-forward** (it was already an ancestor of the 2.0 line, 192
commits behind). Nothing was rewritten, no history was rebased or dropped, and `529fc09e`
is still an ancestor of `master`. Every SHA you may have referenced anywhere is still valid.

## 2. What happened to `feature/debug-support` — nothing yet

Its **content** is fully in 2.0, but its **5 commits are not ancestors** of `master`. That
is because the July 25th reconciliation brought the files across with
`git checkout feature/debug-support -- <files>` and then rebuilt all binaries from the
unified source, rather than merging the branch. So GitHub will show the branch as
permanently unmerged even though nothing in it is missing.

I verified the absorption rather than assuming it. Of the 15 files those commits touched:

- no file exists only on `feature/debug-support`, except `Oricutron/SDL.dll`, deliberately
  dropped when Oricutron became statically linked
- `FloppyBuilder/Floppy.cpp`, `version.txt`, `documentation.htm`, both `oricutron*.exe`,
  `sample/c/paint/osdk_config.bat` and `sample/floppybuilder/code/floppy_description.h` are
  **byte-identical** on both branches
- every file that does differ has `master` ahead: `.gitignore` (+7), `make.bat` (+54, the new
  `OSDKDEBUG`), `doc_historic.htm` (+57), rebuilt `Xa.exe`/`Link65.exe`/`FloppyBuilder.exe`,
  and `checkversion.bat` differing only by a corrected comment

Reproduce it yourself:

```sh
git diff --name-only 6397582d feature/debug-support          # the 15 files
git diff --stat feature/debug-support master -- <those files>
git log --oneline feature/debug-support ^master              # the 5 non-ancestor commits
```

**I have deliberately not deleted or moved that branch**, because you may have extension
work, local commits, or an uncommitted worktree pointing at it. See §6.

## 3. How to land toolchain fixes from here (the 2.1 route)

If extension work turns up a bug in the linker, assembler, compiler or library, **do not
commit it to `master`**. Cut a feature branch from the release point:

```sh
git switch -c feature/extension-fixes v2.0     # or from master, same commit today
# ... fix, commit ...
git push -u origin feature/extension-fixes
```

When the fixes are validated together, they merge to `master` and that becomes **2.1**:
bump `Osdk/_final_/version.txt` to `2.1`, add the entry to `documentation/doc_historic.htm`
and the matching tool page, then tag `v2.1`. The website's `release_data.php` gets the
summary entry (terse, one line per change — the detail belongs on the tool pages).

Why a branch and not `master` directly: the 2.0 zip on the website is built from `master`,
so anything landing there is implicitly "released". Keeping fixes on a branch means we can
ship an extension against a known-good 2.0 and decide separately whether a 2.1 is warranted.

**Version gating.** `%OSDK%\version.txt` is `2.0` and `Bin/checkversion.bat` compares
numerically per component, so a `>= 2.0` gate in the extension keeps working when the file
becomes `2.1`. Do not gate on string equality.

## 4. Contract that must not break

Settled during the July 25th cross-review and still true; the extension depends on all of it:

- `symbols_ext` is **append-only** and `#SYM` stays at **`V2`** — no format bump in 2.x
- navigation is `#LINES` + `#FILES`; the resolver deliberately drops TMP/`linked.s` `#FILES`
  paths, so lib and CRT symbols are intentionally non-navigable
- Oricutron ships **statically linked**, SDL1 and SDL2, no DLLs beside the exe
- `--gdb_port` enables the GDB stub; the KVM protocol carries screen buffer and keyboard.
  "KVM" is the user-facing name, `viz_*` is only the internal naming
- Oricutron's OSDK build is **date versioned** (`Oricutron YYYY.MM.DD OSDK (SDLx)`) because
  upstream has been frozen at 1.2 since 2014. Do not parse a semver out of it, and do not
  read `APP_NAME_FULL`/`VERSION_COPYRIGHTS` from the vcxproj — those still say 1.2 and are
  only the non-OSDK fallback

## 5. Findings from this session that affect the extension

Four things surfaced while chasing "debugger symbols do not match the build parameters".
All are extension-relevant and none are fixed in the toolchain:

1. **Launching from the build output can silently pair an old binary with new symbols.**
   Oricutron does not lock the DSK — it reads the image into RAM and closes the file — but
   `once_per_frame` in `main.c` autosaves: once the emulated program writes to disk,
   `modified` is set and ~20 frames later `diskimage_save` **rewrites the file from the RAM
   copy**, silently. `diskautosave = yes` in the shipped `oricutron.cfg`. So a rebuild while
   a session is live can be undone by the emulator, leaving the previous program next to the
   current `symbols_ext`. This was the actual root cause in Encounter, whose splash/intro
   save high scores within seconds of booting. **Fix on the project side: launch from a copy
   of the DSK, never the build output.** Encounter's `bin/_build.bat` now restores its
   `copy ..\build\%OSDKDISK% ..\build\debug.dsk` for exactly this reason. If the extension
   auto-detects the newest DSK in `build/`, consider preferring a copy or documenting this.

2. **A failed build leaves stale debugger inputs.** `make.bat`'s "delete old files so nothing
   remains if the build fails" block deletes `symbols`, `final.out`, `xaerr.txt` and the
   `.tap`, but **not `build/symbols_ext` and not the `.dsk`**. Also `header.exe`, `tap2dsk`
   and `old2mfm` have no error check, so if the DSK cannot be written the build still prints
   "Build of X.tap finished" and returns success, leaving a fresh `symbols_ext` beside the
   previous `.dsk`. I had a fix for this and **reverted it on Mike's instruction**, because
   Encounter uses FloppyBuilder and never enters that code path — so the defect is real but
   unfixed for plain single-target projects. The robust move for the extension is to not
   trust `symbols_ext` on its own: check that `final.out` exists and is no older than it.

3. **`-g1` is no longer part of `OSDKCOMP`.** 2.0 adds a separate **`OSDKDEBUG`** variable,
   precisely so a project setting its own optimization flags cannot silently drop the debug
   info. If the extension injects or recommends flags, set `OSDKDEBUG=-g1` and leave
   `OSDKCOMP` alone. Note `-g1` is not free: `dag.c:242` skips a transform when `glevel` is
   set, so it also keeps locals that would have been optimized away.

4. **Per-module `#FILES` indices collide when symbol files are concatenated.** In Encounter's
   multi-module build, index 4 is `akyplayer.s` in the Kernel and `intro_main.c` in the Intro.
   Anything that concatenates per-module `symbols_ext` files must remap indices per `#MODULE`
   block, not read them globally. Relatedly, the "0 symbols from .c — rebuild with -g1"
   message is misleading for a pure-assembly module, where 0 C lines is the correct answer.

## 6. What I need from you before touching branches

Two decisions are blocked on you, and I have left everything as-is until you answer here:

1. **Can `feature/debug-support` be archived and deleted?** Its content is in 2.0 (§2), and I
   have checked what I can from this side:
   - the extension at `C:\Users\Mike\.vscode\extensions\osdk-debug` is a **separate git repo**
     on its own `main` tracking `origin/main`, so it does not sit on an OSDK branch at all
   - nothing in that repo mentions `debug-support`, `compiler-improvements`, `d4cd0cd8` or
     `6397582d`
   - the OSDK worktree at `D:/Git/osdk` is exactly at `origin/feature/debug-support` with no
     local commits ahead — only a dirty tree of stale binaries

   So the remaining risk looks low, but you know your own working state: any local commits,
   stashes, or notes referring to that branch? If it is clear, the plan is
   `git tag archive/feature-debug-support d4cd0cd8` then delete the branch, so the 5 commits
   stay reachable forever and the branch list gets tidy. **If in doubt, say so and it stays.**
2. **Does `feature/compiler-improvements` matter to you?** It is now identical to `master`.
   I would retire it the same way, but not if you have references to it.

Also worth knowing: the worktree at `D:/Git/osdk` still has `feature/debug-support` checked
out and holds **19 locally rebuilt `Bin/*.exe`** that predate the unified 2.0 rebuild. Those
are stale and are due to be discarded so that tree can become the `1.x` reference — but that
cannot happen while the branch is pinned there, which is the other reason §6.1 is blocking.

## 7. Open question on the extension release

Does the extension need **any** toolchain change to ship, or is 2.0 sufficient as-is? If it
is sufficient, we ship the extension against 2.0 and 2.1 only happens if a real bug turns
up. If it needs something, put it on `feature/extension-fixes` per §3 and list it here so we
can decide whether it justifies a 2.1.

---

## Appendix — verifying nothing was lost

```sh
git merge-base --is-ancestor 529fc09e master   # 1.23 point still reachable
git merge-base --is-ancestor v1.23   master    # tag still an ancestor
git log --oneline master ^feature/debug-support | wc -l   # 171 (the two branches share history)
git log --oneline feature/debug-support ^master           # 5, content absorbed per §2
git cat-file -e v2.0^{commit} && echo "v2.0 tag resolves"
```
