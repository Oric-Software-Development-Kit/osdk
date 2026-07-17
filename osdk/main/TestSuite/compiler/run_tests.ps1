# run_tests.ps1 - OSDK compiler execution test suite runner.
#
# For every tests\t_*.c file and every requested optimization level:
#   1. builds it with the standard OSDK pipeline (make.bat),
#   2. boots the .tap in a sandboxed copy of Oricutron,
#   3. reads the test's printer output (printer_out.txt) until @END,
#   4. records pass/fail counts, elapsed 100Hz ticks and TAP size.
#
# Results go to results\<timestamp>_<label>.csv so runs can be diffed
# across compiler/MACROS.H changes.
#
# Usage:
#   .\run_tests.ps1                       # all tests, -O1 -O2 -O3
#   .\run_tests.ps1 -Levels 3 -Filter t_struct   # one test at -O3
#   .\run_tests.ps1 -Label "after-fix"    # tag the CSV file name

param(
    [string[]]$Levels = @('1','2','3'),
    [string]$Filter = 't_*',
    [string]$Label = 'run',
    [int]$TimeoutSec = 60,
    [switch]$Headless,     # pass --headless to the emulator: no window, no focus steal
    [switch]$Turbo,        # start the emulator at warp speed (--turbo, needs a build with the flag)
    [switch]$Peephole      # build with the MacroSplitter peephole optimizer (OSDKMACRO=-O)
)

. "$PSScriptRoot\hidden_launch.ps1"

$ErrorActionPreference = 'Stop'
$suite   = $PSScriptRoot
$osdk    = (Resolve-Path "$suite\..\..\Osdk\_final_").Path
$sandbox = "$suite\sandbox"
$scaffold= "$suite\scaffold"
$results = "$suite\results"

# ---------------------------------------------------------------- sandbox
# Private copy of the emulator so we never collide with a developer's
# Oricutron session (or another agent's). Roms are referenced as ../Roms.
if (-not (Test-Path "$sandbox\Oricutron\oricutron.exe")) {
    Write-Host "Creating emulator sandbox..."
    New-Item -ItemType Directory -Force "$sandbox\Oricutron\images" | Out-Null
    Copy-Item "$osdk\Oricutron\oricutron.exe" "$sandbox\Oricutron\"
    Copy-Item "$osdk\Oricutron\SDL.dll" "$sandbox\Oricutron\" -ErrorAction SilentlyContinue
    Copy-Item "$osdk\Oricutron\SDL2.dll" "$sandbox\Oricutron\" -ErrorAction SilentlyContinue
    Copy-Item "$osdk\Oricutron\oricutron.cfg" "$sandbox\Oricutron\"
    Copy-Item "$osdk\Oricutron\images\*" "$sandbox\Oricutron\images\"
    New-Item -ItemType Directory -Force "$sandbox\Roms" | Out-Null
    Copy-Item "$osdk\Roms\*" "$sandbox\Roms\"
}

New-Item -ItemType Directory -Force $results | Out-Null
Remove-Item Env:\NoDefaultCurrentDirectoryInExePath -ErrorAction SilentlyContinue
$env:OSDK = $osdk
$env:OSDKBRIEF = 'YES'

# ---------------------------------------------------------------- lock
# Only one runner may drive the shared emulator sandbox at a time:
# concurrent runs race over OSDK.TAP and printer_out.txt and corrupt
# each other's results. Stale locks (>30 min) are stolen.
$lockFile = "$sandbox\.lock"
$lockAcquired = $false
for ($w = 0; $w -lt 360 -and -not $lockAcquired; $w++) {
    try {
        $fs = [System.IO.File]::Open($lockFile, 'CreateNew', 'Write', 'None')
        $fs.Close(); $lockAcquired = $true
    } catch {
        if ((Test-Path $lockFile) -and ((Get-Date) - (Get-Item $lockFile).LastWriteTime).TotalMinutes -gt 30) {
            Remove-Item $lockFile -Force -ErrorAction SilentlyContinue
        } else {
            if ($w -eq 0) { Write-Host "sandbox busy (another runner is active), waiting..." }
            Start-Sleep -Seconds 5
        }
    }
}
if (-not $lockAcquired) { throw "could not acquire sandbox lock $lockFile" }

