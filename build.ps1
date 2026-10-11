param(
    [string]$QtRoot = 'D:/Qt/qt/5.15.2/msvc2022_32_static',
    [string]$BuildDir = 'temp/build',
    [switch]$SkipTests
)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $arguments = @('-S', '.', '-B', $BuildDir, '-G', 'Visual Studio 17 2022', '-A', 'Win32', "-DCMAKE_PREFIX_PATH=$QtRoot")
    if (Test-Path -LiteralPath "$QtRoot/lib/Qt5Core.lib") {
        # The supplied Qt build links the static MSVC runtime (/MT).
        if ($QtRoot -match '_static$') { $arguments += '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded' }
    }
    & cmake @arguments
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }
    & cmake --build $BuildDir --config Release --target TeraTool_UI --parallel 4
    if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
    if (-not $SkipTests) {
        & ctest --test-dir $BuildDir -C Release --output-on-failure
        if ($LASTEXITCODE -ne 0) { throw 'UI regression tests failed' }
    }
    New-Item -ItemType Directory -Force 'dist' | Out-Null
    Copy-Item -LiteralPath "$BuildDir/Release/TeraTool_UI.exe" -Destination 'dist/TeraTool_UI.exe' -Force
    Write-Host "Built: $PSScriptRoot/dist/TeraTool_UI.exe"
} finally { Pop-Location }
