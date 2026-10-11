# Project File Structure & Code Map

This document explains the organization of every directory and file in the `misra_check` project. 
It provides a complete map of the codebase and includes a step-by-step reading order with realistic time estimates for students.

---

## 1. Complete Project Folder Tree

```
MISRA-Rule-Checker/
│
├── .gitignore                   # Tells Git which files not to track (build/, *.exe, etc.)
├── CMakeLists.txt               # Build configuration (C++20, MSVC/GCC flags, GoogleTest)
├── README.md                    # Project overview and quick start guide
│
├── docs/                        # Comprehensive documentation library
│   ├── 01_PROJECT_FLOW.md       # Architecture pipeline and all 15 rules explained
│   ├── 02_HOW_TO_RUN.md         # Windows setup, CLI commands, real outputs, troubleshooting
│   ├── 03_FILE_STRUCTURE.md     # This file: folder tree, file details, and reading order
│   ├── 04_LINE_BY_LINE_EXECUTION.md # Walkthrough of a tiny input file and line-by-line code
│   ├── 05_REAL_WORLD_AND_WHY.md # Automotive safety, MISRA/AUTOSAR context, real-world tools
│   ├── 06_FACTS_FUN_AND_UNIQUE.md # Fun facts, analogies, project comparison table
│   └── 07_CMAKE_GUIDE.md        # Line-by-line CMake guide and build system explanations
│
├── include/                     # C++ header files (interfaces & declarations)
│   ├── Lexer.h                  # Token struct, TokenType enum, Lexer class declaration
│   ├── Parser.h                 # AST node structs (Stmt, IfStmt, etc.) & Parser class declaration
│   ├── Reporter.h               # OutputFormat enum & Reporter class declaration
│   └── Rules.h                  # Violation, CallGraph, Rule base class, 15 rule classes, RuleEngine
│
├── samples/                     # Example C++ files for demonstration and verification
│   ├── bad_code.cpp             # Triggers all 15 MISRA/AUTOSAR rules (20 violations)
│   └── good_code.cpp            # Safe, fully compliant C++ code (0 violations)
│
├── src/                         # C++ implementation files (.cpp)
│   ├── Lexer.cpp                # Scanning characters into tokens & tracking // NOLINT
│   ├── Parser.cpp               # Recursive descent parser producing the AST
│   ├── Reporter.cpp             # Text, JSON, and SARIF 2.1.0 report generators
│   ├── Rules.cpp                # Implementations of 15 rules, CallGraph with 3-color DFS, RuleEngine
│   └── main.cpp                 # CLI main entry point, argument parsing, file reading
│
├── study/                       # Private interview preparation & pitch notes (ignored by git)
│   ├── 08_INTERVIEW_QA.md       # 60+ detailed interview questions and answers
│   └── 09_PITCH_MOCK_AND_CHEATSHEET.md # Elevator pitch, mock interview, cheat sheet
│
└── tests/                       # Unit tests
    └── tests.cpp                # 36 GoogleTest test cases covering all 15 rules, suppressions, formats
```

---

## 2. File-by-File Detailed Explanation

Below is an in-depth description of every single file in this repository.

---

### Root Configuration Files

#### 1. `.gitignore`
- **What it does:** Tells Git which generated files and folders should NOT be tracked or uploaded to GitHub.
- **What it contains:** Ignores the `build/` directory (where compiler outputs live), `*.exe` executables, `*.obj` object files, `.vs/` Visual Studio caches, and `study/` private interview notes.
- **Which files it interacts with:** Used by Git.

#### 2. `CMakeLists.txt`
- **What it does:** The central build blueprint for the entire project. Works across Windows, Linux, and macOS.
- **What it contains:** 
  - Sets C++ standard to `C++20` strictly.
  - Adds `/W4 /WX /permissive-` on MSVC (warning level 4, warnings as errors).
  - Creates the static library `misra_core` from the 4 source files.
  - Creates the executable `misra_check` from `src/main.cpp`.
  - Downloads GoogleTest v1.14.0 via CMake `FetchContent`.
  - Creates the test executable `unit_tests` from `tests/tests.cpp` and registers it with CTest.
- **Which files it interacts with:** Uses all files in `src/`, `include/`, and `tests/`.

#### 3. `README.md`
- **What it does:** The front door of the repository on GitHub.
- **What it contains:** Quick project description, architecture diagram, 15 rule table, build and test commands, and CLI output examples.
- **Which files it interacts with:** Points users to `docs/` for deeper study.

---

### Header Files (`include/`)

#### 4. `include/Lexer.h` (33 lines)
- **What it does:** Declares the data structures for lexical scanning and comment suppression.
- **What it contains:**
  - `enum class TokenType`: Categories for tokens (`Keyword`, `Identifier`, `Number`, `StringLiteral`, `Symbol`, `EndOfFile`).
  - `struct Token`: Stores `type`, literal `text` string, and 1-based `line` number.
  - `class Lexer`: Declares `tokenize()` and `getSuppressions()`, along with private helpers `peek()`, `advance()`, `skipWhitespace()`, `scanComment()`, `scanWord()`, `scanNum()`, `scanStr()`, and `scanSym()`.
