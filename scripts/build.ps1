param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [string]$CompilerPath = 'C:\raylib\w64devkit\bin\gcc.exe',
    [string]$RaylibSourcePath = 'C:\raylib\raylib\src',
    [switch]$CheckMovement,
    [switch]$CheckCombat,
    [switch]$CheckAI,
    [switch]$CheckInventory,
    [switch]$CheckQuest,
    [switch]$CheckSave,
    [switch]$CheckMusic
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$outputDirectory = Join-Path $projectRoot 'bin'
$outputPath = Join-Path $outputDirectory 'project-lax.exe'

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

if ($CheckMovement) {
    $testPath = Join-Path $outputDirectory 'test-movement.exe'
    $testArguments = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-g',
        '-I', $RaylibSourcePath, '-I', (Join-Path $projectRoot 'src'),
        (Join-Path $projectRoot 'tests\test_movement.c'),
        (Join-Path $projectRoot 'src\world.c'), (Join-Path $projectRoot 'src\player.c'),
        (Join-Path $projectRoot 'src\inventory.c'),
        (Join-Path $projectRoot 'src\assets.c'), (Join-Path $projectRoot 'src\text.c'),
        '-L', $RaylibSourcePath, '-o', $testPath,
        '-lraylib', '-lopengl32', '-lgdi32', '-lwinmm', '-lcomdlg32', '-lole32')
    & $CompilerPath @testArguments
    if ($LASTEXITCODE -ne 0) { throw 'Movement check compilation failed.' }
    & $testPath
    if ($LASTEXITCODE -ne 0) { throw 'Movement checks failed.' }
}

foreach ($checkKind in @('Combat', 'AI', 'Inventory', 'Quest', 'Save', 'Music')) {
    if (($checkKind -eq 'Combat' -and -not $CheckCombat) -or ($checkKind -eq 'AI' -and -not $CheckAI)) { continue }
    if ($checkKind -eq 'Inventory' -and -not $CheckInventory) { continue }
    if ($checkKind -eq 'Quest' -and -not $CheckQuest) { continue }
    if ($checkKind -eq 'Save' -and -not $CheckSave) { continue }
    if ($checkKind -eq 'Music' -and -not $CheckMusic) { continue }
    $testFile = if ($checkKind -eq 'Combat') { 'test_combat.c' } elseif ($checkKind -eq 'AI') { 'test_enemy_ai.c' }
        elseif ($checkKind -eq 'Inventory') { 'test_inventory.c' } elseif ($checkKind -eq 'Quest') { 'test_quest.c' }
        elseif ($checkKind -eq 'Music') { 'test_music.c' } else { 'test_save.c' }
    $combatTestPath = Join-Path $outputDirectory ("test-" + $checkKind.ToLower() + '.exe')
    $combatSources = @($sourceFiles | Where-Object { [System.IO.Path]::GetFileName($_) -ne 'main.c' })
    $combatArguments = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-g',
        '-finput-charset=UTF-8', '-fexec-charset=UTF-8', '-I', $RaylibSourcePath,
        '-I', (Join-Path $projectRoot 'src'), (Join-Path (Join-Path $projectRoot 'tests') $testFile))
    $combatArguments += $combatSources
    $combatArguments += @('-L', $RaylibSourcePath, '-o', $combatTestPath,
        '-lraylib', '-lopengl32', '-lgdi32', '-lwinmm', '-lcomdlg32', '-lole32')
    & $CompilerPath @combatArguments
    if ($LASTEXITCODE -ne 0) { throw "$checkKind check compilation failed." }
    & $combatTestPath
    if ($LASTEXITCODE -ne 0) { throw "$checkKind checks failed." }
}
