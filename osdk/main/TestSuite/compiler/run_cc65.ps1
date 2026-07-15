# run_cc65.ps1 - run the cc65 test/val suite (unmodified) on the OSDK toolchain.
#
# The cc65 test files are NOT part of this repository (GPL): point -Cc65 at a
# cc65 checkout (https://github.com/cc65/cc65). Each test is compiled through
# a wrapper (cc65shim/wrapper.c) that renames its main(), redirects printf
# output to the emulated printer, and reports the test's return value
# (= its failure counter) plus timing.
#
# Usage:
#   .\run_cc65.ps1 -Cc65 D:\path\to\cc65                 # all eligible, -O2
#   .\run_cc65.ps1 -Cc65 ... -Levels 2,3 -Filter add*    # subset
#
# Pre-filtered (recorded, not run):
#   skipped-long   uses long / int32 (16-bit long would run wrong, not fail)
#   skipped-float  uses float/double (different value range than IEEE hosts)
#   skipped-lib    needs a library OSDK doesn't provide (zlib, stdarg, ...)

param(
    [Parameter(Mandatory=$true)][string]$Cc65,
    [string[]]$Levels = @('2'),
    [string]$Filter = '*',
    [string]$Label = 'cc65val',
    [int]$TimeoutSec = 45,
    [switch]$Headless      # run Oricutron on a hidden desktop: no window, no focus steal
)

. "$PSScriptRoot\hidden_launch.ps1"

$ErrorActionPreference = 'Stop'
$suite   = $PSScriptRoot
$osdk    = (Resolve-Path "$suite\..\..\Osdk\_final_").Path
$sandbox = "$suite\sandbox"
$scaffold= "$suite\scaffold-cc65"
$results = "$suite\results"
$valdir  = Join-Path $Cc65 "test\val"
if (-not (Test-Path $valdir)) { throw "test\val not found under $Cc65" }

# sandbox emulator (shared with run_tests.ps1)
if (-not (Test-Path "$sandbox\Oricutron\oricutron.exe")) {
    & "$suite\run_tests.ps1" -Levels 2 -Filter no_such_test | Out-Null
}

New-Item -ItemType Directory -Force $results | Out-Null
Remove-Item Env:\NoDefaultCurrentDirectoryInExePath -ErrorAction SilentlyContinue
$env:OSDK = $osdk
$env:OSDKBRIEF = 'YES'

# ---------------------------------------------------------------- lock
# Only one runner may drive the shared emulator sandbox at a time (see
# run_tests.ps1). Stale locks (>30 min) are stolen.
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
$logdir = "$results\$stamp`_$Label-logs"
New-Item -ItemType Directory -Force $logdir | Out-Null
"test,level,status,failures,ticks,tap_bytes" | Out-File -Encoding ascii $csv

$tests = Get-ChildItem $valdir -Filter "$Filter.c" | Sort-Object Name
Write-Host "$($tests.Count) candidate test(s), levels: $($Levels -join ' ')"
$tally = @{}

# documented-slow tests (~10M cycles at 1MHz): per-test timeout override
$slowTests = @{ 'compare7'=300; 'compare8'=300; 'compare9'=300; 'compare10'=300 }

