param (
    [string]$TargetFile = ""
)

Write-Host "====================================================================" -ForegroundColor Cyan
Write-Host "       MISRA / AUTOSAR PRODUCTION QUALITY GATE EVALUATION           " -ForegroundColor Cyan
Write-Host "====================================================================" -ForegroundColor Cyan

# 1. Locate the checker executable
$exePath = Join-Path $PSScriptRoot "..\build\Debug\misra_check.exe"
if (-not (Test-Path $exePath)) {
    $exePath = Join-Path $PSScriptRoot "..\build\Release\misra_check.exe"
}

if (-not (Test-Path $exePath)) {
    Write-Host "[INFO] misra_check.exe not found. Attempting to build project..." -ForegroundColor Yellow
    $cmakeCmd = "cmake"
    try {
        & $cmakeCmd --build "$PSScriptRoot\..\build" --config Debug
    } catch {
        Write-Host "[ERROR] Could not build misra_check.exe. Run 'cmake --build build --config Debug' first." -ForegroundColor Red
        exit 1
    }
}

if (-not (Test-Path $exePath)) {
    Write-Host "[ERROR] Cannot find misra_check.exe at $exePath" -ForegroundColor Red
    exit 1
}

# 2. Determine which files to evaluate
$filesToScan = @()

if ($TargetFile -ne "") {
    if (Test-Path $TargetFile) {
        $filesToScan += (Resolve-Path $TargetFile).Path
    } else {
        Write-Host "[ERROR] Specified target file does not exist: $TargetFile" -ForegroundColor Red
        exit 1
    }
} else {
    # Scan files in outgoing commits, staged in index, or modified in working tree
    $pushedFiles = git diff origin/main..HEAD --name-only --diff-filter=ACM 2>$null
    $stagedFiles = git diff --cached --name-only --diff-filter=ACM 2>$null
    $uncommittedFiles = git diff --name-only --diff-filter=ACM 2>$null
    $allChanged = ($pushedFiles + $stagedFiles + $uncommittedFiles) | Sort-Object -Unique

    foreach ($f in $allChanged) {
        if ($f -match '\.(cpp|cxx|cc)$' -and $f -notmatch 'bad_code\.cpp' -and $f -notmatch 'tests[\\/]tests\.cpp') {
            if (Test-Path $f) {
                $filesToScan += (Resolve-Path $f).Path
            }
        }
    }
}

if ($filesToScan.Count -eq 0) {
    Write-Host "[INFO] No modified C++ production files to evaluate." -ForegroundColor Yellow
    Write-Host "[PASSED] Quality Gate cleared." -ForegroundColor Green
    exit 0
}

# 3. Evaluate each file
$failedFiles = @()

foreach ($file in $filesToScan) {
    $relativeName = Split-Path -Leaf $file
    Write-Host "`n--> Scanning: $relativeName" -ForegroundColor White

    # Run misra_check.exe
    & $exePath $file
    $code = $LASTEXITCODE

    if ($code -ne 0) {
        $failedFiles += $file
    }
}

Write-Host "`n====================================================================" -ForegroundColor Cyan

# 4. Enforce quality gate verdict
if ($failedFiles.Count -gt 0) {
    Write-Host "[VERDICT: REJECTED] Code Quality Gate FAILED!" -ForegroundColor Red
    Write-Host "The following files contain MISRA/AUTOSAR safety rule violations:" -ForegroundColor Red
    foreach ($failed in $failedFiles) {
        Write-Host "  - $(Split-Path -Leaf $failed)" -ForegroundColor Yellow
    }
    Write-Host "`n[ACTION REQUIRED] You MUST fix all reported violations before this code can be committed or pushed to production!" -ForegroundColor Red
    Write-Host "====================================================================" -ForegroundColor Cyan
    exit 1
} else {
    Write-Host "[VERDICT: PASSED] Code Quality Gate CLEARED! (0 violations)" -ForegroundColor Green
    Write-Host "All scanned C++ files comply with MISRA/AUTOSAR safety rules." -ForegroundColor Green
    Write-Host "Code is verified and ready for production / push." -ForegroundColor Green
    Write-Host "====================================================================" -ForegroundColor Cyan
    exit 0
}
