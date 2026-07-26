
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

1.0
- First version considered production rather than experimental, hence 1.0 rather than 0.3.
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

*/

#define TOOL_VERSION_MAJOR	1
#define TOOL_VERSION_MINOR	0

