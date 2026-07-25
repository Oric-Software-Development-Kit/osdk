@ECHO OFF
::
:: checkversion.bat <minimum-version>
::
:: Compares the installed OSDK version (read from %OSDK%\version.txt) against the
:: <minimum-version> passed by the caller. OSDK versions are major.minor (e.g. 2.0);
:: the comparison is NUMERIC per component, so 2.0 > 1.24 and 12.0 > 2.0 (no string
:: pitfalls). A project's osdk_build.bat calls this to require a minimum OSDK.
::
:: Exit codes:
::   0 = OK          installed version >= minimum
::   1 = too old     installed version <  minimum
::   2 = unknown     %OSDK% not set, or version.txt missing/empty
::   3 = usage       no minimum version argument given
::
:: An OSDK too old to contain this file simply doesn't have it; the caller is expected
:: to treat a missing checkversion.bat as "too old" (graceful on old OSDKs).
::
SETLOCAL EnableDelayedExpansion

IF "%~1"=="" (
    ECHO [checkversion] ERROR: no minimum version specified.
    ENDLOCAL & EXIT /B 3
)
SET "REQ=%~1"

IF "%OSDK%"=="" (
    ECHO [checkversion] ERROR: the OSDK environment variable is not set.
    ENDLOCAL & EXIT /B 2
)
IF NOT EXIST "%OSDK%\version.txt" (
    ECHO [checkversion] ERROR: "%OSDK%\version.txt" not found - OSDK too old to report a version.
    ENDLOCAL & EXIT /B 2
)

SET "CUR="
SET /P CUR=<"%OSDK%\version.txt"
IF "!CUR!"=="" (
    ECHO [checkversion] ERROR: "%OSDK%\version.txt" is empty.
    ENDLOCAL & EXIT /B 2
)

:: Split both "major.minor" into numeric components (a missing minor defaults to 0).
FOR /F "tokens=1,2 delims=." %%a IN ("!CUR!") DO ( SET "cMaj=%%a" & SET "cMin=%%b" )
FOR /F "tokens=1,2 delims=." %%a IN ("!REQ!") DO ( SET "rMaj=%%a" & SET "rMin=%%b" )
IF "!cMin!"=="" SET "cMin=0"
IF "!rMin!"=="" SET "rMin=0"

:: Numeric comparison: major first, then minor. GTR/LSS on all-digit operands are
:: numeric, so "12" GTR "2" is TRUE (unlike a naive string compare).
IF !cMaj! GTR !rMaj! GOTO Ok
IF !cMaj! LSS !rMaj! GOTO TooOld
IF !cMin! GTR !rMin! GOTO Ok
IF !cMin! LSS !rMin! GOTO TooOld

:Ok
ENDLOCAL & EXIT /B 0

:TooOld
ECHO [checkversion] OSDK !CUR! is older than the required !REQ!.
ENDLOCAL & EXIT /B 1
