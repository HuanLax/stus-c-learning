param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [string]$CompilerPath = '',
    [string]$RaylibSourcePath = '')

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$outputDirectory = Join-Path $projectRoot 'bin'
$outputPath = Join-Path $outputDirectory 'project-lax.exe'

# Explicit parameters take priority, followed by environment and common installs.
if (-not $CompilerPath) {
    $CompilerPath = $env:PROJECT_LAX_GCC
    if (-not $CompilerPath) {
        $gccCommand = Get-Command gcc.exe -ErrorAction SilentlyContinue
        if ($gccCommand) { $CompilerPath = $gccCommand.Source }
    }
    if (-not $CompilerPath) {
        foreach ($candidate in @((Join-Path $projectRoot '..\w64devkit\bin\gcc.exe'), 'C:\raylib\w64devkit\bin\gcc.exe')) {
            if (Test-Path -LiteralPath $candidate) { $CompilerPath = $candidate; break }
        }
    }
    if (-not $CompilerPath) { throw 'GCC not found. Set PROJECT_LAX_GCC or pass -CompilerPath.' }
}
if (-not $RaylibSourcePath) {
    $RaylibSourcePath = $env:PROJECT_LAX_RAYLIB
    if (-not $RaylibSourcePath) {
        foreach ($candidate in @((Join-Path $projectRoot '..\raylib\src'), 'C:\raylib\raylib\src')) {
            if ((Test-Path -LiteralPath (Join-Path $candidate 'raylib.h')) -and
                (Test-Path -LiteralPath (Join-Path $candidate 'libraylib.a'))) { $RaylibSourcePath = $candidate; break }
        }
    }
    if (-not $RaylibSourcePath) { throw 'raylib not found. Set PROJECT_LAX_RAYLIB or pass -RaylibSourcePath.' }
}

foreach ($requiredPath in @($CompilerPath, (Join-Path $RaylibSourcePath 'raylib.h'), (Join-Path $RaylibSourcePath 'libraylib.a'))) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "Required build dependency not found: $requiredPath"
    }
}

New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
$sourceFiles = @(Get-ChildItem -LiteralPath (Join-Path $projectRoot 'src') -Filter '*.c' -File | Sort-Object Name | ForEach-Object { $_.FullName })
if ($sourceFiles.Count -eq 0) { throw 'No C source files found.' }

$compilerArguments = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-finput-charset=UTF-8', '-fexec-charset=UTF-8')
if ($Configuration -eq 'Debug') { $compilerArguments += @('-g', '-O0') }
else { $compilerArguments += '-O2' }
$compilerArguments += @('-I', $RaylibSourcePath)
$compilerArguments += $sourceFiles
$compilerArguments += @('-L', $RaylibSourcePath, '-o', $outputPath, '-lraylib', '-lopengl32', '-lgdi32', '-lwinmm', '-lcomdlg32', '-lole32')

& $CompilerPath @compilerArguments
if ($LASTEXITCODE -ne 0) { throw "Build failed with exit code $LASTEXITCODE" }
Write-Host "Built $Configuration -> $outputPath"
