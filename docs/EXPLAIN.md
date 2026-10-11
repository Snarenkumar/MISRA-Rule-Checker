# Complete Project Guide & Code Walkthrough

This document explains every single file in the MISRA/AUTOSAR Static Analysis Tool in simple English. It is specifically designed to help you understand the architecture and confidently explain every detail in an interview.

---

## 1. Read in This Order for Learning

To learn this project step-by-step from zero to expert, read the files in this order:

1. **`include/Lexer.h` & `src/Lexer.cpp`**  
   *Step 1*: Understand how raw C++ source text is transformed into a stream of tokens and how `// NOLINT` comments are discovered.
2. **`include/Parser.h` & `src/Parser.cpp`**  
   *Step 2*: Understand how recursive descent parsing turns tokens into an Abstract Syntax Tree (AST) representing statements and functions.
3. **`include/Rules.h` & `src/Rules.cpp`**  
   *Step 3*: Understand the `Rule` base class, the 15 specific rule implementations, the 3-color DFS Call Graph cycle detector, and the `RuleEngine` orchestrator.
4. **`include/Reporter.h` & `src/Reporter.cpp`**  
   *Step 4*: Understand how violations are rendered into Text, JSON, and industry-standard SARIF 2.1.0 format.
5. **`src/main.cpp`**  
   *Step 5*: Understand how the command-line interface processes arguments, reads the file, runs analysis, and exits with appropriate status codes.
6. **`tests/tests.cpp`**  
   *Step 6*: Inspect unit tests for positive/negative verification of every rule.
7. **`CMakeLists.txt` & `docs/CMAKE_GUIDE.md`**  
   *Step 7*: Learn how modern C++20 builds and GoogleTest dependencies are orchestrated.

---

## 2. File-by-File & Block-by-Block Explanation

---

### File 1: `include/Lexer.h` and `src/Lexer.cpp`
**What it does:** Converts raw C++ source code text into a vector of categorized tokens (words, numbers, symbols) and tracks `// NOLINT` line suppressions.

#### Key Blocks:
- **`TokenType` enum**: Classifies each token as `Keyword`, `Identifier`, `Number`, `StringLiteral`, `Symbol`, or `EndOfFile`.
- **`Token` struct**: Stores the token's type, literal string `text`, and 1-based source `line` number.
- **`Lexer` class**:
  - `peek()` / `advance()`: Inspects the current character without moving forward or consumes it and tracks line numbers on `\n`.
  - `skipWhitespace()`: Skips spaces, tabs, and carriage returns.
  - `scanComment()`: Handles single-line `//` and multi-line `/* */`. Checks single-line comments for `NOLINT` or `NOLINTNEXTLINE(RULE_NAME)` and stores line suppressions in a map.
  - `scanWord()`: Accumulates alphanumeric identifiers. Checks against a set of C++ keywords to label the token as either `TokenType::Keyword` or `TokenType::Identifier`.
  - `scanNum()` / `scanStr()` / `scanSym()`: Scans numbers, string/character literals with escape handling, and single/two-character operators (`==`, `!=`, `<=`, etc.).
  - `tokenize()`: The main loop scanning through the source string until reaching EOF.

#### C++ Keywords Used & Why:
- **`enum class`**: Scoped, strongly-typed enumeration prevents name clashes.
- **`explicit`**: Prevents accidental implicit conversion from `std::string` when constructing `Lexer`.
- **`const`**: Marks helper member functions that do not alter the Lexer state.
- **`static_cast`**: Safely casts `char` to `unsigned char` before calling `std::isspace` or `std::isalpha` (prevents undefined behavior on negative char values).

---

### File 2: `include/Parser.h` and `src/Parser.cpp`
**What it does:** Performs simplified recursive descent parsing on the token stream to produce an Abstract Syntax Tree (AST) representing functions and statements.

