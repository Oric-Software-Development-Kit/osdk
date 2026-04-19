@ECHO OFF


::
:: Initial check.
:: Verify if the SDK is correctly configurated
::
IF "%OSDK%"=="" GOTO ErCfg


::
:: Set the build paremeters
::
CALL osdk_config.bat

::
:: Generate the image
::
IF EXIST BUILD\NUL GOTO NoBuild
MD BUILD
:NoBuild
%OSDK%\bin\PictConv -f1 -d0 -o2 ..\..\data\picture.png BUILD\picture.bin


::
:: Launch the compilation of files
::
CALL %OSDK%\bin\make.bat %OSDKFILE%
GOTO End


::
:: Outputs an error message
::
:ErCfg
ECHO == ERROR ==
ECHO The Oric SDK was not configured properly
ECHO You should have a OSDK environment variable setted to the location of the SDK
GOTO End


:End
pause
