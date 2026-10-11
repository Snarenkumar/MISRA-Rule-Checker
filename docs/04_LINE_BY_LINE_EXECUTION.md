# Line-by-Line Execution & Code Deep Dive

This document takes you on a complete journey through `misra_check`. 
First, we will trace a tiny 8-line C++ file through every single stage of the tool. 
Second, we will quote the real C++ code of each component and explain each line in plain English. 
Third, we provide a complete C++ glossary and detailed tutorials on **Recursive Descent Parsing** and **3-Color DFS Cycle Detection**.

---

## 1. Tracing a Tiny Sample Program Step by Step

Let us analyze this tiny 8-line C++ input file named `mini_test.cpp`:

```cpp
// mini_test.cpp
void ping() {
    ping();
    goto jump;
jump: ;
    switch (1) {
        case 1: break;
    }
}
```

Notice what is inside this function:
1. `ping()` calls `ping()` directly (Recursion!).
2. A `goto jump;` jump statement.
3. A `switch (1)` statement missing a `default:` label.

Let us trace what `misra_check` does with this file step by step.

---

### Step A: The Lexer Stage (Tokens)

The Lexer reads the raw text character-by-character and groups them into 23 tokens:

| Token # | Type | Text | Line |
|---|---|---|---|
| 1 | `TokenType::Keyword` | `"void"` | Line 2 |
| 2 | `TokenType::Identifier`| `"ping"` | Line 2 |
| 3 | `TokenType::Symbol` | `"("` | Line 2 |
| 4 | `TokenType::Symbol` | `")"` | Line 2 |
| 5 | `TokenType::Symbol` | `"{"` | Line 2 |
| 6 | `TokenType::Identifier`| `"ping"` | Line 3 |
| 7 | `TokenType::Symbol` | `"("` | Line 3 |
| 8 | `TokenType::Symbol` | `")"` | Line 3 |
| 9 | `TokenType::Symbol` | `";"` | Line 3 |
| 10 | `TokenType::Keyword` | `"goto"` | Line 4 |
| 11 | `TokenType::Identifier`| `"jump"` | Line 4 |
| 12 | `TokenType::Symbol` | `";"` | Line 4 |
| 13 | `TokenType::Identifier`| `"jump"` | Line 5 |
| 14 | `TokenType::Symbol` | `":"` | Line 5 |
| 15 | `TokenType::Symbol` | `";"` | Line 5 |
| 16 | `TokenType::Keyword` | `"switch"` | Line 6 |
| 17 | `TokenType::Symbol` | `"("` | Line 6 |
| 18 | `TokenType::Number` | `"1"` | Line 6 |
| 19 | `TokenType::Symbol` | `")"` | Line 6 |
| 20 | `TokenType::Symbol` | `"{"` | Line 6 |
| 21 | `TokenType::Keyword` | `"case"` | Line 7 |
| 22 | `TokenType::Number` | `"1"` | Line 7 |
| 23 | `TokenType::Symbol` | `":"` | Line 7 |
| 24 | `TokenType::Keyword` | `"break"` | Line 7 |
| 25 | `TokenType::Symbol` | `";"` | Line 7 |
| 26 | `TokenType::Symbol` | `"}"` | Line 8 |
| 27 | `TokenType::Symbol` | `"}"` | Line 9 |
| 28 | `TokenType::EndOfFile` | `""` | Line 9 |

Notice: The comment `// mini_test.cpp` on Line 1 was scanned by `scanComment()` and discarded because it contained no `NOLINT`.

---

### Step B: The Parser Stage (Abstract Syntax Tree)

The Parser processes the tokens using recursive descent. It recognizes `void ping()` as a `FunctionDecl` and parses its body block `{ ... }`:

```
TranslationUnit
  └── FunctionDecl: name = "ping", line = 2, endLine = 9, calls = ["ping"]
        └── CompoundStmt (Function Body)
              ├── Stmt (expression statement: ping();)
              ├── Stmt (goto statement: goto jump;)
              ├── Stmt (label: jump: ;)
              └── SwitchStmt (line = 6, hasDefault = false)
                    └── CompoundStmt (Switch Body)
                          └── Stmt (case 1: break;)
```

Notice: While parsing `ping();`, the Parser saw an Identifier `"ping"` immediately followed by `"("`. It automatically added `"ping"` to the function's list of outgoing calls: `f->calls.push_back("ping")`.

---

### Step C: The Call Graph & 3-Color DFS Stage

1. **Call Graph Construction:**
   - Function registered: `"ping"` at Line 2.
   - Directed edge added: `"ping"` $\rightarrow$ `"ping"`.

