# Windows packaging script for UncOpener
# Usage: powershell -ExecutionPolicy Bypass -File packaging/package-windows.ps1
#
# Prerequisites:
#   - Qt6 installed with windeployqt available
#   - CMake build completed with Release configuration
#
# This script creates a portable ZIP distribution with all required Qt runtime files.

param(
    [string]$BuildDir = "build",
    [string]$OutputDir = "dist",
    [string]$QtDir = $env:Qt6_DIR
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Version = "1.0"
$AppName = "UncOpener"

Write-Host "=== $AppName Windows Packaging ===" -ForegroundColor Cyan
Write-Host "Version: $Version"
Write-Host "Build directory: $BuildDir"
Write-Host "Output directory: $OutputDir"
Write-Host ""

# Find the executable
$ExePath = Join-Path $ProjectRoot "$BuildDir\Release\src\app\uncopener.exe"
if (-not (Test-Path $ExePath)) {
    # Try alternative path (single-config generator)
    $ExePath = Join-Path $ProjectRoot "$BuildDir\src\app\uncopener.exe"
}

if (-not (Test-Path $ExePath)) {
    Write-Host "ERROR: Cannot find uncopener.exe in $BuildDir" -ForegroundColor Red
    Write-Host "Make sure you have built the project with Release configuration:" -ForegroundColor Yellow
    Write-Host "  cmake --preset default" -ForegroundColor Yellow
    Write-Host "  cmake --build --preset release" -ForegroundColor Yellow
    exit 1
}

Write-Host "Found executable: $ExePath" -ForegroundColor Green

# Find windeployqt
$WinDeployQt = $null
if ($QtDir) {
    $WinDeployQt = Join-Path $QtDir "bin\windeployqt.exe"
}

if (-not $WinDeployQt -or -not (Test-Path $WinDeployQt)) {
    # Try to find it in PATH
    $WinDeployQt = Get-Command windeployqt.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
}

if (-not $WinDeployQt) {
    Write-Host "ERROR: Cannot find windeployqt.exe" -ForegroundColor Red
    Write-Host "Set Qt6_DIR environment variable or add Qt bin directory to PATH" -ForegroundColor Yellow
    exit 1
}

Write-Host "Found windeployqt: $WinDeployQt" -ForegroundColor Green

# Create staging directory
$StageDir = Join-Path $ProjectRoot "$OutputDir\$AppName-$Version-windows"
if (Test-Path $StageDir) {
    Remove-Item -Recurse -Force $StageDir
}
New-Item -ItemType Directory -Force -Path $StageDir | Out-Null

Write-Host ""
Write-Host "Staging files..." -ForegroundColor Cyan

# Copy executable
Copy-Item $ExePath $StageDir

# Copy icon
$IconPath = Join-Path $ProjectRoot "assets\windows\icon.ico"
if (Test-Path $IconPath) {
    Copy-Item $IconPath $StageDir
}

# Run windeployqt
Write-Host "Running windeployqt..." -ForegroundColor Cyan
$ExeInStage = Join-Path $StageDir "uncopener.exe"
& $WinDeployQt --release --no-translations --no-system-d3d-compiler --no-opengl-sw $ExeInStage

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: windeployqt failed" -ForegroundColor Red
    exit 1
}

# Find and copy missing DLL dependencies using llvm-objdump
Write-Host "Checking for missing DLL dependencies..." -ForegroundColor Cyan
$QtBinDir = Split-Path $WinDeployQt -Parent

# Find llvm-objdump (should be in same directory as Qt tools when using LLVM/Clang build)
$LlvmObjdump = Get-Command llvm-objdump.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
if (-not $LlvmObjdump) {
    $LlvmObjdump = Join-Path $QtBinDir "llvm-objdump.exe"
}

function Get-DllDependencies {
    param([string]$FilePath)

    if (-not (Test-Path $LlvmObjdump)) {
        return @()
    }

    $output = & $LlvmObjdump -p $FilePath 2>$null
    $deps = $output | Select-String "DLL Name:" | ForEach-Object {
        ($_ -replace ".*DLL Name:\s*", "").Trim()
    }
    return $deps
}

