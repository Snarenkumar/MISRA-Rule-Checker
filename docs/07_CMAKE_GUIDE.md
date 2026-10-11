# The Ultimate CMake Guide: From Beginner to Pro

This document explains what CMake is, walks through our project's `CMakeLists.txt` line by line, explains all core CMake commands, details the 3 build commands with real outputs, and diagnoses the 10 most common CMake errors on Windows.

---

## 1. What is CMake and Why Does It Exist?

### The Problem
In C++, every operating system and compiler uses a different compilation tool:
- On Windows, developers use **Visual Studio** and its compiler `cl.exe`. It uses `.sln` (solution) and `.vcxproj` (project) files.
- On Linux, developers use **GCC** (`g++`) with **Makefiles** or **Ninja**.
- On macOS, developers use **Clang** with **Xcode** projects.

If you had to maintain separate Visual Studio project files, Makefiles, and Xcode project files for every C++ project, you would spend more time editing project settings than writing C++ code!

### The Solution: CMake
**CMake** is not a compiler. CMake is a **meta-build system generator**.
- You write a single file named `CMakeLists.txt`.
- On Windows, CMake reads that file and generates a `.sln` and `.vcxproj` files for Visual Studio.
- On Linux, it reads that same file and generates a `Makefile`.
- On macOS, it generates Xcode projects or Ninja files.

You write the build recipe once, and CMake translates it for any operating system in the world.

---

## 2. Line-by-Line Breakdown of Our `CMakeLists.txt`

Below is the complete text of our project's `CMakeLists.txt`, line by line:

### Line 1: `cmake_minimum_required(VERSION 3.20)`
- **What it does:** Specifies that CMake version 3.20 or newer is required to configure this project.
- **Why it matters:** CMake 3.20 introduced clean support for modern C++20 compiler flags and robust `FetchContent` capabilities.

### Line 2: `project(misra_check CXX)`
- **What it does:** Names the project `misra_check` and tells CMake that we are using the C++ programming language (`CXX`).

### Line 3: `set(CMAKE_CXX_STANDARD 20)`
- **What it does:** Sets the global C++ standard to **C++20**.

### Line 4: `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
- **What it does:** Forbids the compiler from silently downgrading to C++17 or C++14 if C++20 features are not supported.

### Line 5: `set(CMAKE_CXX_EXTENSIONS OFF)`
- **What it does:** Disables compiler-specific language extensions (like GNU extensions) so the code strictly follows ISO C++20.

### Line 6: `if(MSVC)`
- **What it does:** Checks if the active compiler is Microsoft Visual C++.

### Line 7: `add_compile_options(/W4 /WX /permissive-)`
- **What it does:** 
  - `/W4`: Enables warning level 4 (all meaningful warnings).
  - `/WX`: Treats all warnings as compile errors (zero tolerance for dirty code).
  - `/permissive-`: Enforces standard conformance.

### Line 8: `else()`
- **What it does:** Executes if the compiler is GCC, Clang, or AppleClang.

### Line 9: `add_compile_options(-Wall -Wextra -Werror -pedantic)`
- **What it does:** Enables all warnings (`-Wall`, `-Wextra`), treats warnings as errors (`-Werror`), and strictly rejects non-standard C++ (`-pedantic`).

### Line 10: `endif()`
- **What it does:** Closes the `if(MSVC)` conditional block.

### Line 11: `add_library(misra_core src/Lexer.cpp src/Parser.cpp src/Rules.cpp src/Reporter.cpp)`
- **What it does:** Compiles the four core source files into a reusable static library named `misra_core`.

### Line 12: `target_include_directories(misra_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)`
- **What it does:** Makes the `include/` folder available to `misra_core` and automatically exports it to any executable linking against `misra_core`.

### Line 13: `add_executable(misra_check src/main.cpp)`
- **What it does:** Creates our command-line executable named `misra_check` from `src/main.cpp`.

### Line 14: `target_link_libraries(misra_check PRIVATE misra_core)`
- **What it does:** Links the `misra_core` static library into `misra_check`.

### Line 15: `include(CTest)`
- **What it does:** Configures testing support and enables CTest.

### Line 16: `include(FetchContent)`
- **What it does:** Loads CMake's `FetchContent` module, which downloads and builds external dependencies from the internet during configuration.

### Line 17: `FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip DOWNLOAD_EXTRACT_TIMESTAMP TRUE)`
- **What it does:** Declares the name, download URL, and extract settings for the GoogleTest library.

### Line 18: `set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)`
- **What it does:** Forces GoogleTest on Windows to use the shared C runtime matching our project.

### Line 19: `FetchContent_MakeAvailable(googletest)`
- **What it does:** Downloads GoogleTest, extracts it, and configures its build targets (`GTest::gtest` and `GTest::gtest_main`).

### Line 20: `add_executable(unit_tests tests/tests.cpp)`
- **What it does:** Compiles our test suite `tests/tests.cpp` into an executable named `unit_tests`.

### Line 21: `target_link_libraries(unit_tests PRIVATE misra_core GTest::gtest_main)`
- **What it does:** Links our core library and GoogleTest's ready-to-use `main()` runner into `unit_tests`.

### Line 22: `add_test(NAME AllTests COMMAND unit_tests)`
- **What it does:** Registers `unit_tests` with CTest under the test name `AllTests`.

---

## 3. Core CMake Keywords Explained

Here is what each fundamental CMake command does:

