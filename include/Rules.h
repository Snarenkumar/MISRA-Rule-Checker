#pragma once
#include "Parser.h"
#include <map>
#include <set>

struct Violation {
    std::string ruleId;
    std::string message;
    std::string file;
    int line{1};
};

class CallGraph {
public:
    void build(const TranslationUnit& tu);
    std::vector<std::pair<std::string, int>> findCycles() const;
private:
    std::map<std::string, std::set<std::string>> edges_;
    std::map<std::string, int> lines_;
};

class Rule {
public:
    virtual ~Rule() = default;
    virtual std::string id() const = 0;
    virtual void check(const std::string& f, const std::vector<Token>& t,
                       const TranslationUnit& u, const CallGraph& g,
                       std::vector<Violation>& v) = 0;
};

class NoGotoRule : public Rule {
public:
    std::string id() const override { return "NO_GOTO"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoNewRule : public Rule {
public:
    std::string id() const override { return "NO_NEW"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoDeleteRule : public Rule {
public:
    std::string id() const override { return "NO_DELETE"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoMallocFamilyRule : public Rule {
public:
    std::string id() const override { return "NO_MALLOC_FAMILY"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoContinueRule : public Rule {
public:
    std::string id() const override { return "NO_CONTINUE"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoUsingNamespaceRule : public Rule {
public:
    std::string id() const override { return "NO_USING_NAMESPACE"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoBannedFuncsRule : public Rule {
public:
    std::string id() const override { return "NO_BANNED_FUNCS"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoAssignInCondRule : public Rule {
public:
    std::string id() const override { return "NO_ASSIGN_IN_COND"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class IfBracesRule : public Rule {
public:
    std::string id() const override { return "IF_BRACES"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class LoopBracesRule : public Rule {
public:
    std::string id() const override { return "LOOP_BRACES"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class SwitchDefaultRule : public Rule {
public:
    std::string id() const override { return "SWITCH_DEFAULT"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class UninitVarRule : public Rule {
public:
    std::string id() const override { return "UNINIT_VAR"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class MaxNestingRule : public Rule {
public:
    std::string id() const override { return "MAX_NESTING"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class MaxFuncLengthRule : public Rule {
public:
    std::string id() const override { return "MAX_FUNC_LENGTH"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};
class NoRecursionRule : public Rule {
public:
    std::string id() const override { return "NO_RECURSION"; }
    void check(const std::string& f, const std::vector<Token>& t, const TranslationUnit& u, const CallGraph& g, std::vector<Violation>& v) override;
};

class RuleEngine {
public:
    RuleEngine();
    std::vector<Violation> run(const std::string& file, const std::string& src);
private:
    std::vector<std::unique_ptr<Rule>> rules_;
};
