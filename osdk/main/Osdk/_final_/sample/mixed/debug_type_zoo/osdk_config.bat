@ECHO OFF

::
:: Set the build parameters
::
SET OSDKADDR=$800
SET OSDKNAME=DBGZOO
SET OSDKFILE=main world asm_data asm_helpers

:: -g1 makes the C compiler emit source-line (and type) debug info, so the extended
:: symbol file (build\symbols_ext, produced by make.bat's xa -S) maps C code back to
:: the .c source for breakpoints/stepping in the VS Code debugger.
SET OSDKCOMP=-O2 -g1
