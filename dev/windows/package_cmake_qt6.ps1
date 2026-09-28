[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$QtRoot,

    [Parameter(Mandatory = $true)]
    [string]$FftwRoot,

    [ValidatePattern('^\d+\.\d+\.\d+$')]
    [string]$Version = "12.11.2",

    [string]$BuildDirectory = "cmake-build\windows-msvc-release",

    [string]$StageDirectory = "dist\Engauge Digitizer",

    [string]$CMakeExecutable = "cmake.exe",

    [ValidateRange(0, 60)]
    [int]$SmokeTestSeconds = 5,

    [switch]$CreateInstaller,

    [string]$InnoCompiler
)

$ErrorActionPreference = "Stop"

function Get-AbsolutePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$BasePath
    )

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    return [System.IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Get-ExecutablePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Command
    )

    if (Test-Path $Command -PathType Leaf) {
        return (Resolve-Path $Command).Path
    }

    $resolvedCommand = Get-Command $Command -ErrorAction SilentlyContinue
    if (-not $resolvedCommand) {
        throw "Required executable was not found: $Command"
    }

    return $resolvedCommand.Source
}

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter(Mandatory = $true)]
        [string[]]$ArgumentList
    )

    & $FilePath @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "$FilePath failed with exit code $LASTEXITCODE."
    }
}