foreach ($test in $tests) {
    $name = $test.BaseName
    $src = Get-Content $test.FullName -Raw

    # ------------------------------------------------------ pre-filters
    $skip = $null
    if ($src -match '#include\s+<(zlib|lzsa|zx02|arpa/|lz4|unistd|signal|stdarg)') { $skip = 'skipped-lib' }
    elseif ($src -match '\blong\b|u?int32|U?INT32') { $skip = 'skipped-long' }
    elseif ($src -match '\bfloat\b|\bdouble\b') { $skip = 'skipped-float' }
    if ($skip) {
        foreach ($lvl in $Levels) { "$name,$lvl,$skip,,," | Out-File -Encoding ascii -Append $csv }
        $tally[$skip] = $tally[$skip] + $Levels.Count
        continue
    }

    foreach ($lvl in $Levels) {
        # keep the sandbox lock fresh so a long run isn't mistaken for stale
        try { (Get-Item $lockFile -ErrorAction Stop).LastWriteTime = Get-Date } catch {}

        # -------------------------------------------------- scaffold+build
        if (Test-Path $scaffold) {
            try { Remove-Item -Recurse -Force $scaffold -ErrorAction Stop }
            catch { $scaffold = "$suite\scaffold-cc65-$(Get-Date -Format 'HHmmss')" }
        }
        New-Item -ItemType Directory -Force $scaffold | Out-Null
        Copy-Item "$suite\cc65shim\wrapper.c" "$scaffold\main.c"
        Copy-Item "$suite\cc65shim\*.h" $scaffold
        Copy-Item "$suite\testkit\testkit.h" $scaffold
        Copy-Item "$suite\testkit\tk_io.s" $scaffold
        Copy-Item $test.FullName "$scaffold\testcase.c"
        # some tests include local helper headers from the val directory
        Copy-Item "$valdir\*.h" $scaffold -ErrorAction SilentlyContinue
        @"
SET OSDKADDR=`$400
SET OSDKNAME=TKTEST
SET OSDKFILE=main tk_io
SET OSDKCOMP=-O$lvl
SET OSDKCPPFLAGS=-I .
"@ | Out-File -Encoding ascii "$scaffold\osdk_config.bat"

        Push-Location $scaffold
        $buildLog = cmd /c "call osdk_config.bat && call `"$osdk\bin\make.bat`" %OSDKFILE% 2>&1"
        Pop-Location
        $tap = "$scaffold\build\TKTEST.tap"
        if (-not (Test-Path $tap)) {
            Write-Host ("{0,-28} -O{1}  BUILD ERROR" -f $name, $lvl) -ForegroundColor Red
            ($buildLog | Select-String -Pattern 'error|Error|ERROR' | Select-Object -First 2) | ForEach-Object { Write-Host "    $_" }
            $buildLog | Out-File -Encoding ascii "$logdir\$name-O$lvl-build.log"
            "$name,$lvl,builderror,,," | Out-File -Encoding ascii -Append $csv
            $tally['builderror'] = $tally['builderror'] + 1
            continue
        }
        $tapBytes = (Get-Item $tap).Length
        # tape programs load at $400 and must stay below ~$B400 (charsets/screen above):
        # a TAP beyond ~45KB cannot fit in the Oric's memory and would hang
        # during the tape load without any output
        if ($tapBytes -gt 45000) {
            Write-Host ("{0,-28} -O{1}  toobig    tap={2}b exceeds Oric RAM" -f $name, $lvl, $tapBytes) -ForegroundColor DarkYellow
            "$name,$lvl,toobig,,,$tapBytes" | Out-File -Encoding ascii -Append $csv
            $tally['toobig'] = $tally['toobig'] + 1
            continue
        }

        # -------------------------------------------------- run
        $emuDir = "$sandbox\Oricutron"
        Copy-Item $tap "$emuDir\OSDK.TAP" -Force
        $printer = "$emuDir\printer_out.txt"
        Remove-Item $printer -Force -ErrorAction SilentlyContinue
        $proc = Start-EmulatorProcess -Exe "$emuDir\oricutron.exe" -Arguments '-t OSDK.TAP' -WorkDir $emuDir -Hidden:$Headless
        $effTimeout = if ($slowTests.ContainsKey($name)) { $slowTests[$name] } else { $TimeoutSec }
        $status = 'timeout'; $deadline = (Get-Date).AddSeconds($effTimeout)
        while ((Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 500
            if ((Test-Path $printer) -and (Select-String -Path $printer -Pattern '@END' -Quiet -ErrorAction SilentlyContinue)) { $status = 'ok'; break }
            if ($proc.HasExited) { $status = 'emudied'; break }
        }
        if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue; $proc.WaitForExit() | Out-Null }

        # -------------------------------------------------- verdict
        $failures = ''; $ticks = ''
        if ($status -eq 'ok') {
            $out = Get-Content $printer -Raw
            if ($out -match 'failures=([0-9A-Fa-f]+)\s+ticks=([0-9A-Fa-f]+)') {
                $failures = [Convert]::ToInt32($Matches[1],16)
                $ticks    = [Convert]::ToInt32($Matches[2],16)
            }
            # prefer the test's own printed counter when present
            if ($out -match 'failures:\s*(\d+)') { $failures = [int]$Matches[1] }
            if ($failures -is [int] -and $failures -gt 0) { $status = 'fail' }
            if ($status -ne 'ok') { $out | Out-File -Encoding ascii "$logdir\$name-O$lvl-out.log" }
        }
        elseif (Test-Path $printer) {
            # timeout/emudied: keep whatever the test printed before it stopped
            Get-Content $printer -Raw | Out-File -Encoding ascii "$logdir\$name-O$lvl-out.log"
        }
        $color = switch ($status) { 'ok' {'Green'} 'fail' {'Yellow'} default {'Red'} }
        Write-Host ("{0,-28} -O{1}  {2,-9} failures={3,-4} ticks={4,-5} tap={5}b" -f $name,$lvl,$status,$failures,$ticks,$tapBytes) -ForegroundColor $color
        "$name,$lvl,$status,$failures,$ticks,$tapBytes" | Out-File -Encoding ascii -Append $csv
        $tally[$status] = $tally[$status] + 1
    }
}

} finally {
    Remove-Item $lockFile -Force -ErrorAction SilentlyContinue
}

Write-Host "`nSummary:"; $tally.GetEnumerator() | Sort-Object Name | ForEach-Object { Write-Host ("  {0,-14} {1}" -f $_.Name, $_.Value) }
Write-Host "Results: $csv"