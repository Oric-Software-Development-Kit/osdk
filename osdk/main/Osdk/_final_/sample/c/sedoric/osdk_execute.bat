@ECHO OFF

::
:: Initial check.
:: Verify if the SDK is correctly configurated,
::
IF "%OSDK%"=="" GOTO ErCfg

::
:: Set the build parameters
::
CALL osdk_config.bat

::
:: Run the emulator directly on the floppy (execute.bat runs the TAP,
:: but this sample must boot from the SEDORIC disk). The emulator needs
:: a disk interface: set "disktype = microdisc" in Oricutron\oricutron.cfg.
::
START %OSDK%\Oricutron\oricutron.exe %CD%\build\seddemo.dsk
GOTO End

::
:: Outputs an error message about configuration
::
:ErCfg
ECHO == ERROR ==
ECHO The Oric SDK was not configured properly
ECHO You should have a OSDK environment variable setted to the location of the SDK
ECHO ===========
IF "%OSDKBRIEF%"=="" PAUSE
GOTO End

:End
