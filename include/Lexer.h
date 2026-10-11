#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>

// Token categories for C++ code scanning
enum class TokenType { Keyword, Identifier, Number, StringLiteral, Symbol, EndOfFile };

// A single lexical token with its text and source line
struct Token {
    TokenType type;
    std::string text;
    int line{1};
};

// Scanner that breaks source text into tokens and tracks NOLINT suppressions
class Lexer {
public:
    explicit Lexer(std::string src);
    std::vector<Token> tokenize();
    const std::map<int, std::set<std::string>>& getSuppressions() const;

private:
    char peek() const;
    char advance();
    void skipWhitespace();
    void scanComment();
    Token scanWord();
    Token scanNum();
    Token scanStr(char q);
    Token scanSym();

    std::string src_;
    size_t pos_{0};
    int line_{1};
    std::map<int, std::set<std::string>> suppressions_;
};
