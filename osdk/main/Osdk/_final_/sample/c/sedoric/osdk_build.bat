@ECHO OFF

::
:: Initial check.
:: Verify if the SDK is correctly configurated
::
IF "%OSDK%"=="" GOTO ErCfg


::
:: Set the build parameters
::
CALL osdk_config.bat


::
:: Build the main program (BUILD\SEDDEMO.tap)
::
CALL %OSDK%\bin\make.bat %OSDKFILE%
IF NOT EXIST build\%OSDKNAME%.tap GOTO End


::
:: Convert the picture to a TAP loading at $A000.
:: (-o1 directly outputs a TAP file without a BASIC loader)
::
%OSDK%\bin\PictConv -f1 -d0 -o1 ..\..\data\picture.png build\picture.tap


::
:: Build the SEDORIC floppy: each TAP becomes a disk file named after its
:: internal tape name (SEDDEMO -> SEDDEMO.COM, picture -> PICTURE.BIN);
:: the -i init string runs the program at boot. tap2dsk produces an
:: old-style image, old2mfm converts it in place to the MFM format that
:: emulators load.
::
%OSDK%\bin\tap2dsk -nSEDDEMO -iSEDDEMO build\%OSDKNAME%.tap build\picture.tap build\seddemo.dsk
%OSDK%\bin\old2mfm build\seddemo.dsk
GOTO End


::
:: Outputs an error message
::
:ErCfg
ECHO == ERROR ==
ECHO The OSDK variable is not defined, you should run the OSDK configuration tool
:End
