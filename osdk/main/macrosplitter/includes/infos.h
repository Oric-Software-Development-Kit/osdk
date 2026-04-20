
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

*/

#define TOOL_VERSION_MAJOR	0
#define TOOL_VERSION_MINOR	2