#### Key Blocks:
- **AST Nodes (`Stmt` hierarchy)**:
  - `Stmt`: Base class for all statements, storing the starting `line`.
  - `CompoundStmt`: Represents curly-brace code blocks `{ ... }`, holding a vector of `std::unique_ptr<Stmt>`.
  - `VarDeclStmt`: Represents variable declarations like `int x = 0;`, tracking whether an initializer `= ...` was present.
  - `IfStmt`, `WhileStmt`, `ForStmt`: Store the condition tokens and child statement bodies.
  - `SwitchStmt`: Tracks the switch body and whether a `default:` label was found.
  - `FunctionDecl`: Holds function `name`, start line, end line, body block, and list of called function names.
  - `TranslationUnit`: Root AST node containing all parsed top-level functions.
- **`Parser` class**:
  - `peek()` / `advance()` / `match()`: Token navigation and matching utilities.
  - `consumeParen()` / `consumeUntil()`: Collects nested tokens inside parentheses or until a delimiter like `;`. Records function calls (identifier followed by `(`) for call graph analysis.
  - `parseStmt()`: Dispatches parsing to `parseBlock`, `parseIf`, `parseWhile`, `parseFor`, `parseSwitch`, or `parseVar`.
  - `parseFunc()`: Parses function signature, body block, and records end-line for function length checking.
  - `parse()`: Traverses the file, gracefully skips classes/structs/templates, and returns a `std::unique_ptr<TranslationUnit>`.

#### C++ Keywords Used & Why:
- **`virtual`**: Enables polymorphic deletion through `virtual ~Stmt() = default;`.
- **`std::unique_ptr`**: Enforces strict single-ownership of AST nodes without memory leaks.
- **`std::move`**: Transfers ownership of unique pointers without copying.

---

### File 3: `include/Rules.h` and `src/Rules.cpp`
**What it does:** Implements the 15 MISRA/AUTOSAR rules, the call graph cycle detector for recursion, and the `RuleEngine` orchestrator.

#### Key Blocks:
- **`Violation` struct**: Stores `ruleId`, warning `message`, `file` name, and `line` number.
- **`CallGraph` class**:
  - `build()`: Records function definitions and outgoing call edges.
  - `findCycles()`: Implements **3-color Depth First Search (DFS)** (0=White/unvisited, 1=Gray/visiting, 2=Black/finished). If a Gray node is encountered during traversal, a cycle exists (direct or indirect recursion).
- **`Rule` Base Class**:
  - Pure virtual method `check(...)` that each rule implements.
- **The 15 Concrete Rule Classes**:
  1. `NoGotoRule`: Rejects the `goto` keyword.
  2. `NoNewRule`: Rejects the `new` operator.
  3. `NoDeleteRule`: Rejects the `delete` operator.
  4. `NoMallocFamilyRule`: Rejects `malloc`, `calloc`, `realloc`, and `free`.
  5. `NoContinueRule`: Rejects the `continue` keyword.
  6. `NoUsingNamespaceRule`: Rejects `using namespace`.
  7. `NoBannedFuncsRule`: Rejects unsafe legacy functions `gets`, `sprintf`, and `atoi`.
  8. `NoAssignInCondRule`: Flags accidental assignment `=` inside `if`, `while`, or `for` conditions.
  9. `IfBracesRule`: Requires curly braces for all `if` and `else` branches.
  10. `LoopBracesRule`: Requires curly braces for `while` and `for` bodies.
  11. `SwitchDefaultRule`: Requires `switch` blocks to contain a `default:` label.
  12. `UninitVarRule`: Flags variable declarations missing initialization.
  13. `MaxNestingRule`: Flags code blocks nested deeper than 4 levels.
  14. `MaxFuncLengthRule`: Flags functions longer than 50 lines.
  15. `NoRecursionRule`: Flags direct and indirect recursion using the CallGraph cycle list.
- **`RuleEngine` class**:
  - Registers all 15 rules in its constructor.
  - `run()`: Executes Lexer, Parser, CallGraph, runs all 15 rules, filters out suppressed violations (`NOLINT`), sorts by line number, and returns the list.

