# How to Run `misra_check` (Step-by-Step Guide)

Welcome! This guide teaches you how to install requirements, build the project, run tests, analyze your own C++ code, and fix common errors on Windows.

---

## 1. System Requirements & How to Check Them

To build and run this project on Windows, you need three tools:
1. **Git** (tracks source code history)
2. **CMake** (generates project build files)
3. **C++ Compiler** (compiles C++ code into an executable program)

### How to Check If You Have Them
Open **PowerShell** and run these three commands:

```powershell
# 1. Check Git
git --version

# 2. Check CMake
cmake --version

# 3. Check C++ Compiler (MSVC cl.exe)
cl
```

### What You Should See
- `git --version` should print something like `git version 2.40.0` or higher.
- `cmake --version` should print `cmake version 3.20.0` or higher.
- `cl` should print `Microsoft (R) C/C++ Optimizing Compiler Version 19...`

### If Something Is Missing: How to Install
- **Git**: Download and run the official installer from [https://git-scm.com](https://git-scm.com). Check the box that says "Add Git to PATH".
- **CMake & MSVC Compiler**:
  1. Download **Visual Studio Community** (or **Visual Studio Build Tools**) from [https://visualstudio.microsoft.com](https://visualstudio.microsoft.com).
  2. In the Visual Studio Installer, select the workload: **"Desktop development with C++"**.
  3. Ensure the optional component **"C++ CMake tools for Windows"** is checked.
  4. Complete the installation and restart PowerShell.

---

## 2. The 3 Exact Build Commands

From the root directory of this repository (`f:\main servers under productions\MISRAAUTOSAR`), run these commands in PowerShell:

### Step 1: Configure
Tells CMake to inspect your compiler and set up a `build/` folder:
```powershell
cmake -B build -S .
```
**Real Output:**
```
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- Configuring done (0.7s)
-- Generating done (0.1s)
-- Build files have been written to: F:/main servers under productions/MISRAAUTOSAR/build
```

### Step 2: Build
Compiles all C++ code, builds the `misra_core` library, the `misra_check.exe` CLI tool, and the test suite:
```powershell
cmake --build build --config Debug
```
**Real Output:**
```
MSBuild version 18.8.2+ce25c0108 for .NET Framework

  gmock.vcxproj -> F:\main servers under productions\MISRAAUTOSAR\build\lib\Debug\gmock.lib
  gtest.vcxproj -> F:\main servers under productions\MISRAAUTOSAR\build\lib\Debug\gtest.lib
  misra_core.vcxproj -> F:\main servers under productions\MISRAAUTOSAR\build\Debug\misra_core.lib
  misra_check.vcxproj -> F:\main servers under productions\MISRAAUTOSAR\build\Debug\misra_check.exe
  unit_tests.vcxproj -> F:\main servers under productions\MISRAAUTOSAR\build\Debug\unit_tests.exe
```

### Step 3: Run the Automated Unit Tests
Runs all unit tests using CTest:
```powershell
ctest --test-dir build -C Debug --output-on-failure
```
**Real Output:**
```
Test project F:/main servers under productions/MISRAAUTOSAR/build
    Start 1: AllTests
1/1 Test #1: AllTests .........................   Passed    0.57 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.57 sec
```

To see every test case by name, run the test runner executable directly:
```powershell
.\build\Debug\unit_tests.exe
```
This runs all **36 tests** across `RulesTest`, `SuppressionTest`, and `ReporterTest`. All 36 tests pass!

---

## 3. Running the Tool on Sample Files

Our project includes two real sample files in the `samples/` directory:
- `samples/bad_code.cpp` (violates all 15 rules)
- `samples/good_code.cpp` (safe, compliant code with 0 violations)

### 1. Plain Text Output on `bad_code.cpp`
```powershell
.\build\Debug\misra_check.exe samples\bad_code.cpp
```
**Real Terminal Output:**
```
samples\bad_code.cpp:4: [NO_USING_NAMESPACE] Use of 'using namespace' is prohibited.
samples\bad_code.cpp:7: [NO_RECURSION] Recursion detected: recursive_function -> recursive_function
samples\bad_code.cpp:12: [MAX_FUNC_LENGTH] Function length exceeds 50 lines.
samples\bad_code.cpp:67: [MAX_FUNC_LENGTH] Function length exceeds 50 lines.
samples\bad_code.cpp:69: [UNINIT_VAR] Variable 'uninit_var' uninitialized.
samples\bad_code.cpp:72: [NO_NEW] Dynamic allocation with 'new' is prohibited.
samples\bad_code.cpp:75: [NO_DELETE] Deallocation with 'delete' is prohibited.
samples\bad_code.cpp:78: [NO_MALLOC_FAMILY] Memory function 'malloc' is prohibited.
samples\bad_code.cpp:79: [NO_MALLOC_FAMILY] Memory function 'free' is prohibited.
samples\bad_code.cpp:82: [UNINIT_VAR] Variable 'buf' uninitialized.
samples\bad_code.cpp:83: [NO_BANNED_FUNCS] Banned function 'gets' is prohibited.
samples\bad_code.cpp:84: [NO_BANNED_FUNCS] Banned function 'sprintf' is prohibited.
samples\bad_code.cpp:85: [NO_BANNED_FUNCS] Banned function 'atoi' is prohibited.
samples\bad_code.cpp:89: [NO_ASSIGN_IN_COND] Assignment in condition is prohibited.
samples\bad_code.cpp:95: [IF_BRACES] 'if' body must use braces.
samples\bad_code.cpp:99: [LOOP_BRACES] 'while' body must use braces.
samples\bad_code.cpp:104: [NO_CONTINUE] Use of 'continue' is prohibited.
samples\bad_code.cpp:108: [SWITCH_DEFAULT] 'switch' missing 'default:' label.
samples\bad_code.cpp:114: [NO_GOTO] Use of 'goto' is prohibited.
samples\bad_code.cpp:122: [MAX_NESTING] Nesting depth exceeds 4.
Found 20 violations.
```

#### How to Read This Output:
Each violation line has 4 clear parts:
1. `samples\bad_code.cpp`: The file containing the issue.
2. `114`: The exact line number in the file.
3. `[NO_GOTO]`: The unique identifier of the safety rule that was broken.
4. `Use of 'goto' is prohibited.`: A plain English explanation of the rule.
5. `Found 20 violations.`: The total violation count at the end. The program exits with status code `1` indicating failure.

---

### 2. Plain Text Output on `good_code.cpp`
```powershell
.\build\Debug\misra_check.exe samples\good_code.cpp
```
**Real Terminal Output:**
```
Found 0 violations.
```
The program exits with status code `0` indicating clean success.

---

### 3. JSON Output with `--json`
```powershell
.\build\Debug\misra_check.exe samples\bad_code.cpp --json
```
**Real Output (Snippet):**
```json
{
  "violations": [
    {
      "file": "samples\\bad_code.cpp",
      "line": 4,
      "rule": "NO_USING_NAMESPACE",
      "message": "Use of 'using namespace' is prohibited."
    },
    {
      "file": "samples\\bad_code.cpp",
      "line": 114,
      "rule": "NO_GOTO",
      "message": "Use of 'goto' is prohibited."
    }
  ],
  "summary": { "total": 20 }
}
```
**How to use it:** Pass this JSON directly to Python scripts or web dashboards to generate charts of code health over time.

---

### 4. SARIF Output with `--sarif`
```powershell
.\build\Debug\misra_check.exe samples\bad_code.cpp --sarif
```
**Real Output (Snippet):**
```json
{
  "version": "2.1.0",
  "$schema": "https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json",
  "runs": [
    {
      "tool": { "driver": { "name": "misra_check", "version": "1.0.0" } },
      "results": [
        {
          "ruleId": "NO_GOTO",
          "message": { "text": "Use of 'goto' is prohibited." },
          "locations": [{ "physicalLocation": { "artifactLocation": { "uri": "samples\\bad_code.cpp" }, "region": { "startLine": 114 } } }]
        }
      ]
    }
  ]
}
```
**What is SARIF:** **SARIF** stands for *Static Analysis Results Interchange Format*. It is the official open industry standard (OASIS) used by GitHub Code Scanning, SonarQube, and Microsoft Visual Studio. If you run `misra_check` in GitHub Actions, you can upload the SARIF file and GitHub will annotate the pull request lines with red warnings automatically!

---

## 4. How to Check Your Own C++ File

You can check any C++ file you write! Here is how:

1. **Step 1:** Create your C++ file anywhere. For example, create `my_code.cpp` in the repository root:
   ```cpp
   // my_code.cpp
   void drive() {
       int speed; // Uninitialized!
       if (speed > 50) 
           speed = 50; // Missing braces!
   }
   ```
2. **Step 2:** Run `misra_check.exe` pointing to your file:
   ```powershell
   .\build\Debug\misra_check.exe my_code.cpp
   ```
3. **Step 3:** Read the output:
   ```
   my_code.cpp:3: [UNINIT_VAR] Variable 'speed' uninitialized.
   my_code.cpp:4: [IF_BRACES] 'if' body must use braces.
   Found 2 violations.
   ```
4. **Step 4:** Fix the issues in your file:
   ```cpp
   // my_code.cpp - FIXED
   void drive() {
       int speed = 0; // Initialized
       if (speed > 50) {
           speed = 50; // Braced
       }
   }
   ```
5. **Step 5:** Re-run the tool:
   ```powershell
   .\build\Debug\misra_check.exe my_code.cpp
   ```
   Output: `Found 0 violations.` You are safe!

---

## 5. How to Silence Warnings with `// NOLINT`

Sometimes, you must interact with old legacy code or a hardware driver that requires a specific construct. You can tell `misra_check` to intentionally skip a line using `// NOLINT`.

### Example 1: Same-Line Wildcard Suppression
- **Before (Fails with NO_GOTO):**
  ```cpp
  void jump() {
      goto escape; // VIOLATION: NO_GOTO
  escape:
      return;
  }
  ```
- **After (Silenced):**
  ```cpp
  void jump() {
      goto escape; // NOLINT
  escape:
      return;
  }
  ```

### Example 2: Specific Rule Suppression
You can name the exact rule to silence so other rules still work:
```cpp
void jump() {
    goto escape; // NOLINT(NO_GOTO)
escape:
    return;
}
```

### Example 3: Next-Line Suppression
You can place the comment on the line right before the code:
```cpp
void jump() {
    // NOLINTNEXTLINE(NO_GOTO)
    goto escape;
escape:
    return;
}
```

---

## 6. Command-Line Options Explained

| Flag | Meaning | What it does |
|---|---|---|
| `<file.cpp>` | Input File | The path to the C++ source file you want to inspect. |
| `--json` | Output Format | Prints the violation report in JSON format instead of plain text. |
| `--sarif` | Output Format | Prints the violation report in SARIF 2.1.0 format for CI/CD tools. |
| `--help` / `-h` | Help Screen | Prints usage instructions: `Usage: misra_check <file.cpp> [--json] [--sarif]`. |

---

## 7. The 10 Most Common Errors & How to Fix Them

### Error 1: `'cmake' is not recognized as an internal or external command`
- **Cause:** CMake is either not installed or its `bin` folder is not in your system's PATH variable.
- **Fix:** Install CMake and check the box "Add CMake to the system PATH for all users". Or use the Developer Command Prompt for Visual Studio.

### Error 2: `No CMAKE_CXX_COMPILER could be found`
- **Cause:** CMake cannot find Microsoft Visual C++ (`cl.exe`) or GCC.
- **Fix:** Open Visual Studio Installer, click **Modify**, select **Desktop development with C++**, and install it.

### Error 3: `Error: cannot open file 'xyz.cpp'`
- **Cause:** You typed the path to a file that does not exist.
- **Fix:** Check your spelling or use tab-completion in PowerShell: `.\build\Debug\misra_check.exe samples\bad_code.cpp`.

### Error 4: `LINK : fatal error LNK1104: cannot open file ... misra_check.exe`
- **Cause:** `misra_check.exe` is currently running in another terminal window or debugger, so Windows locked the file.
- **Fix:** Close any other open terminal windows running `misra_check.exe` and re-run the build.

### Error 5: `AllTests FAILED: Unable to find executable run_tests.exe`
- **Cause:** In Visual Studio multi-config builds, `RUN_TESTS` is a built-in project name.
- **Fix:** Our project names the test executable `unit_tests.exe` to avoid this exact collision. Run `cmake --build build --config Debug` to compile it.

### Error 6: `warning C4100: unreferenced formal parameter` treated as error
- **Cause:** The project uses `/WX` (treat warnings as errors). If you leave an unused function argument, MSVC stops the build.
- **Fix:** In C++, omit the parameter name if it is unused: `void check(const std::string& /* f */)`.

### Error 7: `git push fails: remote contains work that you do not have locally`
- **Cause:** Someone made a commit on GitHub that you haven't pulled yet.
- **Fix:** Run `git pull origin main --rebase` before running `git push`.

### Error 8: `git asks for username and password in terminal`
- **Cause:** GitHub no longer accepts account passwords for command-line Git operations.
- **Fix:** You must use a GitHub **Personal Access Token (PAT)** or install the **GitHub CLI** (`gh auth login`).

### Error 9: `fatal: not a git repository`
- **Cause:** You are running git commands from the wrong folder, or `.git` was deleted.
- **Fix:** Navigate to `f:\main servers under productions\MISRAAUTOSAR` and run `git init`.

### Error 10: `test project build failed due to internet connection`
- **Cause:** On the very first CMake configure, CMake uses `FetchContent` to download GoogleTest from GitHub. If you have no internet connection, it cannot download GoogleTest.
- **Fix:** Connect to Wi-Fi/Internet for the initial configure. Once GoogleTest is downloaded into `build/_deps/`, you can work completely offline!
