# Production Quality Gate & CI/CD Pipeline Guide

This guide explains the automated **Production Quality Gate** built into this repository.

---

## 1. How It Works (The 2-Layer Protection)

We have implemented two defense layers to guarantee bad code never reaches production:

```
┌─────────────────────────────────────────────────────────────┐
│ LAYER 1: LOCAL GIT HOOK (.git/hooks/pre-push)                │
│ Runs immediately on your machine when you type 'git push'   │
│ ❌ Rejects and halts push if violations are found            │
│ ✅ Allows push only if 0 violations                          │
└──────────────────────────────┬──────────────────────────────┘
                               │ (Only if Layer 1 passes)
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ LAYER 2: GITHUB ACTIONS CI (.github/workflows/misra_gate.yml)│
│ Runs in the cloud on GitHub for every push or Pull Request   │
│ ❌ Marks build RED, blocks PR merge if violations exist      │
│ ✅ Marks build GREEN when compliant                          │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Testing the Pipeline with a New File (Step-by-Step)

Follow these steps to see the pipeline catch and reject dangerous code:

### Step A: Create a New C++ File with Violations
Create a new file named `test_feature.cpp` in the repository root:

```cpp
// test_feature.cpp
void testVehicleBrakes() {
    int pressure; // VIOLATION: UNINIT_VAR
    goto emergency_stop; // VIOLATION: NO_GOTO
emergency_stop:
    return;
}
```

### Step B: Try to Commit and Push
Run these commands in PowerShell:

```powershell
git add test_feature.cpp
git commit -m "Add new vehicle brake feature"
git push origin main
```

### Step C: Watch Git Automatically Reject the Push!
Git will run the `.git/hooks/pre-push` hook, evaluate the code, and print:

```text
🚀 [GIT HOOK] Verifying MISRA/AUTOSAR rules before push...
====================================================================
       MISRA / AUTOSAR PRODUCTION QUALITY GATE EVALUATION           
====================================================================

--> Scanning: test_feature.cpp
test_feature.cpp:2: [UNINIT_VAR] Variable 'pressure' uninitialized.
test_feature.cpp:3: [NO_GOTO] Use of 'goto' is prohibited.
Found 2 violations.

====================================================================
[VERDICT: REJECTED] Code Quality Gate FAILED!
The following files contain MISRA/AUTOSAR safety rule violations:
  - test_feature.cpp

[ACTION REQUIRED] You MUST fix all reported violations before this code can be committed or pushed to production!
====================================================================

❌ [PUSH ABORTED] Your push was blocked by the MISRA Quality Gate.
👉 Fix all violations shown above, re-build, and try pushing again.
```

**Result:** The push is completely **cancelled by Git**. The broken code is prevented from reaching GitHub!

---

### Step D: Fix the Code and Push Successfully

Now fix `test_feature.cpp` to make it safe:

```cpp
// test_feature.cpp - Compliant version
void testVehicleBrakes() {
    int pressure = 0; // Fixed: initialized
    if (pressure <= 0) {
        return; // Fixed: structured control flow instead of goto
    }
}
```

Stage and commit the fix:
```powershell
git add test_feature.cpp
git commit -m "Fix MISRA violations in brake feature"
git push origin main
```

**Result:**
```text
🚀 [GIT HOOK] Verifying MISRA/AUTOSAR rules before push...
====================================================================
[VERDICT: PASSED] Code Quality Gate CLEARED! (0 violations)
====================================================================
✅ [PUSH ALLOWED] Quality Gate passed. Proceeding with push to remote repository...
To https://github.com/Snarenkumar/MISRA-Rule-Checker.git
   ... -> main
```

---

## 3. Manual Quality Gate Check

You can also run the evaluation script manually at any time without pushing:

```powershell
# Evaluate all modified files
.\scripts\check_quality_gate.ps1

# Evaluate a specific file
.\scripts\check_quality_gate.ps1 -TargetFile "samples/good_code.cpp"
```

---

## 4. Summary of Files Added for the Pipeline

- **`.github/workflows/misra_quality_gate.yml`**: GitHub Actions cloud pipeline.
- **`.git/hooks/pre-push`**: Local Git hook blocking pushes with violations.
- **`scripts/check_quality_gate.ps1`**: Core PowerShell evaluation script.
- **`scripts/setup_git_gate.ps1`**: Installer script to enable the hooks on any computer.
