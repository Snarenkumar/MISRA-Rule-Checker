#include <gtest/gtest.h>
#include "Rules.h"
#include "Reporter.h"

static std::vector<Violation> check(const std::string& code) {
    RuleEngine engine;
    return engine.run("test.cpp", code);
}

static bool has(const std::vector<Violation>& v, const std::string& rule) {
    for (const auto& vi : v) if (vi.ruleId == rule) return true;
    return false;
}

// 1. NO_GOTO
TEST(RulesTest, NoGotoPositive) { EXPECT_TRUE(has(check("void f() { goto lbl; lbl: return; }"), "NO_GOTO")); }
TEST(RulesTest, NoGotoNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; }"), "NO_GOTO")); }

// 2. NO_NEW
TEST(RulesTest, NoNewPositive) { EXPECT_TRUE(has(check("void f() { int* p = new int; }"), "NO_NEW")); }
TEST(RulesTest, NoNewNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; }"), "NO_NEW")); }

// 3. NO_DELETE
TEST(RulesTest, NoDeletePositive) { EXPECT_TRUE(has(check("void f() { int* p = nullptr; delete p; }"), "NO_DELETE")); }
TEST(RulesTest, NoDeleteNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; }"), "NO_DELETE")); }

// 4. NO_MALLOC_FAMILY
TEST(RulesTest, NoMallocPositive) { EXPECT_TRUE(has(check("void f() { void* p = malloc(10); }"), "NO_MALLOC_FAMILY")); }
TEST(RulesTest, NoMallocNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; }"), "NO_MALLOC_FAMILY")); }

// 5. NO_CONTINUE
TEST(RulesTest, NoContinuePositive) { EXPECT_TRUE(has(check("void f() { while (1) { continue; } }"), "NO_CONTINUE")); }
TEST(RulesTest, NoContinueNegative) { EXPECT_FALSE(has(check("void f() { while (1) { break; } }"), "NO_CONTINUE")); }

// 6. NO_USING_NAMESPACE
TEST(RulesTest, NoUsingNamespacePositive) { EXPECT_TRUE(has(check("using namespace std; void f() {}"), "NO_USING_NAMESPACE")); }
TEST(RulesTest, NoUsingNamespaceNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; }"), "NO_USING_NAMESPACE")); }

// 7. NO_BANNED_FUNCS
TEST(RulesTest, NoBannedFuncsPositive) { EXPECT_TRUE(has(check("void f() { gets(); }"), "NO_BANNED_FUNCS")); }
TEST(RulesTest, NoBannedFuncsNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; }"), "NO_BANNED_FUNCS")); }

// 8. NO_ASSIGN_IN_COND
TEST(RulesTest, NoAssignInCondPositive) { EXPECT_TRUE(has(check("void f() { int x = 0; if (x = 1) {} }"), "NO_ASSIGN_IN_COND")); }
TEST(RulesTest, NoAssignInCondNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; if (x == 1) {} }"), "NO_ASSIGN_IN_COND")); }

// 9. IF_BRACES
TEST(RulesTest, IfBracesPositive) { EXPECT_TRUE(has(check("void f() { if (1) return; }"), "IF_BRACES")); }
TEST(RulesTest, IfBracesNegative) { EXPECT_FALSE(has(check("void f() { if (1) { return; } }"), "IF_BRACES")); }

// 10. LOOP_BRACES
TEST(RulesTest, LoopBracesPositive) { EXPECT_TRUE(has(check("void f() { while (1) return; }"), "LOOP_BRACES")); }
TEST(RulesTest, LoopBracesNegative) { EXPECT_FALSE(has(check("void f() { while (1) { return; } }"), "LOOP_BRACES")); }

// 11. SWITCH_DEFAULT
TEST(RulesTest, SwitchDefaultPositive) { EXPECT_TRUE(has(check("void f() { switch (1) { case 1: break; } }"), "SWITCH_DEFAULT")); }
TEST(RulesTest, SwitchDefaultNegative) { EXPECT_FALSE(has(check("void f() { switch (1) { default: break; } }"), "SWITCH_DEFAULT")); }

// 12. UNINIT_VAR
TEST(RulesTest, UninitVarPositive) { EXPECT_TRUE(has(check("void f() { int x; }"), "UNINIT_VAR")); }
TEST(RulesTest, UninitVarNegative) { EXPECT_FALSE(has(check("void f() { int x = 0; }"), "UNINIT_VAR")); }

// 13. MAX_NESTING
TEST(RulesTest, MaxNestingPositive) { EXPECT_TRUE(has(check("void f() { if (1) { if (1) { if (1) { if (1) { if (1) {} } } } } }"), "MAX_NESTING")); }
TEST(RulesTest, MaxNestingNegative) { EXPECT_FALSE(has(check("void f() { if (1) { if (1) {} } }"), "MAX_NESTING")); }

// 14. MAX_FUNC_LENGTH
TEST(RulesTest, MaxFuncLengthPositive) {
    std::string longFunc = "void f() {\n";
    for (int i = 0; i < 60; ++i) longFunc += "  int x = 0;\n";
    longFunc += "}\n";
    EXPECT_TRUE(has(check(longFunc), "MAX_FUNC_LENGTH"));
}
TEST(RulesTest, MaxFuncLengthNegative) { EXPECT_FALSE(has(check("void f() {\n  int x = 0;\n}\n"), "MAX_FUNC_LENGTH")); }

// 15. NO_RECURSION
TEST(RulesTest, NoRecursionDirectPositive) { EXPECT_TRUE(has(check("void f() { f(); }"), "NO_RECURSION")); }
TEST(RulesTest, NoRecursionIndirectPositive) { EXPECT_TRUE(has(check("void a() { b(); } void b() { a(); }"), "NO_RECURSION")); }
TEST(RulesTest, NoRecursionNegative) { EXPECT_FALSE(has(check("void a() { b(); } void b() {}"), "NO_RECURSION")); }

// Extra tests: NOLINT suppression & Reporters
TEST(SuppressionTest, NoLintLine) { EXPECT_FALSE(has(check("void f() { goto lbl; // NOLINT\n lbl: return; }"), "NO_GOTO")); }
TEST(SuppressionTest, NoLintNextLine) { EXPECT_FALSE(has(check("void f() {\n // NOLINTNEXTLINE\n goto lbl; lbl: return; }"), "NO_GOTO")); }
TEST(ReporterTest, TextOutput) {
    auto v = check("void f() { goto lbl; lbl: return; }");
    EXPECT_NE(Reporter::toText(v).find("NO_GOTO"), std::string::npos);
}
TEST(ReporterTest, JsonOutput) {
    auto v = check("void f() { goto lbl; lbl: return; }");
    EXPECT_NE(Reporter::toJson(v).find("\"rule\": \"NO_GOTO\""), std::string::npos);
}
TEST(ReporterTest, SarifOutput) {
    auto v = check("void f() { goto lbl; lbl: return; }");
    EXPECT_NE(Reporter::toSarif(v).find("\"ruleId\": \"NO_GOTO\""), std::string::npos);
}
