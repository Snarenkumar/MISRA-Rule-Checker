#include "Lexer.h"
#include <cctype>
#include <sstream>
#include <unordered_set>

Lexer::Lexer(std::string src) : src_(std::move(src)) {}
char Lexer::peek() const { return pos_ < src_.size() ? src_[pos_] : '\0'; }
char Lexer::advance() { char c = src_[pos_++]; if (c == '\n') line_++; return c; }
void Lexer::skipWhitespace() { while (pos_ < src_.size() && std::isspace(static_cast<unsigned char>(peek()))) advance(); }

void Lexer::scanComment() {
    int curLine = line_;
    if (peek() == '/') {
        std::string txt;
        while (pos_ < src_.size() && peek() != '\n') txt += advance();
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
    } else {
        advance();
        while (pos_ < src_.size()) if (advance() == '*' && peek() == '/') { advance(); break; }
    }
}

Token Lexer::scanWord() {
    int startLine = line_;
    std::string text;
    while (pos_ < src_.size() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) text += advance();
    static const std::unordered_set<std::string> kw = {
        "if", "else", "while", "for", "switch", "case", "default", "goto",
        "return", "break", "continue", "new", "delete", "using", "namespace",
        "int", "void", "char", "float", "double", "bool", "class", "struct", "template"
    };
    return Token{kw.count(text) ? TokenType::Keyword : TokenType::Identifier, std::move(text), startLine};
}

Token Lexer::scanNum() {
    int startLine = line_;
    std::string text;
    while (pos_ < src_.size() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '.')) text += advance();
    return Token{TokenType::Number, std::move(text), startLine};
}

Token Lexer::scanStr(char q) {
    int startLine = line_;
    std::string text;
    text += advance();
    while (pos_ < src_.size() && peek() != q) {
        if (peek() == '\\') text += advance();
        if (pos_ < src_.size()) text += advance();
    }
    if (pos_ < src_.size()) text += advance();
    return Token{TokenType::StringLiteral, std::move(text), startLine};
}

Token Lexer::scanSym() {
    int startLine = line_;
    std::string text(1, advance());
    char next = peek();
    if ((text == "=" && next == '=') || (text == "!" && next == '=') ||
        (text == "<" && next == '=') || (text == ">" && next == '=') ||
        (text == "&" && next == '&') || (text == "|" && next == '|') ||
        (text == "+" && next == '+') || (text == "-" && next == '-')) text += advance();
    return Token{TokenType::Symbol, std::move(text), startLine};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos_ < src_.size()) {
        skipWhitespace();
        if (pos_ >= src_.size()) break;
        char c = peek();
        if (c == '/' && (pos_ + 1 < src_.size()) && (src_[pos_ + 1] == '/' || src_[pos_ + 1] == '*')) {
            advance(); scanComment();
        } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') tokens.push_back(scanWord());
        else if (std::isdigit(static_cast<unsigned char>(c))) tokens.push_back(scanNum());
        else if (c == '"' || c == '\'') tokens.push_back(scanStr(c));
        else tokens.push_back(scanSym());
    }
    tokens.push_back(Token{TokenType::EndOfFile, "", line_});
    return tokens;
}

const std::map<int, std::set<std::string>>& Lexer::getSuppressions() const { return suppressions_; }
