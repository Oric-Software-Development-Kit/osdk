# run_bench.ps1 - run the ISS oricCompilerBenchmark samples on a chosen OSDK.
#
# The samples are NOT part of this repository: point -Bench at a checkout of
# https://github.com/iss000/oricCompilerBenchmark. Each sample is wrapped by
# benchshim/benchmain.c: its output goes through a cheap cport store
# (comparable to the benchmark's own mos6502vm CPORT), timing uses the ROM
# 100Hz counter and is reported through the emulated printer.
#
# Usage:
#   .\run_bench.ps1 -Bench D:\path\to\oricCompilerBenchmark                 # current worktree OSDK
#   .\run_bench.ps1 -Bench ... -OsdkRoot D:\tmp\osdk-123\osdk\main\Osdk\_final_ -Label osdk123
#   .\run_bench.ps1 -Bench ... -Filter 06-sieve
param(
    [Parameter(Mandatory=$true)][string]$Bench,
    [string]$OsdkRoot = '',
    [string]$Filter = '*',
    [string]$Label = 'bench',
    [string]$Comp = '-O2',
    [string]$Macro = '',          # MacroSplitter flags, e.g. -O to enable the peephole
    [int]$TimeoutSec = 120,
    [switch]$Headless,
    [switch]$Turbo
)

. "$PSScriptRoot\hidden_launch.ps1"

$ErrorActionPreference = 'Stop'
$suite   = $PSScriptRoot
if ($OsdkRoot -eq '') { $OsdkRoot = (Resolve-Path "$suite\..\..\Osdk\_final_").Path }
$sandbox = "$suite\sandbox"
$scaffold= "$suite\scaffold-bench"
$results = "$suite\results"
$samples = Join-Path $Bench "playground\samples"
if (-not (Test-Path $samples)) { throw "playground\samples not found under $Bench" }

# sandbox emulator (shared with run_tests.ps1; always the suite's own build,
# independent of which OSDK toolchain is being measured)
if (-not (Test-Path "$sandbox\Oricutron\oricutron.exe")) {
    & "$suite\run_tests.ps1" -Levels 2 -Filter no_such_test | Out-Null
}

New-Item -ItemType Directory -Force $results | Out-Null
Remove-Item Env:\NoDefaultCurrentDirectoryInExePath -ErrorAction SilentlyContinue
$env:OSDK = $OsdkRoot
$env:OSDKBRIEF = 'YES'

# ---------------------------------------------------------------- lock
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
"sample,status,retval,ticks,cycles,tap_bytes" | Out-File -Encoding ascii $csv

$dirs = Get-ChildItem $samples -Directory -Filter $Filter | Sort-Object Name
Write-Host "$($dirs.Count) sample(s), OSDK=$OsdkRoot, $Comp $(if ($Macro) {"macro=$Macro"})"
$tally = @{}

# long-running samples: per-sample timeout override (seconds)
$slow = @{ '07-aes256'=600; '08-mandelbrot'=600; '10-pi'=600; '20-heap-sort'=300; '16-quick-sort'=300 }