| Command | Purpose | Example |
|---|---|---|
| `project(name LANG)` | Declares the project name and programming languages. | `project(my_tool CXX)` |
| `set(VAR value)` | Sets a CMake variable to a specific value. | `set(CMAKE_CXX_STANDARD 20)` |
| `add_library(name sources...)` | Compiles source files into a `.lib` (static) or `.dll` (shared) library. | `add_library(core src/a.cpp)` |
| `add_executable(name sources...)` | Compiles source files into a runnable program (`.exe`). | `add_executable(tool src/main.cpp)` |
| `target_include_directories(...)` | Tells the compiler which folder contains header files (`#include`). | `target_include_directories(core PUBLIC include)` |
| `target_link_libraries(...)` | Connects a library to an executable so functions can be called. | `target_link_libraries(tool PRIVATE core)` |
| `include(FetchContent)` | Includes a CMake module that downloads third-party code from Git/HTTP. | `include(FetchContent)` |
| `FetchContent_Declare(...)` | Specifies the URL/tag where a dependency lives. | `FetchContent_Declare(googletest URL ...)` |
| `FetchContent_MakeAvailable(...)` | Downloads, unpacks, and integrates the external dependency. | `FetchContent_MakeAvailable(googletest)` |
| `enable_testing()` / `include(CTest)` | Turns on automated testing features in CMake. | `include(CTest)` |
| `add_test(NAME name COMMAND target)` | Tells CTest to run a specific executable and check its exit code. | `add_test(NAME Test1 COMMAND unit_tests)` |
| `ctest` | The command-line test runner provided by CMake. | `ctest --test-dir build -C Debug` |

---

## 4. The 3 Build Commands and the `build/` Folder

### Command 1: Configure
```powershell
cmake -B build -S .
```
- `-B build`: Specifies the build output directory (`build/`).
- `-S .`: Specifies the source directory (`.` means current folder).
- **What it does:** Checks your compiler, checks Windows SDK, downloads GoogleTest, and creates Visual Studio solution files (`misra_check.sln`, `misra_check.vcxproj`, `unit_tests.vcxproj`).

### Command 2: Build
```powershell
cmake --build build --config Debug
```
- `--build build`: Tells CMake to call the underlying build tool (MSBuild on Windows) inside `build/`.
- `--config Debug`: Selects Debug configuration (with debugging symbols).
- **What it does:** Compiles C++ sources, creates `misra_core.lib`, `misra_check.exe`, and `unit_tests.exe`.

### Command 3: Test
```powershell
ctest --test-dir build -C Debug --output-on-failure
```
- `--test-dir build`: Points CTest to the build directory.
- `-C Debug`: Selects Debug binaries.
- `--output-on-failure`: If any test fails, prints the exact error message and assertion details.

### What the `build/` Folder Contains
When CMake runs, it puts all generated files in `build/`:
- `build/Debug/misra_check.exe`: The finished CLI tool.
- `build/Debug/unit_tests.exe`: The unit test runner.
- `build/Debug/misra_core.lib`: The compiled static library.
- `build/_deps/`: The extracted GoogleTest source code.
- `build/CMakeCache.txt`: The cached settings (compiler paths, flags).
- `build/CMakeFiles/`: Internal temporary files and logs.

---

## 5. The 10 Most Common CMake Errors on Windows & How to Fix Them

### 1. `CMake Error: The source directory ... does not contain CMakeLists.txt`
- **Cause:** You ran the command from the wrong folder or forgot the `-S .` flag.
- **Fix:** Navigate to the folder containing `CMakeLists.txt` and run `cmake -B build -S .`.

### 2. `No CMAKE_CXX_COMPILER could be found`
- **Cause:** Visual Studio C++ compiler tools are not installed.
- **Fix:** Open Visual Studio Installer, click Modify, check **"Desktop development with C++"**, and install.

### 3. `Target "run_tests" collides with CMake target "RUN_TESTS"`
- **Cause:** When you call `include(CTest)`, Visual Studio creates a built-in target named `RUN_TESTS`. On Windows (which has case-insensitive file systems), a custom target named `run_tests` causes a name collision.
- **Fix:** Name your test executable something unique, like `unit_tests`.

### 4. `FetchContent: Could not download dependency (HTTP error)`
- **Cause:** No internet connection or a firewall blocking GitHub downloads during initial configure.
- **Fix:** Connect to the internet for the first configure command. Once downloaded into `build/_deps/`, it is cached locally.

### 5. `LNK2019: unresolved external symbol`
- **Cause:** You declared a function in a header file (`.h`) but forgot to list the implementation file (`.cpp`) in `add_library` or `add_executable`.
- **Fix:** Add the `.cpp` file to `add_library` in `CMakeLists.txt`.

### 6. `fatal error C1083: Cannot open include file: 'Lexer.h'`
- **Cause:** The compiler does not know where your headers live.
- **Fix:** Ensure `target_include_directories(misra_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)` is present.

### 7. `MSBuild: error MSB3073: ... exited with code 1`
- **Cause:** A pre-build or post-build event (such as a failing test) failed during compilation.
- **Fix:** Check the compiler output above the error line to see the specific C++ compilation error.

### 8. `CMakeCache.txt contains stale compiler paths`
- **Cause:** You moved the repository to a different folder or changed Visual Studio versions.
- **Fix:** Delete the `build/` folder completely (`Remove-Item -Recurse -Force build`) and re-run `cmake -B build -S .`.

### 9. `/WX: warning treated as error (warning C4189: local variable is initialized but not referenced)`
- **Cause:** You declared a variable but did not use it. Because `/WX` is enabled, MSVC treats warnings as errors.
- **Fix:** Either use the variable or remove it.

### 10. `CTest: 0 tests found`
- **Cause:** You forgot `add_test(NAME AllTests COMMAND unit_tests)` in your `CMakeLists.txt`.
- **Fix:** Add the `add_test` call at the bottom of `CMakeLists.txt`.