function Assert-RepositoryChildPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$RepositoryRoot
    )

    $repositoryPrefix = $RepositoryRoot.TrimEnd('\', '/') + `
        [System.IO.Path]::DirectorySeparatorChar
    if (-not $Path.StartsWith(
            $repositoryPrefix,
            [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clean a path outside the repository: $Path"
    }
}

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$qtRootPath = (Resolve-Path $QtRoot).Path
$fftwRootPath = (Resolve-Path $FftwRoot).Path
$buildDirectoryPath = Get-AbsolutePath $BuildDirectory $repositoryRoot
$stageDirectoryPath = Get-AbsolutePath $StageDirectory $repositoryRoot
$distDirectoryPath = Join-Path $repositoryRoot "dist"
$windowsFileVersion = "$Version.0"
$portableArchive = Join-Path $distDirectoryPath `
    "Engauge-Digitizer-$Version-Windows-x64-Portable.zip"

Assert-RepositoryChildPath $stageDirectoryPath $repositoryRoot

$cmakeProject = Get-Content (Join-Path $repositoryRoot "CMakeLists.txt") -Raw
$sourceVersion = Get-Content `
    (Join-Path $repositoryRoot "src\util\Version.cpp") -Raw
if ($cmakeProject -notmatch
        "project\(EngaugeDigitizer VERSION $([regex]::Escape($Version)) LANGUAGES CXX\)" -or
    $sourceVersion -notmatch
        "VERSION_NUMBER\s*=\s*`"$([regex]::Escape($Version))`"") {
    throw "Package version $Version does not match CMakeLists.txt and Version.cpp."
}

$cmake = Get-ExecutablePath $CMakeExecutable
$ctestCandidate = Join-Path (Split-Path $cmake -Parent) "ctest.exe"
$ctest = if (Test-Path $ctestCandidate -PathType Leaf) {
    $ctestCandidate
}
else {
    Get-ExecutablePath "ctest.exe"
}
$windeployqt = Join-Path $qtRootPath "bin\windeployqt.exe"
$fftwHeader = Join-Path $fftwRootPath "include\fftw3.h"
$fftwDefinition = Join-Path $fftwRootPath "lib\libfftw3-3.def"
$fftwImportLibrary = Join-Path $fftwRootPath "lib\libfftw3-3.lib"
$fftwRuntime = Join-Path $fftwRootPath "lib\libfftw3-3.dll"

foreach ($requiredFile in @($windeployqt, $fftwHeader, $fftwRuntime)) {
    if (-not (Test-Path $requiredFile -PathType Leaf)) {
        throw "Required packaging file was not found: $requiredFile"
    }
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} `
    "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere -PathType Leaf)) {
    throw "Visual Studio Installer tool was not found: $vswhere"
}

$visualStudioRoot = & $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
if ($LASTEXITCODE -ne 0 -or -not $visualStudioRoot) {
    throw "Visual Studio 2022 with the C++ toolchain was not found."
}

$launchDevShell = Join-Path $visualStudioRoot `
    "Common7\Tools\Launch-VsDevShell.ps1"
if (-not (Test-Path $launchDevShell -PathType Leaf)) {
    throw "Visual Studio developer shell was not found: $launchDevShell"
}

& $launchDevShell -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
if ($LASTEXITCODE -ne 0) {
    throw "Visual Studio developer shell initialization failed."
}

if (-not (Test-Path $fftwImportLibrary -PathType Leaf)) {
    if (-not (Test-Path $fftwDefinition -PathType Leaf)) {
        throw "FFTW import library and module definition were both missing."
    }

    Invoke-Checked "lib.exe" @(
        "/NOLOGO",
        "/MACHINE:X64",
        "/DEF:$fftwDefinition",
        "/OUT:$fftwImportLibrary"
    )
}

$env:QT_ROOT = $qtRootPath
$env:FFTW_HOME = $fftwRootPath
$env:Path = "$(Join-Path $qtRootPath 'bin');$env:Path"

Push-Location $repositoryRoot
try {
    Invoke-Checked $cmake @(
        "--preset", "windows-msvc-release",
        "-B", $buildDirectoryPath
    )
    Invoke-Checked $cmake @(
        "--build", $buildDirectoryPath,
        "--config", "Release",
        "--parallel", "4"
    )
    Invoke-Checked $ctest @(
        "--test-dir", $buildDirectoryPath,
        "--build-config", "Release",
        "--output-on-failure"
    )

    if (Test-Path $stageDirectoryPath) {
        Remove-Item -LiteralPath $stageDirectoryPath -Recurse -Force
    }
    New-Item -ItemType Directory -Force $stageDirectoryPath | Out-Null

    Invoke-Checked $cmake @(
        "--install", $buildDirectoryPath,
        "--prefix", $stageDirectoryPath,
        "--config", "Release"
    )

    $engaugeExecutable = Join-Path $stageDirectoryPath "Engauge.exe"
    Invoke-Checked $windeployqt @(
        "--release",
        "--compiler-runtime",
        "--dir", $stageDirectoryPath,
        $engaugeExecutable
    )

    $requiredProducts = @(
        "Engauge.exe",
        "libfftw3-3.dll",
        "Qt6Core.dll",
        "Qt6Gui.dll",
        "Qt6Help.dll",
        "Qt6Widgets.dll",
        "platforms\qwindows.dll",
        "documentation\engauge.qch",
        "documentation\engauge.qhc",
        "translations\qt_zh_CN.qm",
        "translations\qt_zh_TW.qm",
        "LICENSE",
        "README.md",
        "THIRD-PARTY-NOTICES.md",
        "licenses\LGPL-3.0.txt"
    )
    $missingProducts = $requiredProducts | Where-Object {
        -not (Test-Path (Join-Path $stageDirectoryPath $_) -PathType Leaf)
    }
    if ($missingProducts) {
        throw "Missing deployed files: $($missingProducts -join ', ')."
    }

    foreach ($qtTranslation in @("qt_zh_CN.qm", "qt_zh_TW.qm")) {
        $qtTranslationFile = Get-Item (Join-Path $stageDirectoryPath `
            "translations\$qtTranslation")
        if ($qtTranslationFile.Length -lt 100000) {
            throw "Qt translation was not fully deployed: $qtTranslation"
        }
    }

    $expectedTranslations = @(
        "ar", "cs", "de", "en", "es", "fa_IR", "fr", "hi", "it",
        "ja", "kk", "ko", "nb", "nl", "pt", "ru", "zh_CN", "zh_TW"
    )
    $missingTranslations = $expectedTranslations | Where-Object {
        $translationFile = Join-Path $stageDirectoryPath `
            "translations\engauge_$_.qm"
        -not (Test-Path $translationFile -PathType Leaf)
    }
    if ($missingTranslations) {
        throw "Missing deployed translations: $($missingTranslations -join ', ')."
    }

    # Air-gapped deployment hardening. windeployqt deploys the generic Qt plugin
    # set, which includes networking, TLS and server database drivers that this
    # build does not use: the Windows build compiles with ENGAUGE_ENABLE_NETWORK=OFF
    # (so NetworkClient is not even built), and none of the shipped binaries import
    # Qt6Network.dll. The help engine only needs the SQLite driver. Removing these
    # keeps a closed network deployment free of unused network components; the
    # smoke test further down proves the application still starts afterwards.
    $airGapRemovals = @(
        "Qt6Network.dll",
        "networkinformation",
        "tls",
        "sqldrivers\qsqlmimer.dll",
        "sqldrivers\qsqlodbc.dll",
        "sqldrivers\qsqlpsql.dll"
    )
    foreach ($airGapRemoval in $airGapRemovals) {
        $removalPath = Join-Path $stageDirectoryPath $airGapRemoval
        if (Test-Path $removalPath) {
            Remove-Item -LiteralPath $removalPath -Recurse -Force
        }
    }

    $networkLeftovers = Get-ChildItem -Path $stageDirectoryPath -Recurse -Force -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like "*Network*" -or $_.Name -like "*tls*" }
    if ($networkLeftovers) {
        throw "Network artifacts remain in the staged deployment: $($networkLeftovers.FullName -join ', ')."
    }
    if (-not (Test-Path (Join-Path $stageDirectoryPath "sqldrivers\qsqlite.dll") -PathType Leaf)) {
        throw "The SQLite driver required by the help engine is missing."
    }

    if ($SmokeTestSeconds -gt 0) {
        $savedPath = $env:Path
        $savedQtPluginPath = $env:QT_PLUGIN_PATH
        $savedQmlImportPath = $env:QML2_IMPORT_PATH
        $savedEngaugeSettingsDirectory = $env:ENGAUGE_SETTINGS_DIR
        $tempDirectory = [System.IO.Path]::GetFullPath(
            [System.IO.Path]::GetTempPath())
        $smokeSettingsDirectory = [System.IO.Path]::GetFullPath((Join-Path `
            $tempDirectory `
            "engauge-package-smoke-$([guid]::NewGuid().ToString('N'))"))
        $tempPrefix = $tempDirectory.TrimEnd('\', '/') + `
            [System.IO.Path]::DirectorySeparatorChar
        if (-not $smokeSettingsDirectory.StartsWith(
                $tempPrefix,
                [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Smoke-test settings directory escaped the temp directory."
        }
        New-Item -ItemType Directory $smokeSettingsDirectory | Out-Null
        try {
            $env:Path = "$env:SystemRoot\System32;$env:SystemRoot"
            $env:ENGAUGE_SETTINGS_DIR = $smokeSettingsDirectory
            Remove-Item Env:\QT_PLUGIN_PATH -ErrorAction SilentlyContinue
            Remove-Item Env:\QML2_IMPORT_PATH -ErrorAction SilentlyContinue
            $process = Start-Process `
                -FilePath $engaugeExecutable `
                -WorkingDirectory $stageDirectoryPath `
                -WindowStyle Hidden `
                -PassThru
            Start-Sleep -Seconds $SmokeTestSeconds
            if ($process.HasExited) {
                throw "Portable Engauge exited during startup with code $($process.ExitCode)."
            }
            Stop-Process -Id $process.Id -Force
            Wait-Process -Id $process.Id -ErrorAction SilentlyContinue
        }
        finally {
            $env:Path = $savedPath
            if ($null -eq $savedQtPluginPath) {
                Remove-Item Env:\QT_PLUGIN_PATH -ErrorAction SilentlyContinue
            }
            else {
                $env:QT_PLUGIN_PATH = $savedQtPluginPath
            }
            if ($null -eq $savedQmlImportPath) {
                Remove-Item Env:\QML2_IMPORT_PATH -ErrorAction SilentlyContinue
            }
            else {
                $env:QML2_IMPORT_PATH = $savedQmlImportPath
            }
            if ($null -eq $savedEngaugeSettingsDirectory) {
                Remove-Item Env:\ENGAUGE_SETTINGS_DIR -ErrorAction SilentlyContinue
            }
            else {
                $env:ENGAUGE_SETTINGS_DIR = $savedEngaugeSettingsDirectory
            }
            Remove-Item `
                -LiteralPath $smokeSettingsDirectory `
                -Recurse `
                -Force
        }
    }

    New-Item -ItemType Directory -Force $distDirectoryPath | Out-Null
    if (Test-Path $portableArchive -PathType Leaf) {
        Remove-Item -LiteralPath $portableArchive -Force
    }
    Compress-Archive `
        -Path (Join-Path $stageDirectoryPath "*") `
        -DestinationPath $portableArchive `
        -CompressionLevel Optimal

    if ($CreateInstaller) {
        if ($InnoCompiler) {
            $iscc = Get-ExecutablePath $InnoCompiler
        }
        else {
            $isccCandidates = @(
                (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe"),
                (Join-Path $env:LOCALAPPDATA "Programs\Inno Setup 6\ISCC.exe")
            )
            $iscc = $isccCandidates | Where-Object {
                Test-Path $_ -PathType Leaf
            } | Select-Object -First 1
            if (-not $iscc) {
                $isccCommand = Get-Command "ISCC.exe" -ErrorAction SilentlyContinue
                if ($isccCommand) {
                    $iscc = $isccCommand.Source
                }
            }
        }

        if (-not $iscc) {
            throw "Inno Setup 6 was not found. Install it or pass -InnoCompiler."
        }

        Invoke-Checked $iscc @(
            "/DMyAppVersion=$Version",
            "/DMyAppFileVersion=$windowsFileVersion",
            "/DMyAppSourceDir=$stageDirectoryPath",
            (Join-Path $repositoryRoot "dev\windows\engauge_qt6.iss")
        )
        $setupExecutable = Join-Path $distDirectoryPath `
            "Engauge-Digitizer-$Version-Windows-x64-Setup.exe"
        if (-not (Test-Path $setupExecutable -PathType Leaf)) {
            throw "Inno Setup completed without producing $setupExecutable."
        }
        Write-Host "Created installer: $setupExecutable"
    }

    Write-Host "Created deployment directory: $stageDirectoryPath"
    Write-Host "Created portable archive: $portableArchive"
}
finally {
    Pop-Location
}
