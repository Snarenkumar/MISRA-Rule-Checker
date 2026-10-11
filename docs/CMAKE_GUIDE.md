# Beginner Guide to CMake

## 1. What is CMake?

In C++, you don't compile code with a single magic click unless a build system manages it for you. 
Different operating systems have different compilers:
- Windows uses MSVC (`cl.exe`).
- Linux uses GCC (`g++`).
- macOS uses Clang (`clang++`).

Each compiler takes completely different flags and command arguments. 

**CMake is a build generator.** 
You write a single configuration file named `CMakeLists.txt`, and CMake automatically translates it into the native project files for your system (such as Visual Studio solution files `.sln` on Windows, or Makefiles/Ninja on Linux).

---

## 2. Line-by-Line Explanation of Our `CMakeLists.txt`

Below is every single line from our project's `CMakeLists.txt` explained in plain English:

### Line 1: `cmake_minimum_required(VERSION 3.20)`
- **What it does**: Tells CMake that we require at least CMake version 3.20.
- **Why**: CMake 3.20 provides modern support for C++20 and FetchContent.

### Line 2: `project(misra_check CXX)`
- **What it does**: Names the project `misra_check` and specifies that we are using the C++ language (`CXX`).

### Line 3: `set(CMAKE_CXX_STANDARD 20)`
- **What it does**: Specifies that our compiler must compile in C++20 mode.

### Line 4: `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
- **What it does**: Forbids the compiler from silently falling back to older C++ standards if C++20 isn't available.

### Line 5: `set(CMAKE_CXX_EXTENSIONS OFF)`
- **What it does**: Disables compiler-specific extensions (like GNU extensions) so our code adheres strictly to standard ISO C++.

### Line 6: `if(MSVC)`
- **What it does**: Checks if the active compiler is Microsoft Visual C++.

### Line 7: `add_compile_options(/W4 /WX /permissive-)`
- **What it does**: 
  - `/W4`: Turns on warning level 4 (all meaningful warnings).
  - `/WX`: Treats all warnings as compile errors (zero tolerance for dirty code).
  - `/permissive-`: Forces strict C++ standard conformity.

### Line 8: `else()`
- **What it does**: Runs if the compiler is NOT MSVC (e.g., GCC or Clang on Linux/macOS).

### Line 9: `add_compile_options(-Wall -Wextra -Werror -pedantic)`
- **What it does**: 
  - `-Wall` & `-Wextra`: Turns on standard and extra warnings.
  - `-Werror`: Treats warnings as errors.
  - `-pedantic`: Rejects non-standard language constructs.

### Line 10: `endif()`
- **What it does**: Closes the `if(MSVC)` conditional block.

### Line 11: `add_library(misra_core src/Lexer.cpp src/Parser.cpp src/Rules.cpp src/Reporter.cpp)`
- **What it does**: Bundles our four core analysis source files into a static library named `misra_core`.

### Line 12: `target_include_directories(misra_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)`
- **What it does**: Exposes the `include/` directory to `misra_core` and to any target that links against it so `#include "Lexer.h"` works everywhere.

### Line 13: `add_executable(misra_check src/main.cpp)`
- **What it does**: Tells CMake to create our command-line executable `misra_check` from `src/main.cpp`.

### Line 14: `target_link_libraries(misra_check PRIVATE misra_core)`
- **What it does**: Links the static library `misra_core` into `misra_check`.

### Line 15: `include(CTest)`
- **What it does**: Enables the built-in CTest framework to discover and run tests.

### Line 16: `include(FetchContent)`
- **What it does**: Loads CMake's `FetchContent` module, which automatically downloads third-party dependencies during configure time.

### Line 17: `FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip DOWNLOAD_EXTRACT_TIMESTAMP TRUE)`
- **What it does**: Specifies where to download GoogleTest v1.14.0 zip from GitHub.

### Line 18: `set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)`
- **What it does**: On Windows, prevents GoogleTest from trying to use a mismatched runtime library (matches `/MD` or `/MDd`).

### Line 19: `FetchContent_MakeAvailable(googletest)`
- **What it does**: Downloads, extracts, and compiles GoogleTest as part of the build.

### Line 20: `add_executable(unit_tests tests/tests.cpp)`
- **What it does**: Creates our test executable `unit_tests` from `tests/tests.cpp`.

### Line 21: `target_link_libraries(unit_tests PRIVATE misra_core GTest::gtest_main)`
- **What it does**: Links our core library and GoogleTest's ready-to-run `main()` function into the test runner.

### Line 22: `add_test(NAME AllTests COMMAND unit_tests)`
- **What it does**: Registers `unit_tests` with CTest under the test name `AllTests`.

---

## 3. The 3 Build Commands

To build and run this project from a terminal (PowerShell or Bash) in the repository root:

### Step 1: Configure
Generates the build files inside a `build/` folder:
```powershell
cmake -B build -S .
```

### Step 2: Build
Compiles the static library, CLI tool, and test runner:
```powershell
cmake --build build --config Debug
```

### Step 3: Run Tests
Executes the test suite with CTest:
```powershell
ctest --test-dir build -C Debug --output-on-failure
```

---

## 4. Running the Checker CLI

After building, run the checker on code samples:

```powershell
# Plain text report
.\build\Debug\misra_check.exe samples\bad_code.cpp

# JSON report
.\build\Debug\misra_check.exe samples\bad_code.cpp --json

# SARIF report (standard for GitHub code scanning & VS Code)
.\build\Debug\misra_check.exe samples\bad_code.cpp --sarif

# Clean code (should return 0 violations and exit code 0)
.\build\Debug\misra_check.exe samples\good_code.cpp
```
