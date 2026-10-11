#include "Parser.h"

Parser::Parser(std::vector<Token> tokens) : toks_(std::move(tokens)) {}
const Token& Parser::peek() const { return pos_ < toks_.size() ? toks_[pos_] : toks_.back(); }
const Token& Parser::advance() { return pos_ < toks_.size() ? toks_[pos_++] : toks_.back(); }

bool Parser::match(TokenType t, const std::string& txt) {
    if (pos_ < toks_.size() && toks_[pos_].type == t && (txt.empty() || toks_[pos_].text == txt)) {
        advance(); return true;
    }
    return false;
}

std::vector<Token> Parser::consumeParen() {
    std::vector<Token> res;
    if (!match(TokenType::Symbol, "(")) return res;
    int depth = 1;
    while (pos_ < toks_.size() && depth > 0) {
        if (peek().text == "(") depth++;
        if (peek().text == ")") depth--;
        if (depth == 0) { advance(); break; }
        res.push_back(advance());
    }
    if (curCalls_) for (size_t i = 0; i + 1 < res.size(); ++i)
        if (res[i].type == TokenType::Identifier && res[i + 1].text == "(") curCalls_->push_back(res[i].text);
    return res;
}

std::vector<Token> Parser::consumeUntil(const std::string& term) {
    std::vector<Token> res;
    while (pos_ < toks_.size() && peek().text != term) res.push_back(advance());
    match(TokenType::Symbol, term);
    if (curCalls_) for (size_t i = 0; i + 1 < res.size(); ++i)
        if (res[i].type == TokenType::Identifier && res[i + 1].text == "(") curCalls_->push_back(res[i].text);
    return res;
}

std::unique_ptr<CompoundStmt> Parser::parseBlock() {
    auto b = std::make_unique<CompoundStmt>();
    b->line = peek().line;
    if (!match(TokenType::Symbol, "{")) return nullptr;
    while (pos_ < toks_.size() && peek().text != "}") {
        auto s = parseStmt();
        if (s) b->stmts.push_back(std::move(s));
    }
    match(TokenType::Symbol, "}");
    return b;
}

std::unique_ptr<VarDeclStmt> Parser::parseVar() {
    auto v = std::make_unique<VarDeclStmt>();
    v->line = peek().line;
    while (pos_ < toks_.size() && peek().text != "=" && peek().text != ";") {
        if (peek().type == TokenType::Identifier) v->name = peek().text;
        advance();
    }
    if (match(TokenType::Symbol, "=")) { v->hasInit = true; consumeUntil(";"); }
    else match(TokenType::Symbol, ";");
    return v;
}

std::unique_ptr<IfStmt> Parser::parseIf() {
    auto s = std::make_unique<IfStmt>();
    s->line = advance().line;
    s->cond = consumeParen();
    s->thenBranch = parseStmt();
    if (match(TokenType::Keyword, "else")) s->elseBranch = parseStmt();
    return s;
}

std::unique_ptr<WhileStmt> Parser::parseWhile() {
    auto s = std::make_unique<WhileStmt>();
    s->line = advance().line;
    s->cond = consumeParen();
    s->body = parseStmt();
    return s;
}

std::unique_ptr<ForStmt> Parser::parseFor() {
    auto s = std::make_unique<ForStmt>();
    s->line = advance().line;
    s->cond = consumeParen();
    s->body = parseStmt();
    return s;
}

std::unique_ptr<SwitchStmt> Parser::parseSwitch() {
    auto s = std::make_unique<SwitchStmt>();
    s->line = advance().line;
    consumeParen();
    size_t start = pos_;
    s->body = parseBlock();
    for (size_t i = start; i < pos_; ++i) if (toks_[i].text == "default") s->hasDefault = true;
    return s;
}

std::unique_ptr<Stmt> Parser::parseStmt() {
    if (pos_ >= toks_.size() || peek().type == TokenType::EndOfFile) return nullptr;
    if (peek().text == "case" || peek().text == "default") { consumeUntil(":"); return parseStmt(); }
    if (peek().type == TokenType::Identifier && pos_ + 1 < toks_.size() && toks_[pos_ + 1].text == ":") {
        advance(); advance(); return parseStmt();
    }
    if (peek().text == "{") return parseBlock();
    if (peek().text == "if") return parseIf();
    if (peek().text == "while") return parseWhile();
    if (peek().text == "for") return parseFor();
    if (peek().text == "switch") return parseSwitch();
    if (peek().text == "int" || peek().text == "char" || peek().text == "void" ||
        peek().text == "float" || peek().text == "double" || peek().text == "bool") return parseVar();
    auto s = std::make_unique<Stmt>();
    s->line = peek().line;
    consumeUntil(";");
    return s;
}

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

std::unique_ptr<TranslationUnit> Parser::parse() {
    auto tu = std::make_unique<TranslationUnit>();
    while (pos_ < toks_.size() && peek().type != TokenType::EndOfFile) {
        if (peek().text == "class" || peek().text == "struct" || peek().text == "template" || peek().text == "using") {
            consumeUntil(";"); continue;
        }
        size_t look = pos_;
        bool foundFunc = false;
        while (look < toks_.size() && toks_[look].text != ";" && toks_[look].text != "{") {
            if (toks_[look].type == TokenType::Identifier && look + 1 < toks_.size() && toks_[look + 1].text == "(") {
                tu->funcs.push_back(parseFunc());
                foundFunc = true;
                break;
            }
            look++;
        }
        if (!foundFunc) parseStmt();
    }
    return tu;
}