#### C++ Keywords Used & Why:
- **`override`**: Ensures the compiler verifies that `check()` correctly overrides `Rule::check()`.
- **`dynamic_cast`**: Safely inspects the concrete runtime type of AST `Stmt` pointers during tree walks.
- **`auto`**: Deduces complex iterator, lambda, and container types cleanly.

---

### File 4: `include/Reporter.h` and `src/Reporter.cpp`
**What it does:** Formats violation lists into human-readable text, structured JSON, or SARIF (Static Analysis Results Interchange Format) 2.1.0.

#### Key Blocks:
- **`toText()`**: Formats violations as `filename:line: [RULE_ID] message`.
- **`toJson()`**: Generates JSON output suitable for automated tooling.
- **`toSarif()`**: Generates standard OASIS SARIF 2.1.0 output compatible with GitHub Code Scanning, SonarQube, and VS Code extensions.
- **`print()`**: Dispatches to the chosen output format stream.

---

### File 5: `src/main.cpp`
**What it does:** CLI entry point.

#### Key Blocks:
- Parses command-line flags (`--json`, `--sarif`, `<file.cpp>`).
- Reads the target source file into a string buffer.
- Runs `RuleEngine::run()`.
- Calls `Reporter::print()`.
- Returns exit code `0` if 0 violations were found, or `1` if violations were detected.

---

### File 6: `tests/tests.cpp`
**What it does:** Provides 36 automated unit tests using GoogleTest.
- Tests 1 positive (triggers violation) and 1 negative (no violation) case for each of the 15 rules (30 tests).
- Tests `// NOLINT` and `// NOLINTNEXTLINE` line suppressions.
- Tests Text, JSON, and SARIF report generation.

---

## 3. Summary of the 15 Rules

| Rule ID | MISRA / AUTOSAR Safety Motivation | How It's Detected |
|---|---|---|
| `NO_GOTO` | Prevents unstructured jumps and spaghetti control flow | Token scan for `goto` |
| `NO_NEW` | Dynamic memory in safety-critical code causes heap fragmentation | Token scan for `new` |
| `NO_DELETE` | Manual deallocation risks double-free and use-after-free | Token scan for `delete` |
| `NO_MALLOC_FAMILY` | Unsafe C memory management lacks type safety and constructors | Token scan for `malloc`, `calloc`, `realloc`, `free` |
| `NO_CONTINUE` | Discourages premature loop cycle jumps that obscure loop invariants | Token scan for `continue` |
| `NO_USING_NAMESPACE` | Prevents namespace pollution and ambiguous symbol resolution | Token scan for `using` followed by `namespace` |
| `NO_BANNED_FUNCS` | `gets`, `sprintf`, `atoi` cause buffer overflows and unhandled errors | Token scan for banned identifiers |
| `NO_ASSIGN_IN_COND` | Prevents bug where `if (x = 1)` is typed instead of `if (x == 1)` | Token scan inside condition ranges for `=` |
| `IF_BRACES` | Prevents "dangling else" and accidental omission of multiline bodies | Checks if then/else AST node is a `CompoundStmt` |
| `LOOP_BRACES` | Enforces that all loop bodies are clearly bounded blocks | Checks if while/for body is a `CompoundStmt` |
| `SWITCH_DEFAULT` | Ensures unhandled enumeration values or unexpected inputs are caught | Inspects `SwitchStmt.hasDefault` |
| `UNINIT_VAR` | Reading uninitialized variables leads to undefined behavior | Inspects `VarDeclStmt.hasInit` |
| `MAX_NESTING` | Deep nesting (>4 levels) impairs readability and cognitive load | Recursive tree walk tracking depth |
| `MAX_FUNC_LENGTH` | Functions >50 lines violate single-responsibility principles | Measures `endLine - line + 1` of `FunctionDecl` |
| `NO_RECURSION` | Recursion risks stack overflow on embedded systems with fixed memory | 3-color DFS cycle detector on CallGraph |
