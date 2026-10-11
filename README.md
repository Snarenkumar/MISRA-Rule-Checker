# misra_check: MISRA/AUTOSAR Static Analysis Tool

A compact, student-friendly static analysis CLI tool built from scratch in C++20 without external parsing libraries. It verifies C++ code compliance against 15 essential MISRA/AUTOSAR safety rules.

## Pipeline
`Lexer` -> `Parser` (AST) -> `CallGraph` (3-Color DFS) -> `RuleEngine` (15 Rules + `// NOLINT`) -> `Reporter` (Text / JSON / SARIF)

## The 15 Rules
1. `NO_GOTO` - Prohibits `goto` jumps.
2. `NO_NEW` - Prohibits dynamic heap allocation with `new`.
3. `NO_DELETE` - Prohibits deallocation with `delete`.
4. `NO_MALLOC_FAMILY` - Prohibits `malloc`, `calloc`, `realloc`, `free`.
5. `NO_CONTINUE` - Prohibits `continue` statements in loops.
6. `NO_USING_NAMESPACE` - Prohibits `using namespace` to avoid symbol pollution.
7. `NO_BANNED_FUNCS` - Prohibits unsafe functions (`gets`, `sprintf`, `atoi`).
8. `NO_ASSIGN_IN_COND` - Prohibits accidental assignment `=` in conditions.
9. `IF_BRACES` - Enforces compound `{ }` braces on `if`/`else` branches.
10. `LOOP_BRACES` - Enforces compound `{ }` braces on `while`/`for` loops.
11. `SWITCH_DEFAULT` - Requires every `switch` to have a `default:` label.
12. `UNINIT_VAR` - Prohibits uninitialized variable declarations.
13. `MAX_NESTING` - Prohibits nesting depth exceeding 4 levels.
14. `MAX_FUNC_LENGTH` - Prohibits function definitions exceeding 50 lines.
15. `NO_RECURSION` - Detects direct and indirect recursive call cycles using 3-color DFS.

## Project Structure
```
include/
  Lexer.h, Parser.h, Rules.h, Reporter.h
src/
  Lexer.cpp, Parser.cpp, Rules.cpp, Reporter.cpp, main.cpp
tests/
  tests.cpp (36 GoogleTest unit tests)
samples/
  bad_code.cpp, good_code.cpp
docs/
  EXPLAIN.md, CMAKE_GUIDE.md
CMakeLists.txt, README.md, .gitignore
```

## Quick Build & Test (Windows PowerShell)
```powershell
# 1. Configure
cmake -B build -S .

# 2. Build
cmake --build build --config Debug

# 3. Test
ctest --test-dir build -C Debug --output-on-failure
```

## Running the CLI
```powershell
# Run on sample bad code
.\build\Debug\misra_check.exe samples\bad_code.cpp

# JSON report
.\build\Debug\misra_check.exe samples\bad_code.cpp --json

# SARIF report
.\build\Debug\misra_check.exe samples\bad_code.cpp --sarif

# Run on sample good code (0 violations)
.\build\Debug\misra_check.exe samples\good_code.cpp
```
