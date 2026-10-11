#pragma once
#include "Lexer.h"
#include <memory>
#include <string>
#include <vector>

// Base class for statements
struct Stmt {
    int line{1};
    virtual ~Stmt() = default;
};

// Block statement { ... }
struct CompoundStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
};

// Variable declaration: int x; or int x = 0;
struct VarDeclStmt : public Stmt {
    std::string name;
    bool hasInit{false};
};

// If statement: if (cond) thenBranch else elseBranch
struct IfStmt : public Stmt {
    std::vector<Token> cond;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
};

// Loop statements: while / for
struct WhileStmt : public Stmt {
    std::vector<Token> cond;
    std::unique_ptr<Stmt> body;
};

struct ForStmt : public Stmt {
    std::vector<Token> cond;
    std::unique_ptr<Stmt> body;
};

// Switch statement with default tracking
struct SwitchStmt : public Stmt {
    std::unique_ptr<Stmt> body;
    bool hasDefault{false};
};

// Free function declaration or definition
struct FunctionDecl {
    std::string name;
    int line{1};
    int endLine{1};
    std::unique_ptr<CompoundStmt> body;
    std::vector<std::string> calls;
};

// Root AST containing all functions
struct TranslationUnit {
    std::vector<std::unique_ptr<FunctionDecl>> funcs;
};

// Hand-written recursive descent parser
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<TranslationUnit> parse();

private:
    const Token& peek() const;
    const Token& advance();
    bool match(TokenType t, const std::string& txt = "");
    std::vector<Token> consumeParen();
    std::vector<Token> consumeUntil(const std::string& term);

    std::unique_ptr<FunctionDecl> parseFunc();
    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<CompoundStmt> parseBlock();
    std::unique_ptr<VarDeclStmt> parseVar();
    std::unique_ptr<IfStmt> parseIf();
    std::unique_ptr<WhileStmt> parseWhile();
    std::unique_ptr<ForStmt> parseFor();
    std::unique_ptr<SwitchStmt> parseSwitch();

    std::vector<Token> toks_;
    size_t pos_{0};
    std::vector<std::string>* curCalls_{nullptr};
};
