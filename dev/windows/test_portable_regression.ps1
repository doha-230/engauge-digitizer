[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PortableRoot
)

$ErrorActionPreference = "Stop"

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$portableRootPath = (Resolve-Path $PortableRoot).Path
$portableExecutable = Join-Path $portableRootPath "Engauge.exe"
if (-not (Test-Path $portableExecutable -PathType Leaf)) {
    throw "Portable executable was not found: $portableExecutable"
}

$testRoot = Join-Path $env:RUNNER_TEMP "engauge-curve-properties-$([guid]::NewGuid().ToString('N'))"
New-Item -ItemType Directory -Path $testRoot | Out-Null
try {
    $fixture = Join-Path $testRoot "curve_properties_undo.xml"
    $expected = Join-Path $testRoot "curve_properties_undo.csv_expected_1"
    $actual = Join-Path $testRoot "curve_properties_undo.csv_actual_1"
    Copy-Item (Join-Path $repositoryRoot "test\curve_properties_undo.xml") $fixture
    Copy-Item (Join-Path $repositoryRoot "test\curve_properties_undo.csv_expected_1") $expected

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $portableExecutable
    $startInfo.WorkingDirectory = $testRoot
    $startInfo.UseShellExecute = $false
    foreach ($argument in @("-errorreport", $fixture, "-regression", "-reset")) {
        [void]$startInfo.ArgumentList.Add($argument)
    }
    $startInfo.Environment["Path"] = "$env:SystemRoot\System32;$env:SystemRoot"
    $startInfo.Environment["ENGAUGE_SETTINGS_DIR"] = $testRoot
    [void]$startInfo.Environment.Remove("QT_PLUGIN_PATH")
    [void]$startInfo.Environment.Remove("QML2_IMPORT_PATH")

    $process = [System.Diagnostics.Process]::Start($startInfo)
    if ($null -eq $process) {
        throw "Portable Engauge did not start."
    }
    try {
        if (-not $process.WaitForExit(90000)) {
            $process.Kill($true)
            throw "Portable curve properties regression timed out."
        }
        if ($process.ExitCode -ne 0) {
            throw "Portable curve properties regression exited with code $($process.ExitCode)."
        }
    }
    finally {
        $process.Dispose()
    }

    if (-not (Test-Path $actual -PathType Leaf)) {
        throw "Portable regression did not export CSV: $actual"
    }
    $expectedLines = @(Get-Content $expected)
    $actualLines = @(Get-Content $actual)
    if ($expectedLines.Count -ne $actualLines.Count) {
        throw "Portable regression exported $($actualLines.Count) lines; expected $($expectedLines.Count)."
    }
    for ($line = 0; $line -lt $expectedLines.Count; $line++) {
        if ($expectedLines[$line] -cne $actualLines[$line]) {
            throw "Portable regression CSV differs on line $($line + 1): expected '$($expectedLines[$line])', got '$($actualLines[$line])'."
        }
    }
    Write-Host "Portable curve properties Undo/Redo regression passed."
}
finally {
    Remove-Item -LiteralPath $testRoot -Recurse -Force
}