2. **3-Color Depth-First Search Cycle Detection:**
   - Start DFS with all functions colored **White (0)**.
   - **Step 1:** Visit function `"ping"`.
     - Set `color["ping"] = 1` (**Gray** - currently exploring).
     - Push `"ping"` onto traversal stack: `stack = ["ping"]`.
   - **Step 2:** Look at all functions called by `"ping"`.
     - Target is `"ping"`.
     - Check `color["ping"]`: It is already **Gray (1)**!
     - **CYCLE FOUND!** A path from a Gray node back to itself means recursion!
     - Construct cycle string: `"ping -> ping"` at Line 2.
   - **Step 3:** Finish visiting `"ping"`.
     - Pop `"ping"` from stack.
     - Set `color["ping"] = 2` (**Black** - finished).

---

### Step D: The Rules Stage (Checking)

The `RuleEngine` runs all 15 rules on the data:

1. `NoGotoRule::check`:
   - Scans the token stream.
   - Sees Token 10: `text == "goto"` on Line 4.
   - **FIRES:** Adds violation `[NO_GOTO]` at Line 4.
2. `SwitchDefaultRule::check`:
   - Walks the AST.
   - Finds `SwitchStmt` on Line 6 with `hasDefault == false`.
   - **FIRES:** Adds violation `[SWITCH_DEFAULT]` at Line 6.
3. `NoRecursionRule::check`:
   - Queries `CallGraph::findCycles()`.
   - Receives cycle `"ping -> ping"` at Line 2.
   - **FIRES:** Adds violation `[NO_RECURSION]` at Line 2.
4. All other 12 rules find no violations.

---

### Step E: The Reporter Stage (Output)

The `RuleEngine` sorts the 3 violations by line number (Line 2, Line 4, Line 6) and passes them to `Reporter::toText()`.

**Final Output Printed to Terminal:**
```
mini_test.cpp:2: [NO_RECURSION] Recursion detected: ping -> ping
mini_test.cpp:4: [NO_GOTO] Use of 'goto' is prohibited.
mini_test.cpp:6: [SWITCH_DEFAULT] 'switch' missing 'default:' label.
Found 3 violations.
```

---

## 2. Line-by-Line Code Walkthrough of the Real Implementation

Now let us look at the actual C++ code from this repository and explain every single line.

---

### Part 1: The Lexer (`src/Lexer.cpp`)

```cpp
Lexer::Lexer(std::string src) : src_(std::move(src)) {}
```
- **Explanation:** Constructor for the `Lexer`. It takes the input source text string `src` and moves it efficiently into the private member variable `src_` without copying.

```cpp
char Lexer::peek() const { return pos_ < src_.size() ? src_[pos_] : '\0'; }
```
- **Explanation:** Looks at the character currently under the scanner cursor (`pos_`) without consuming it. If we reached the end of the text, it safely returns the null terminator character `'\0'`.

```cpp
char Lexer::advance() { char c = src_[pos_++]; if (c == '\n') line_++; return c; }
```
- **Explanation:** Consumes the current character and moves `pos_` forward by 1. If the character was a newline `\n`, it increments our 1-based source line counter `line_`.

```cpp
void Lexer::scanComment() {
    int curLine = line_;
    if (peek() == '/') {
        std::string txt;
        while (pos_ < src_.size() && peek() != '\n') txt += advance();
```
- **Explanation:** When a single-line comment `//` is encountered, it remembers the current line number and loops until the end of the line, gathering the comment text.

```cpp
        size_t np = txt.find("NOLINT");
        if (np != std::string::npos) {
            int tgt = (txt.find("NOLINTNEXTLINE") != std::string::npos) ? curLine + 1 : curLine;
            size_t o = txt.find('(', np), c = txt.find(')', o);
            if (o != std::string::npos && c != std::string::npos) {
                std::stringstream ss(txt.substr(o + 1, c - o - 1));
                std::string id;
                while (std::getline(ss, id, ',')) {
                    size_t s = id.find_first_not_of(" "), e = id.find_last_not_of(" ");
                    if (s != std::string::npos) suppressions_[tgt].insert(id.substr(s, e - s + 1));
                }
            } else suppressions_[tgt].insert("*");
        }
```
- **Explanation:** Checks if the word `"NOLINT"` is present in the comment. If it says `"NOLINTNEXTLINE"`, the target line is `curLine + 1`; otherwise it is `curLine`. If parentheses exist like `(NO_GOTO, NO_NEW)`, it splits the comma-separated rule names, trims spaces, and inserts them into our suppression map `suppressions_`. If no parentheses exist, it inserts `"*"` to silence all rules on that line.

---

### Part 2: The Parser (`src/Parser.cpp`)