- **Which other files it uses:** Uses `<string>`, `<vector>`, `<map>`, `<set>`. Included by `include/Parser.h` and `src/Lexer.cpp`.

#### 5. `include/Parser.h` (74 lines)
- **What it does:** Declares the Abstract Syntax Tree (AST) node hierarchy and the recursive descent `Parser` class.
- **What it contains:**
  - `struct Stmt`: Base class for statements with a virtual destructor and a `line` number.
  - `struct CompoundStmt`: Represents `{ ... }` blocks; holds a vector of `std::unique_ptr<Stmt>`.
  - `struct VarDeclStmt`: Represents `int x = 0;`; holds variable name and boolean `hasInit`.
  - `struct IfStmt`: Holds condition tokens, `thenBranch`, and optional `elseBranch`.
  - `struct WhileStmt` & `struct ForStmt`: Hold condition tokens and loop `body`.
  - `struct SwitchStmt`: Holds switch `body` and boolean `hasDefault`.
  - `struct FunctionDecl`: Holds function `name`, start/end lines, `body`, and list of called function names (`calls`).
  - `struct TranslationUnit`: Root AST node containing all parsed `FunctionDecl` nodes.
  - `class Parser`: Declares `parse()` and recursive parsing helpers (`parseFunc`, `parseStmt`, `parseBlock`, `parseVar`, `parseIf`, etc.).
- **Which other files it uses:** Includes `Lexer.h` and `<memory>`. Included by `include/Rules.h` and `src/Parser.cpp`.

#### 6. `include/Reporter.h` (12 lines)
- **What it does:** Declares the presentation layer for generating output reports.
- **What it contains:**
  - `enum class OutputFormat`: Supports `Text`, `Json`, and `Sarif`.
  - `class Reporter`: Declares static methods `toText()`, `toJson()`, `toSarif()`, and `print()`.
- **Which other files it uses:** Includes `Rules.h` (for `Violation`) and `<iostream>`. Included by `src/Reporter.cpp` and `src/main.cpp`.

#### 7. `include/Rules.h` (108 lines)
- **What it does:** Declares violations, call graph cycle detection, the `Rule` base class, all 15 concrete rule classes, and the `RuleEngine`.
- **What it contains:**
  - `struct Violation`: Holds `ruleId`, warning `message`, `file` name, and `line` number.
  - `class CallGraph`: Stores adjacency map `edges_` and function line numbers `lines_`. Declares `build()` and `findCycles()`.
  - `class Rule`: Abstract base class with pure virtual `std::string id() const` and `check(...)`.
  - 15 Concrete Rule classes: `NoGotoRule`, `NoNewRule`, `NoDeleteRule`, `NoMallocFamilyRule`, `NoContinueRule`, `NoUsingNamespaceRule`, `NoBannedFuncsRule`, `NoAssignInCondRule`, `IfBracesRule`, `LoopBracesRule`, `SwitchDefaultRule`, `UninitVarRule`, `MaxNestingRule`, `MaxFuncLengthRule`, and `NoRecursionRule`.
  - `class RuleEngine`: Owns `std::vector<std::unique_ptr<Rule>>` and declares `run()`.
- **Which other files it uses:** Includes `Parser.h`, `<map>`, `<set>`. Included by `src/Rules.cpp` and `tests/tests.cpp`.

---

### Source Files (`src/`)

#### 8. `src/Lexer.cpp` (86 lines)
- **What it does:** Implements character-by-character lexical scanning and line suppression detection.
- **What it contains:**
  - `Lexer::scanComment()`: Extracts `NOLINT` or `NOLINTNEXTLINE(RULE_NAME)` directives and records line numbers.
  - `Lexer::scanWord()`: Accumulates alphanumeric text and categorizes keywords versus identifiers.
  - `Lexer::scanSym()`: Scans single-character and two-character compound symbols (`==`, `!=`, `<=`, `&&`, etc.).
  - `Lexer::tokenize()`: Main loop converting raw source code into `std::vector<Token>`.
- **Which other files it uses:** Includes `Lexer.h`, `<cctype>`, `<sstream>`, `<unordered_set>`.

#### 9. `src/Parser.cpp` (140 lines)
- **What it does:** Implements recursive descent parsing, constructing AST nodes and recording function calls.
- **What it contains:**
  - `Parser::consumeParen()` & `consumeUntil()`: Collects nested token ranges and records function call names for the call graph.
  - `Parser::parseBlock()`: Parses `{ ... }` curly-brace statement blocks.
  - `Parser::parseVar()`: Parses variable declarations and detects explicit `= ...` initializers.
  - `Parser::parseIf()`, `parseWhile()`, `parseFor()`, `parseSwitch()`: Parses control structures.
  - `Parser::parseStmt()`: Handles labels (`target:`), `case`, `default`, blocks, control structures, and variable declarations.
  - `Parser::parseFunc()`: Parses function name, parameters, body, and calculates function span (`endLine - line + 1`).
  - `Parser::parse()`: Loops over the translation unit, skips classes/structs/templates gracefully, and returns `TranslationUnit`.