try {


$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$csv = "$results\$stamp`_$Label.csv"
"test,level,status,pass,fail,ticks,tap_bytes" | Out-File -Encoding ascii $csv

$tests = Get-ChildItem "$suite\tests" -Filter "$Filter.c" | Sort-Object Name
Write-Host "$($tests.Count) test(s), levels: $($Levels -join ' ')"

foreach ($test in $tests) {
    foreach ($lvl in $Levels) {
        $name = $test.BaseName

        # keep the sandbox lock fresh so a long run isn't mistaken for stale
        try { (Get-Item $lockFile -ErrorAction Stop).LastWriteTime = Get-Date } catch {}

        # -------------------------------------------------- scaffold+build
        if (Test-Path $scaffold) {
            try { Remove-Item -Recurse -Force $scaffold -ErrorAction Stop }
            catch {
                # a stale process is holding the folder: fall back to a unique one
                $scaffold = "$suite\scaffold-$(Get-Date -Format 'HHmmss')"
            }
        }
        New-Item -ItemType Directory -Force $scaffold | Out-Null
        Copy-Item $test.FullName "$scaffold\main.c"
        Copy-Item "$suite\testkit\testkit.h" $scaffold
        Copy-Item "$suite\testkit\tk_io.s" $scaffold
        # a test may ship a companion header (same basename) it #includes
        $testh = [IO.Path]::ChangeExtension($test.FullName, ".h")
        if (Test-Path $testh) { Copy-Item $testh $scaffold }
        @"
SET OSDKADDR=`$400
SET OSDKNAME=TKTEST
SET OSDKFILE=main tk_io
SET OSDKCOMP=-O$lvl
$(if ($Peephole) { "SET OSDKMACRO=-O" })
"@ | Out-File -Encoding ascii "$scaffold\osdk_config.bat"

        Push-Location $scaffold
        $buildLog = cmd /c "call osdk_config.bat && call `"$osdk\bin\make.bat`" %OSDKFILE% 2>&1"
        Pop-Location
        $tap = "$scaffold\build\TKTEST.tap"
        if (-not (Test-Path $tap)) {
            Write-Host ("{0,-12} -O{1}  BUILD ERROR" -f $name, $lvl) -ForegroundColor Red
            ($buildLog | Select-Object -Last 5) | ForEach-Object { Write-Host "    $_" }
            "$name,$lvl,builderror,,,," | Out-File -Encoding ascii -Append $csv
            continue
        }
        $tapBytes = (Get-Item $tap).Length

        # -------------------------------------------------- run in emulator
        $emuDir = "$sandbox\Oricutron"
        Copy-Item $tap "$emuDir\OSDK.TAP" -Force
        $printer = "$emuDir\printer_out.txt"
        Remove-Item $printer -Force -ErrorAction SilentlyContinue

        $proc = Start-EmulatorProcess -Exe "$emuDir\oricutron.exe" -Arguments ($(if ($Headless) {'--headless '} else {''}) + $(if ($Turbo) {'--turbo '} else {''}) + '-t OSDK.TAP') -WorkDir $emuDir
        $status = 'timeout'; $deadline = (Get-Date).AddSeconds($TimeoutSec)
        while ((Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 500
            if ((Test-Path $printer) -and (Select-String -Path $printer -Pattern '@END' -Quiet -ErrorAction SilentlyContinue)) {
                $status = 'ok'; break
            }
            if ($proc.HasExited) { $status = 'emudied'; break }
        }
        if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue; $proc.WaitForExit() | Out-Null }

        # -------------------------------------------------- parse results
        $pass = ''; $fail = ''; $ticks = ''
        if ($status -eq 'ok') {
            $out = Get-Content $printer -Raw
            if ($out -match 'pass=([0-9A-Fa-f]+)\s+fail=([0-9A-Fa-f]+)\s+ticks=([0-9A-Fa-f]+)') {
                $pass  = [Convert]::ToInt32($Matches[1],16)
                $fail  = [Convert]::ToInt32($Matches[2],16)
                $ticks = [Convert]::ToInt32($Matches[3],16)
                if ($fail -gt 0) { $status = 'fail' }
            } else { $status = 'noresult' }
            # echo any FAIL detail lines
            foreach ($line in ($out -split "`r?`n" | Where-Object { $_ -match '^FAIL' })) {
                Write-Host "    $line" -ForegroundColor Yellow
            }
        }
        $color = if ($status -eq 'ok') { 'Green' } elseif ($status -eq 'fail') { 'Yellow' } else { 'Red' }
        Write-Host ("{0,-12} -O{1}  {2,-9} pass={3,-3} fail={4,-3} ticks={5,-5} tap={6}b" -f $name,$lvl,$status,$pass,$fail,$ticks,$tapBytes) -ForegroundColor $color
        "$name,$lvl,$status,$pass,$fail,$ticks,$tapBytes" | Out-File -Encoding ascii -Append $csv
    }
}

} finally {
    Remove-Item $lockFile -Force -ErrorAction SilentlyContinue
}

Write-Host "`nResults written to $csv"