```cpp
std::unique_ptr<SwitchStmt> Parser::parseSwitch() {
    auto s = std::make_unique<SwitchStmt>();
    s->line = advance().line;
    consumeParen();
    size_t start = pos_;
    s->body = parseBlock();
    for (size_t i = start; i < pos_; ++i) if (toks_[i].text == "default") s->hasDefault = true;
    return s;
}
```
- **Line 1:** Declares the function `parseSwitch()` returning a smart pointer `std::unique_ptr<SwitchStmt>`.
- **Line 2:** Dynamically allocates a new `SwitchStmt` object managed by a unique pointer.
- **Line 3:** Consumes the `"switch"` token and records its source line number.
- **Line 4:** Consumes the condition in parentheses `(expression)`.
- **Line 5:** Records the token position where the switch block `{` begins.
- **Line 6:** Calls `parseBlock()` to parse all statements inside the curly braces.
- **Line 7:** Loops through all tokens that belonged to this switch block. If any token has `text == "default"`, it marks `hasDefault = true`.
- **Line 8:** Returns the completed switch statement node.

```cpp
std::unique_ptr<FunctionDecl> Parser::parseFunc() {
    auto f = std::make_unique<FunctionDecl>();
    f->line = peek().line;
    while (pos_ < toks_.size() && peek().text != "(") {
        if (peek().type == TokenType::Identifier) f->name = peek().text;
        advance();
    }
    consumeParen();
    if (match(TokenType::Symbol, ";")) return f;
    if (peek().text == "{") {
        curCalls_ = &f->calls;
        f->body = parseBlock();
        curCalls_ = nullptr;
    }
    f->endLine = pos_ > 0 ? toks_[pos_ - 1].line : f->line;
    return f;
}
```
- **Explanation:** Parses a C++ function. It extracts the return type and identifier name before the opening parenthesis `(`. It consumes the parameter list `consumeParen()`. If followed by `;`, it was only a forward declaration. If followed by `{`, it points `curCalls_` to `f->calls` so any function calls parsed inside the body are recorded. Finally, it calculates `endLine` from the closing brace token so `MAX_FUNC_LENGTH` can measure function size.

---

### Part 3: The Call Graph & 3-Color DFS (`src/Rules.cpp`)

```cpp
void CallGraph::build(const TranslationUnit& tu) {
    for (const auto& f : tu.funcs) {
        lines_[f->name] = f->line;
        for (const auto& c : f->calls) edges_[f->name].insert(c);
    }
}
```
- **Explanation:** Iterates through every parsed function in the translation unit. For each function `f`, it stores its line number in `lines_[f->name]` and inserts every callee `c` into the adjacency set `edges_[f->name]`.

```cpp
std::vector<std::pair<std::string, int>> CallGraph::findCycles() const {
    std::vector<std::pair<std::string, int>> res;
    std::map<std::string, int> col; // 0=White, 1=Gray, 2=Black
    std::vector<std::string> st;
    auto dfs = [&](auto& self, const std::string& u) -> void {
        col[u] = 1; st.push_back(u);
        if (edges_.count(u)) for (const auto& v : edges_.at(u)) {
            if (!lines_.count(v)) continue;
            if (col[v] == 1) {
                std::string p;
                for (const auto& n : st) if (n == v || !p.empty()) p += (p.empty() ? "" : " -> ") + n;
                res.push_back({p + " -> " + v, lines_.at(v)});
            } else if (col[v] == 0) self(self, v);
        }
        st.pop_back(); col[u] = 2;
    };
    for (const auto& p : lines_) if (!col[p.first]) dfs(dfs, p.first);
    return res;
}
```
- **Line 2:** Creates the result vector holding pairs of cycle descriptions and line numbers.
- **Line 3:** Creates the color map where `0 = White` (unvisited), `1 = Gray` (currently exploring), and `2 = Black` (finished).
- **Line 4:** Creates a vector `st` acting as our traversal call stack.
- **Line 5:** Declares a recursive C++ lambda function `dfs` capturing variables by reference.
- **Line 6:** Marks the current function `u` as Gray (`col[u] = 1`) and pushes it onto `st`.
- **Line 7:** If `u` calls other functions, loops over each callee `v`.
- **Line 8:** If callee `v` is a standard library function not defined in our file, ignores it.
- **Line 9:** If callee `v` is already Gray (`col[v] == 1`), a cycle is detected!
- **Line 10-12:** Formats the cycle chain (e.g., `"funcA -> funcB -> funcA"`) and adds it to `res` with the source line of function `v`.
- **Line 13:** If `v` is White (`col[v] == 0`), recursively visits `v`.
- **Line 15:** Backtracks: pops `u` from stack and marks it as Black (`col[u] = 2`).
- **Line 17:** Loops over all declared functions; if unvisited, starts a DFS from that function.
- **Line 18:** Returns all detected recursion cycles.

