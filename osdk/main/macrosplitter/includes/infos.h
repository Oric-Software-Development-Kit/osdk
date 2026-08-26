
/*
Change history for MacroSplitter

0.1
- Initial version, splits semicolon-separated instructions into separate lines

0.2 - 2026/03/16
- Added -O flag for optional 6502 peephole optimization
- Self-store elimination (lda X : sta X -> removes the redundant sta)
- Load-after-store elimination (sta X : lda X -> removes the redundant lda)
- Dead store elimination (sta X : sta X -> removes the first redundant sta)
- Tail call optimization (jsr X : rts -> jmp X, removes rts, skipped if preceded by conditional branch)
- Cross-register transfer (stx X ... lda X -> txa, and similar combinations)
- Same-immediate reload elimination (lda #X : sta Y : lda #X -> removes the redundant lda)
- Resolves #<(N) and #>(N) constant expressions for numeric literals
- Works with lda/sta, ldx/stx, and ldy/sty register pairs
- Added -M flag for built-in macro expansion (replaces cpp.exe step)
- Emits annotation comments showing original macro calls before each expanded block
- Supports all MACROS.H definitions including nested expansion up to 16 levels
- Parenthesis-balanced argument parsing for args like (ap), (fp), (sp)
- New OSDKMACROEXPAND=1 environment variable in osdk_config.bat enables the new pipeline

1.0 - 2026/07/27
- First version considered production rather than experimental, hence 1.0 rather than 0.3.
- The -g1 debug directives (.csource / .ctype) are now transparent to the optimizer. They were
  treated as ordinary directives, and a directive is a barrier, so with a .csource sitting
  between almost every pair of generated instructions most peephole patterns never matched:
  a debug build could be several percent larger than the same source without -g1 (aes256 lost
  591 bytes). They carry no code and no size, so they are classified as comments now, which
  every "skip transparent tokens" scan already understands.
- Peephole: CFG-liveness dead temp-store elimination. A store to a compiler temporary whose
  value is not read on any path out of the block is removed outright.
- Peephole: dead immediate and constant register staging is eliminated, and a byte-select of
  a known zero (#<(0) / #>(0)) is folded to a plain immediate.
- Peephole: commutative staging fold, so "lda X : sta T : lda Y : eor T" becomes
  "lda Y : eor X" for the commutative operations.
- Peephole: transfer followed by store becomes a direct store, and the temp-dead-after test
  was hardened for register pairs.
- Peephole: dead op1:op2 long-load staging is sunk to the destination temporary.
- Removed the blank lines left behind where instructions were deleted, so the generated
  assembly stays readable when inspecting what the optimizer did.

1.1
- Peephole: fixed a silent miscompile in the commutative staging fold. It asked whether the
  scratch temp was dead with a straight-line scan that stopped at the first branch and
  reported "dead", so in "if ((t & a) || ((t & b) == c))" the reload in front of the second
  test was dropped while only the first read was rewritten: the second test then read the
  first test's result. Byte-typed locals are what expose it (the word path reloads through
  CSBW and overwrites the stale value first), which is how an unsigned char cell type made
  every open door read as shut in Oric DungeonMaster. The fold now asks the CFG liveness
  analysis, which is what the dead-store pass already used.
- Peephole: the carry-branch fold used the same straight-line assumption, dropping the store
  of the AND result to a temp with no liveness test at all. It now checks.
- Peephole: branch targets are resolved with the assembler's own scoping rules. Labels were
  keyed by name in a file-wide map, so the 410 reuses of "skip" in MACROS.H all collapsed onto
  whichever came last and every "bne skip" got an edge to the wrong block - wrong liveness in
  both directions. A label in a ".( .)" block is now looked up in that block and then outward,
  and a cheap local label ("@name") in the scope of its nearest preceding standard label; an
  unresolvable or ambiguous name makes the block opaque instead of guessing. Recovering that
  precision more than pays for the two stores the fixes above keep.
- New TestSuite/compiler test t_peephole.c covers byte locals read either side of a
  short-circuit branch (run the suite with -Peephole).
- The optimizer now reports the commutative staging folds and dead temp stores it removes at
  OSDKVERBOSITY=3. Both were silent, which is why the miscompile above was hard to place.

*/

#define TOOL_VERSION_MAJOR	1
#define TOOL_VERSION_MINOR	1

