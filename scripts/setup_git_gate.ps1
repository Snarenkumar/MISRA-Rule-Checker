# Installs the MISRA/AUTOSAR pre-push and pre-commit hooks into the local Git repository

$gitHooksDir = Join-Path $PSScriptRoot "..\.git\hooks"

if (-not (Test-Path $gitHooksDir)) {
    Write-Host "[ERROR] .git/hooks directory not found. Make sure you are in a git repository." -ForegroundColor Red
    exit 1
}

$prePushHook = @"
#!/bin/sh
# Git Pre-Push Hook: MISRA/AUTOSAR Production Quality Gate
echo ""
echo "🚀 [GIT HOOK] Verifying MISRA/AUTOSAR rules before push..."
HOOK_DIR="\$(cd "\$(dirname "\$0")" && pwd)"
ROOT_DIR="\$(cd "\$HOOK_DIR/../.." && pwd)"

powershell.exe -ExecutionPolicy Bypass -File "\$ROOT_DIR/scripts/check_quality_gate.ps1"
RESULT=\$?

if [ \$RESULT -ne 0 ]; then
    echo ""
    echo "❌ [PUSH ABORTED] Your push was blocked by the MISRA Quality Gate."
    echo "👉 Fix all violations shown above and try pushing again."
    echo ""
    exit 1
fi

echo "✅ [PUSH ALLOWED] Quality Gate passed. Proceeding with push..."
echo ""
exit 0
"@

$prePushPath = Join-Path $gitHooksDir "pre-push"
Set-Content -Path $prePushPath -Value $prePushHook -NoNewline

Write-Host "====================================================================" -ForegroundColor Green
Write-Host "✅ MISRA/AUTOSAR Git Hook successfully installed to:" -ForegroundColor Green
Write-Host "   $prePushPath" -ForegroundColor Cyan
Write-Host ""
Write-Host "Every time you run 'git push', the tool will automatically:" -ForegroundColor White
Write-Host "  1. Scan all modified / newly added .cpp files." -ForegroundColor White
Write-Host "  2. Evaluate them against all 15 MISRA/AUTOSAR rules." -ForegroundColor White
Write-Host "  3. REJECT and ABORT the push if any violations are found." -ForegroundColor Yellow
Write-Host "  4. ALLOW the push only when 0 violations are found." -ForegroundColor Green
Write-Host "====================================================================" -ForegroundColor Green