---

### Part 4: Token Rule Example (`NoGotoRule`) & AST Rule Example (`IfBracesRule`)

#### `NoGotoRule::check` (Token Rule)
```cpp
void NoGotoRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& t : toks) if (t.text == "goto") v.push_back({id(), "Use of 'goto' is prohibited.", f, t.line});
}
```
- **Explanation:** Iterates through the flat token list. If any token's text is `"goto"`, it instantly records a new `Violation` with rule ID `"NO_GOTO"`, the file name `f`, and the token's source `line`.

#### `IfBracesRule::check` (AST Rule)
```cpp
void IfBracesRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) walk(fn->body.get(), [&](const Stmt* s) {
        if (auto* i = dynamic_cast<const IfStmt*>(s)) {
            if (i->thenBranch && !dynamic_cast<const CompoundStmt*>(i->thenBranch.get())) v.push_back({id(), "'if' body must use braces.", f, i->thenBranch->line});
            if (i->elseBranch && !dynamic_cast<const CompoundStmt*>(i->elseBranch.get()) && !dynamic_cast<const IfStmt*>(i->elseBranch.get()))
                v.push_back({id(), "'else' body must use braces.", f, i->elseBranch->line});
        }
    });
}
```
- **Explanation:** Uses `walk()` to traverse the statement tree of every function. When it encounters an `IfStmt`:
  - It checks if `thenBranch` exists and whether it is a `CompoundStmt` (`{ ... }`). If not, it adds a violation because the `if` body is missing braces!
  - It does the same check for `elseBranch` (while allowing `else if` chaining).

---

### Part 5: The Reporter (`src/Reporter.cpp`)

```cpp
std::string Reporter::toText(const std::vector<Violation>& viols) {
    std::ostringstream ss;
    for (const auto& v : viols) {
        ss << v.file << ":" << v.line << ": [" << v.ruleId << "] " << v.message << "\n";
    }
    ss << "Found " << viols.size() << " violation" << (viols.size() == 1 ? "" : "s") << ".\n";
    return ss.str();
}
```
- **Explanation:** Formats the list of violations into a clean string stream matching the standard compiler diagnostic format (`filename:line: [RULE] message`), finishes with a summary count, and returns the result.

---

## 3. C++ Language Features Glossary

Here is every modern C++ feature used in this project, explained simply:

| C++ Feature | What It Is | Tiny Example | Why Used in This Project |
|---|---|---|---|
| `enum class` | Strongly typed enumeration. | `enum class Color { Red, Blue };` | Prevents naming clashes between token types. |
| `struct` vs `class` | Data bundle with public members by default. | `struct Token { int line; };` | Used for plain AST nodes and data holders without boilerplate getters. |
| `virtual` | Allows derived classes to override methods. | `virtual void check(...) = 0;` | Defines the pure virtual contract in the `Rule` base class. |
| `override` | Verifies a method really overrides a base method. | `void check(...) override;` | Catches signature mistakes at compile time. |
| `std::unique_ptr` | Smart pointer that owns and cleans up memory. | `auto p = std::make_unique<IfStmt>();` | Prevents memory leaks in our AST without needing manual `delete`. |
| `std::move` | Transfers ownership without copying. | `toks_(std::move(tokens))` | Fast, efficient passing of token vectors. |
| `const&` | Reference to data without copying, read-only. | `void build(const TranslationUnit& tu);` | Fast function calls that avoid expensive deep copying. |
| `std::vector` | Dynamic resizable array. | `std::vector<Token> tokens;` | Stores our ordered token stream and statement lists. |
| `std::map` | Sorted key-value dictionary. | `std::map<int, std::set<std::string>> supps;` | Stores `// NOLINT` line lookups and call graph colors. |
| `std::set` | Sorted collection of unique elements. | `std::set<std::string> calledFuncs;` | Ensures each function call edge is only stored once. |
| `dynamic_cast` | Safely checks runtime type of a base pointer. | `if (auto* i = dynamic_cast<const IfStmt*>(s))` | Identifies what type of statement an AST node is during tree walking. |
| `auto` | Lets the compiler deduce the variable type. | `auto tu = parser.parse();` | Keeps code readable without typing giant pointer types. |
| Lambda `[&]` | Anonymous inline function. | `auto dfs = [&](auto& self) { ... };` | Lets us write DFS and AST tree-walking algorithms compactly. |
| `static_cast` | Safe, explicit type conversion. | `static_cast<unsigned char>(c)` | Prevents negative character values from crashing `std::isspace`. |
