# compare_results.ps1 - produce a markdown comparison of two test-run CSVs.
#
# Usage:
#   .\compare_results.ps1 old.csv new.csv                 # to stdout
#   .\compare_results.ps1 old.csv new.csv -Out report.md
#   .\compare_results.ps1 old.csv new.csv -LabelA "1.40" -LabelB "1.41"

param(
    [Parameter(Mandatory=$true)][string]$CsvA,
    [Parameter(Mandatory=$true)][string]$CsvB,
    [string]$LabelA = '',
    [string]$LabelB = '',
    [string]$Out = ''
)

if (-not $LabelA) { $LabelA = [IO.Path]::GetFileNameWithoutExtension($CsvA) }
if (-not $LabelB) { $LabelB = [IO.Path]::GetFileNameWithoutExtension($CsvB) }

$a = Import-Csv $CsvA
$b = Import-Csv $CsvB
$keys = @($a | ForEach-Object { "$($_.test)|$($_.level)" }) + @($b | ForEach-Object { "$($_.test)|$($_.level)" }) | Sort-Object -Unique

$md = New-Object System.Collections.Generic.List[string]
$md.Add("# Compiler test suite comparison")
$md.Add("")
$md.Add("| | A | B |")
$md.Add("|---|---|---|")
$md.Add("| run | ``$LabelA`` | ``$LabelB`` |")
$md.Add("")
$md.Add("| test | -O | status A | status B | pass A>B | fail A>B | ticks A>B | tap bytes A>B | verdict |")
$md.Add("|---|---|---|---|---|---|---|---|---|")

$better=0; $worse=0; $equal=0
foreach ($k in $keys) {
    $t,$l = $k -split '\|'
    $ra = $a | Where-Object { $_.test -eq $t -and $_.level -eq $l } | Select-Object -First 1
    $rb = $b | Where-Object { $_.test -eq $t -and $_.level -eq $l } | Select-Object -First 1
    $sa = if ($ra) { $ra.status } else { 'missing' }
    $sb = if ($rb) { $rb.status } else { 'missing' }

    $rank = @{ ok = 3; fail = 2; noresult = 1; timeout = 1; emudied = 1; builderror = 0; missing = 0 }
    $verdict = '='
    if ($rank[$sb] -gt $rank[$sa]) { $verdict = 'IMPROVED'; $better++ }
    elseif ($rank[$sb] -lt $rank[$sa]) { $verdict = 'REGRESSED'; $worse++ }
    elseif ($sa -eq 'ok' -or $sa -eq 'fail') {
        if ($ra.pass -ne $rb.pass -or $ra.fail -ne $rb.fail) {
            if ([int]$rb.fail -lt [int]$ra.fail) { $verdict = 'IMPROVED'; $better++ }
            else { $verdict = 'REGRESSED'; $worse++ }
        } else { $equal++ }
    } else { $equal++ }

    function Pair($x, $y) { if ($x -eq $y) { "$x" } else { "$x > $y" } }
    $md.Add("| $t | $l | $sa | $sb | $(Pair $ra.pass $rb.pass) | $(Pair $ra.fail $rb.fail) | $(Pair $ra.ticks $rb.ticks) | $(Pair $ra.tap_bytes $rb.tap_bytes) | $verdict |")
}

$md.Add("")
$md.Add("**Summary: $better improved, $worse regressed, $equal unchanged.**")

if ($Out) { $md -join "`n" | Out-File -Encoding utf8 $Out; Write-Host "Written to $Out" }
else { $md -join "`n" }