- **Which other files it uses:** Includes `Parser.h`.

#### 10. `src/Rules.cpp` (147 lines)
- **What it does:** Implements CallGraph 3-color DFS cycle detection, all 15 safety checks, and the RuleEngine orchestrator.
- **What it contains:**
  - `CallGraph::findCycles()`: Implements **3-color Depth-First Search** (0=White, 1=Gray, 2=Black) to detect direct and indirect call loops.
  - `walk()` helper: Traverses the AST tree recursively without requiring a visitor pattern.
  - Implementations of all 15 `check()` methods.
  - `RuleEngine::RuleEngine()`: Instantiates and stores all 15 rule classes using `std::make_unique`.
  - `RuleEngine::run()`: Runs Lexer -> Parser -> CallGraph -> Rules -> filters suppressed lines -> sorts chronologically by line number.
- **Which other files it uses:** Includes `Rules.h`, `<algorithm>`.

#### 11. `src/Reporter.cpp` (56 lines)
- **What it does:** Serializes violation objects into human-readable or machine-readable strings.
- **What it contains:**
  - `esc()` helper: Escapes quotes, backslashes, and newlines for valid JSON/SARIF.
  - `Reporter::toText()`: Outputs `file:line: [RULE_ID] message`.
  - `Reporter::toJson()`: Outputs a JSON object with `"violations"` list and `"summary"`.
  - `Reporter::toSarif()`: Outputs valid OASIS SARIF 2.1.0 schema with URI artifact locations and start lines.
  - `Reporter::print()`: Dispatches to standard output stream `std::cout`.
- **Which other files it uses:** Includes `Reporter.h`, `<sstream>`.

#### 12. `src/main.cpp` (21 lines)
- **What it does:** The Command-Line Interface (CLI) entry point.
- **What it contains:**
  - `main(int argc, char* argv[])`:
    - Checks arguments for input file and flags (`--json`, `--sarif`).
    - Opens and reads the file into a string buffer.
    - Executes `RuleEngine::run()`.
    - Invokes `Reporter::print()`.
    - Returns exit code `0` if 0 violations, or `1` if violations were found.
- **Which other files it uses:** Includes `Reporter.h`, `<fstream>`, `<sstream>`.

---

### Tests and Samples

#### 13. `tests/tests.cpp` (84 lines)
- **What it does:** Comprehensive automated test suite powered by GoogleTest.
- **What it contains:**
  - Helper functions `check()` and `has()`.
  - 30 rule tests (1 positive test triggering the violation + 1 negative test verifying clean code for all 15 rules).
  - 1 indirect recursion test (`a()` calls `b()`, and `b()` calls `a()`).
  - 2 suppression tests (`// NOLINT` and `// NOLINTNEXTLINE`).
  - 3 reporter tests (verifying Text, JSON, and SARIF output strings).
- **Which other files it uses:** Includes `<gtest/gtest.h>`, `Rules.h`, `Reporter.h`.

#### 14. `samples/bad_code.cpp`
- **What it does:** A realistic sample C++ file triggering every single one of the 15 rules at least once. Running `misra_check` reports **20 violations**.

#### 15. `samples/good_code.cpp`
- **What it does:** A clean, safe, fully compliant C++ file following all 15 guidelines. Running `misra_check` reports **0 violations** with exit code 0.

---

## 3. "Read the Files in This Order" Learning Plan

If you want to understand this entire project from scratch, follow this exact sequence:

| Step | File to Read | What You Learn | Recommended Time |
|---|---|---|---|
| **1** | [docs/01_PROJECT_FLOW.md](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/docs/01_PROJECT_FLOW.md) | High-level understanding of the pipeline & the 15 rules | 20 mins |
| **2** | [include/Lexer.h](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/include/Lexer.h) & [src/Lexer.cpp](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/src/Lexer.cpp) | How text turns into tokens and how comments are checked | 25 mins |
| **3** | [include/Parser.h](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/include/Parser.h) & [src/Parser.cpp](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/src/Parser.cpp) | How statements become an Abstract Syntax Tree (AST) | 45 mins |
| **4** | [include/Rules.h](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/include/Rules.h) & [src/Rules.cpp](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/src/Rules.cpp) | How rules inspect AST/tokens & how 3-color DFS finds cycles | 45 mins |
| **5** | [include/Reporter.h](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/include/Reporter.h) & [src/Reporter.cpp](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/src/Reporter.cpp) | How violations turn into Text, JSON, and SARIF | 20 mins |
| **6** | [src/main.cpp](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/src/main.cpp) | How the CLI brings all parts together into a tool | 15 mins |
| **7** | [tests/tests.cpp](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/tests/tests.cpp) | How to write unit tests to verify your analyzer | 25 mins |
| **8** | [CMakeLists.txt](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/CMakeLists.txt) & [docs/07_CMAKE_GUIDE.md](file:///f:/main%20servers%20under%20productions/MISRAAUTOSAR/docs/07_CMAKE_GUIDE.md) | How the build system compiles and tests the code | 30 mins |
| **Total**| **Complete Mastery** | **From zero to explaining the tool in an interview** | **~3.5 hours** |