foreach ($dir in $dirs) {
    $name = $dir.Name
    $src = Get-ChildItem $dir.FullName -Filter '*.c' | Select-Object -First 1
    if ($null -eq $src) {
        "$name,nosource,,," | Out-File -Encoding ascii -Append $csv
        $tally['nosource'] = $tally['nosource'] + 1
        continue
    }

    try { (Get-Item $lockFile -ErrorAction Stop).LastWriteTime = Get-Date } catch {}

    # -------------------------------------------------- scaffold+build
    if (Test-Path $scaffold) {
        try { Remove-Item -Recurse -Force $scaffold -ErrorAction Stop }
        catch { $scaffold = "$suite\scaffold-bench-$(Get-Date -Format 'HHmmss')" }
    }
    New-Item -ItemType Directory -Force $scaffold | Out-Null
    New-Item -ItemType Directory -Force "$scaffold\sample" | Out-Null
    Copy-Item "$suite\benchshim\benchmain.c" "$scaffold\main.c"
    # library glue shims (conio/types/compat) + testkit at scaffold root, so
    # <conio.h> and "compat.h" resolve via -I .
    Copy-Item "$suite\benchshim\*.h" $scaffold
    Copy-Item "$suite\benchshim\cport.s" $scaffold
    Copy-Item "$suite\testkit\testkit.h" $scaffold
    Copy-Item "$suite\testkit\tk_io.s" $scaffold
    # the sample keeps a directory below scaffold, so its "../sort-helper.h"
    # style includes point back at scaffold root
    Copy-Item $src.FullName "$scaffold\sample\testcase.c"
    Copy-Item "$samples\*.h" $scaffold -ErrorAction SilentlyContinue
    # sample-local helper headers stay beside the sample
    Copy-Item "$($dir.FullName)\*.h" "$scaffold\sample" -ErrorAction SilentlyContinue
    @"
SET OSDKADDR=`$400
SET OSDKNAME=BENCH
SET OSDKFILE=main cport tk_io
SET OSDKCOMP=$Comp
SET OSDKMACRO=$Macro
SET OSDKCPPFLAGS=-I .
"@ | Out-File -Encoding ascii "$scaffold\osdk_config.bat"

    Push-Location $scaffold
    $buildLog = cmd /c "call osdk_config.bat && call `"$OsdkRoot\bin\make.bat`" %OSDKFILE% 2>&1"
    Pop-Location
    $tap = "$scaffold\build\BENCH.tap"
    if (-not (Test-Path $tap)) {
        Write-Host ("{0,-20} BUILD ERROR" -f $name) -ForegroundColor Red
        ($buildLog | Select-String -Pattern 'error|Error|ERROR' | Select-Object -First 2) | ForEach-Object { Write-Host "    $_" }
        $buildLog | Out-File -Encoding ascii "$logdir\$name-build.log"
        "$name,builderror,,," | Out-File -Encoding ascii -Append $csv
        $tally['builderror'] = $tally['builderror'] + 1
        continue
    }
    $tapBytes = (Get-Item $tap).Length
    if ($tapBytes -gt 45000) {
        Write-Host ("{0,-20} toobig    tap={1}b" -f $name, $tapBytes) -ForegroundColor DarkYellow
        "$name,toobig,,,$tapBytes" | Out-File -Encoding ascii -Append $csv
        $tally['toobig'] = $tally['toobig'] + 1
        continue
    }

    # -------------------------------------------------- run
    # The sandbox emulator can fail to autoload the tape and sit idle (looks like
    # a jam) or exit early, so we retry rather than burn the whole timeout. Care
    # is needed with the idle heuristic though: TotalProcessorTime stays LOW for
    # tens of seconds during tape loading even under --turbo, so "no CPU" alone
    # does NOT mean a failed autoload. A ~3s CPU threshold killed every sample
    # long before it finished (06-sieve needs ~25s to reach @END and then reports
    # @CYCLES normally) - that was the phantom "22/23 idle" result. Idle is now
    # only declared after IDLE_LIMIT polls with neither CPU activity NOR any
    # growth of printer_out.txt, so a slow-but-working run is never cut short.
    $emuDir = "$sandbox\Oricutron"
    Copy-Item $tap "$emuDir\OSDK.TAP" -Force
    $printer = "$emuDir\printer_out.txt"
    $effTimeout = if ($slow.ContainsKey($name)) { $slow[$name] } else { $TimeoutSec }
    foreach ($attempt in 1..3) {
        Remove-Item $printer -Force -ErrorAction SilentlyContinue
        $proc = Start-EmulatorProcess -Exe "$emuDir\oricutron.exe" -Arguments ($(if ($Headless) {'--headless '} else {''}) + $(if ($Turbo) {'--turbo '} else {''}) + '--cport -t OSDK.TAP') -WorkDir $emuDir
        Start-Sleep -Seconds 2   # startup grace before sampling CPU
        $status = 'timeout'; $deadline = (Get-Date).AddSeconds($effTimeout)
        $prevCpu = try { $proc.TotalProcessorTime } catch { [TimeSpan]::Zero }; $idle = 0
        $prevOut = 0
        $IDLE_LIMIT = 120        # 120 x 500ms = 60s of no CPU AND no output
        while ((Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 500
            if ((Test-Path $printer) -and (Select-String -Path $printer -Pattern '@END' -Quiet -ErrorAction SilentlyContinue)) { $status = 'ok'; break }
            if ($proc.HasExited) { $status = 'emudied'; break }
            try { $proc.Refresh(); $nowCpu = $proc.TotalProcessorTime } catch { $nowCpu = $prevCpu }
            $nowOut = if (Test-Path $printer) { (Get-Item $printer).Length } else { 0 }
            if ((($nowCpu - $prevCpu).TotalMilliseconds -lt 20) -and ($nowOut -eq $prevOut)) { $idle++ } else { $idle = 0 }
            $prevCpu = $nowCpu; $prevOut = $nowOut
            if ($idle -ge $IDLE_LIMIT) { $status = 'idle'; break }
        }
        if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue; $proc.WaitForExit() | Out-Null }
        if ($status -eq 'ok') { break }
        Start-Sleep -Milliseconds 500   # settle before retry so the prior instance fully releases
    }

    # -------------------------------------------------- verdict
    $retval = ''; $ticks = ''; $cycles = ''
    if ($status -eq 'ok') {
        $out = Get-Content $printer -Raw
        if ($out -match 'failures=([0-9A-Fa-f]+)\s+ticks=([0-9A-Fa-f]+)') {
            $retval = [Convert]::ToInt32($Matches[1],16)
            $ticks  = [Convert]::ToInt32($Matches[2],16)
        }
        if ($out -match '@CYCLES (\d+)') { $cycles = [long]$Matches[1] }
        $out | Out-File -Encoding ascii "$logdir\$name-out.log"
    }
    elseif (Test-Path $printer) {
        Get-Content $printer -Raw | Out-File -Encoding ascii "$logdir\$name-out.log"
    }
    $color = switch ($status) { 'ok' {'Green'} default {'Red'} }
    Write-Host ("{0,-20} {1,-9} ret={2,-4} cycles={3,-10} tap={4}b" -f $name,$status,$retval,$cycles,$tapBytes) -ForegroundColor $color
    "$name,$status,$retval,$ticks,$cycles,$tapBytes" | Out-File -Encoding ascii -Append $csv
    $tally[$status] = $tally[$status] + 1
}

} finally {
    Remove-Item $lockFile -Force -ErrorAction SilentlyContinue
}

Write-Host "`nSummary:"; $tally.GetEnumerator() | Sort-Object Name | ForEach-Object { Write-Host ("  {0,-14} {1}" -f $_.Name, $_.Value) }
Write-Host "Results: $csv"
