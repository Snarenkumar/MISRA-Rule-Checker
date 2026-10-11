#include "Rules.h"
#include <algorithm>

void CallGraph::build(const TranslationUnit& tu) {
    for (const auto& f : tu.funcs) {
        lines_[f->name] = f->line;
        for (const auto& c : f->calls) edges_[f->name].insert(c);
    }
}

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

static void walk(const Stmt* s, auto fn) {
    if (!s) return;
    fn(s);
    if (auto* b = dynamic_cast<const CompoundStmt*>(s)) for (const auto& in : b->stmts) walk(in.get(), fn);
    else if (auto* i = dynamic_cast<const IfStmt*>(s)) { walk(i->thenBranch.get(), fn); walk(i->elseBranch.get(), fn); }
    else if (auto* w = dynamic_cast<const WhileStmt*>(s)) walk(w->body.get(), fn);
    else if (auto* f = dynamic_cast<const ForStmt*>(s)) walk(f->body.get(), fn);
    else if (auto* sw = dynamic_cast<const SwitchStmt*>(s)) walk(sw->body.get(), fn);
}

void NoGotoRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& t : toks) if (t.text == "goto") v.push_back({id(), "Use of 'goto' is prohibited.", f, t.line});
}
void NoNewRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& t : toks) if (t.text == "new") v.push_back({id(), "Dynamic allocation with 'new' is prohibited.", f, t.line});
}
void NoDeleteRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& t : toks) if (t.text == "delete") v.push_back({id(), "Deallocation with 'delete' is prohibited.", f, t.line});
}
void NoMallocFamilyRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& t : toks) if (t.type == TokenType::Identifier && (t.text == "malloc" || t.text == "calloc" || t.text == "realloc" || t.text == "free"))
        v.push_back({id(), "Memory function '" + t.text + "' is prohibited.", f, t.line});
}
void NoContinueRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& t : toks) if (t.text == "continue") v.push_back({id(), "Use of 'continue' is prohibited.", f, t.line});
}
void NoUsingNamespaceRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (size_t i = 0; i + 1 < toks.size(); ++i) if (toks[i].text == "using" && toks[i + 1].text == "namespace")
        v.push_back({id(), "Use of 'using namespace' is prohibited.", f, toks[i].line});
}
void NoBannedFuncsRule::check(const std::string& f, const std::vector<Token>& toks, const TranslationUnit&, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& t : toks) if (t.type == TokenType::Identifier && (t.text == "gets" || t.text == "sprintf" || t.text == "atoi"))
        v.push_back({id(), "Banned function '" + t.text + "' is prohibited.", f, t.line});
}

void NoAssignInCondRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) walk(fn->body.get(), [&](const Stmt* s) {
        auto checkCond = [&](const std::vector<Token>& c) {
            for (const auto& t : c) if (t.text == "=") v.push_back({id(), "Assignment in condition is prohibited.", f, t.line});
        };
        if (auto* i = dynamic_cast<const IfStmt*>(s)) checkCond(i->cond);
        if (auto* w = dynamic_cast<const WhileStmt*>(s)) checkCond(w->cond);
        if (auto* fl = dynamic_cast<const ForStmt*>(s)) checkCond(fl->cond);
    });
}
void IfBracesRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) walk(fn->body.get(), [&](const Stmt* s) {
        if (auto* i = dynamic_cast<const IfStmt*>(s)) {
            if (i->thenBranch && !dynamic_cast<const CompoundStmt*>(i->thenBranch.get())) v.push_back({id(), "'if' body must use braces.", f, i->thenBranch->line});
            if (i->elseBranch && !dynamic_cast<const CompoundStmt*>(i->elseBranch.get()) && !dynamic_cast<const IfStmt*>(i->elseBranch.get()))
                v.push_back({id(), "'else' body must use braces.", f, i->elseBranch->line});
        }
    });
}
void LoopBracesRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) walk(fn->body.get(), [&](const Stmt* s) {
        if (auto* w = dynamic_cast<const WhileStmt*>(s)) if (w->body && !dynamic_cast<const CompoundStmt*>(w->body.get())) v.push_back({id(), "'while' body must use braces.", f, w->body->line});
        if (auto* fl = dynamic_cast<const ForStmt*>(s)) if (fl->body && !dynamic_cast<const CompoundStmt*>(fl->body.get())) v.push_back({id(), "'for' body must use braces.", f, fl->body->line});
    });
}
void SwitchDefaultRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) walk(fn->body.get(), [&](const Stmt* s) {
        if (auto* sw = dynamic_cast<const SwitchStmt*>(s)) if (!sw->hasDefault) v.push_back({id(), "'switch' missing 'default:' label.", f, sw->line});
    });
}
void UninitVarRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) walk(fn->body.get(), [&](const Stmt* s) {
        if (auto* var = dynamic_cast<const VarDeclStmt*>(s)) if (!var->hasInit) v.push_back({id(), "Variable '" + var->name + "' uninitialized.", f, var->line});
    });
}
void MaxNestingRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) {
        int depth = 0;
        auto rec = [&](auto& self, const Stmt* s) -> void {
            if (!s) return;
            bool ctrl = dynamic_cast<const IfStmt*>(s) || dynamic_cast<const WhileStmt*>(s) ||
                        dynamic_cast<const ForStmt*>(s) || dynamic_cast<const SwitchStmt*>(s);
            if (ctrl && ++depth > 4) v.push_back({id(), "Nesting depth exceeds 4.", f, s->line});
            if (auto* b = dynamic_cast<const CompoundStmt*>(s)) for (const auto& in : b->stmts) self(self, in.get());
            else if (auto* i = dynamic_cast<const IfStmt*>(s)) { self(self, i->thenBranch.get()); self(self, i->elseBranch.get()); }
            else if (auto* w = dynamic_cast<const WhileStmt*>(s)) self(self, w->body.get());
            else if (auto* fl = dynamic_cast<const ForStmt*>(s)) self(self, fl->body.get());
            else if (auto* sw = dynamic_cast<const SwitchStmt*>(s)) self(self, sw->body.get());
            if (ctrl) depth--;
        };
        rec(rec, fn->body.get());
    }
}
void MaxFuncLengthRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit& tu, const CallGraph&, std::vector<Violation>& v) {
    for (const auto& fn : tu.funcs) if (fn->endLine - fn->line + 1 > 50) v.push_back({id(), "Function length exceeds 50 lines.", f, fn->line});
}
void NoRecursionRule::check(const std::string& f, const std::vector<Token>&, const TranslationUnit&, const CallGraph& cg, std::vector<Violation>& v) {
    for (const auto& c : cg.findCycles()) v.push_back({id(), "Recursion detected: " + c.first, f, c.second});
}

RuleEngine::RuleEngine() {
    rules_.push_back(std::make_unique<NoGotoRule>()); rules_.push_back(std::make_unique<NoNewRule>());
    rules_.push_back(std::make_unique<NoDeleteRule>()); rules_.push_back(std::make_unique<NoMallocFamilyRule>());
    rules_.push_back(std::make_unique<NoContinueRule>()); rules_.push_back(std::make_unique<NoUsingNamespaceRule>());
    rules_.push_back(std::make_unique<NoBannedFuncsRule>()); rules_.push_back(std::make_unique<NoAssignInCondRule>());
    rules_.push_back(std::make_unique<IfBracesRule>()); rules_.push_back(std::make_unique<LoopBracesRule>());
    rules_.push_back(std::make_unique<SwitchDefaultRule>()); rules_.push_back(std::make_unique<UninitVarRule>());
    rules_.push_back(std::make_unique<MaxNestingRule>()); rules_.push_back(std::make_unique<MaxFuncLengthRule>());
    rules_.push_back(std::make_unique<NoRecursionRule>());
}

std::vector<Violation> RuleEngine::run(const std::string& file, const std::string& src) {
    Lexer lexer(src);
    auto toks = lexer.tokenize();
    auto supps = lexer.getSuppressions();
    auto tu = Parser(toks).parse();
    CallGraph cg; cg.build(*tu);
    std::vector<Violation> all, filtered;
    for (const auto& r : rules_) r->check(file, toks, *tu, cg, all);
    for (const auto& vi : all) {
        auto it = supps.find(vi.line);
        if (it != supps.end() && (it->second.count(vi.ruleId) || it->second.count("*"))) continue;
        filtered.push_back(vi);
    }
    std::sort(filtered.begin(), filtered.end(), [](const Violation& a, const Violation& b) {
        return a.line != b.line ? a.line < b.line : a.ruleId < b.ruleId;
    });
    return filtered;
}