function Copy-MissingDependencies {
    param(
        [string]$TargetDir,
        [string]$SourceDir,
        [ref]$ProcessedFiles
    )

    $allFiles = Get-ChildItem $TargetDir -Filter "*.dll" -Recurse
    $allFiles += Get-ChildItem $TargetDir -Filter "*.exe"

    $copiedAny = $false

    foreach ($file in $allFiles) {
        if ($ProcessedFiles.Value -contains $file.FullName) {
            continue
        }
        $ProcessedFiles.Value += $file.FullName

        $deps = Get-DllDependencies -FilePath $file.FullName
        foreach ($dep in $deps) {
            # Skip system DLLs
            if ($dep -match "^(KERNEL32|USER32|GDI32|SHELL32|ADVAPI32|ole32|OLEAUT32|COMDLG32|WINSPOOL|COMCTL32|IMM32|WINMM|WS2_32|VERSION|SHLWAPI|CRYPT32|NETAPI32|SECUR32|USERENV|MPR|SETUPAPI|BCRYPT|NCRYPT|NTDLL|msvcrt|VCRUNTIME|MSVCP|api-ms-win|ext-ms-win|ucrtbase)") {
                continue
            }

            $depInTarget = Join-Path $TargetDir $dep
            if (-not (Test-Path $depInTarget)) {
                $depInSource = Join-Path $SourceDir $dep
                if (Test-Path $depInSource) {
                    Write-Host "  Copying missing dependency: $dep" -ForegroundColor Yellow
                    Copy-Item $depInSource $TargetDir
                    $copiedAny = $true
                }
            }
        }
    }

    return $copiedAny
}

if (-not (Test-Path $LlvmObjdump)) {
    Write-Host "ERROR: llvm-objdump not found, cannot resolve DLL dependencies" -ForegroundColor Red
    Write-Host "       Install LLVM or add llvm-objdump.exe to PATH" -ForegroundColor Yellow
    exit 1
}

$processed = @()
$iteration = 0
$maxIterations = 10

do {
    $iteration++
    $copiedAny = Copy-MissingDependencies -TargetDir $StageDir -SourceDir $QtBinDir -ProcessedFiles ([ref]$processed)
} while ($copiedAny -and $iteration -lt $maxIterations)

if ($iteration -ge $maxIterations) {
    Write-Host "ERROR: Reached maximum iterations while resolving dependencies" -ForegroundColor Red
    exit 1
}

# Create README for the package
$ReadmeContent = @"
# $AppName $Version

## Installation

This is a portable installation. Simply extract and run uncopener.exe.

## URL Scheme Registration

When you first run UncOpener:
1. Open uncopener.exe to access the configuration GUI
2. Configure your scheme name (default: 'unc')
3. Add allowed UNC paths to the allow-list
4. Click "Register Scheme" to register the URL handler

The registration is per-user (HKEY_CURRENT_USER) and does not require administrator privileges.

## Usage

Once registered, clicking links like 'uncopener://server/path/to/resource' in your browser
will open the corresponding UNC path in Windows Explorer.

## Uninstallation

1. Open uncopener.exe
2. Click "Unregister Scheme" to remove the URL handler registration
3. Delete this folder

## More Information

https://github.com/bebuch/UncOpener
"@

$ReadmeContent | Out-File -FilePath (Join-Path $StageDir "README.txt") -Encoding UTF8

# Create ZIP
$ZipPath = Join-Path $ProjectRoot "$OutputDir\$AppName-$Version-windows.zip"
if (Test-Path $ZipPath) {
    Remove-Item $ZipPath
}

Write-Host ""
Write-Host "Creating ZIP archive..." -ForegroundColor Cyan
Compress-Archive -Path "$StageDir\*" -DestinationPath $ZipPath

Write-Host ""
Write-Host "=== Packaging complete ===" -ForegroundColor Green
Write-Host "Output: $ZipPath" -ForegroundColor Cyan
Write-Host ""

# Show contents
Write-Host "Package contents:"
Get-ChildItem $StageDir -Recurse | ForEach-Object {
    $RelPath = $_.FullName.Substring($StageDir.Length + 1)
    Write-Host "  $RelPath"
}
