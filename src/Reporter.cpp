#include "Reporter.h"
#include <sstream>

static std::string esc(const std::string& in) {
    std::string out;
    for (char c : in) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

std::string Reporter::toText(const std::vector<Violation>& viols) {
    std::ostringstream ss;
    for (const auto& v : viols) {
        ss << v.file << ":" << v.line << ": [" << v.ruleId << "] " << v.message << "\n";
    }
    ss << "Found " << viols.size() << " violation" << (viols.size() == 1 ? "" : "s") << ".\n";
    return ss.str();
}

std::string Reporter::toJson(const std::vector<Violation>& viols) {
    std::ostringstream ss;
    ss << "{\n  \"violations\": [\n";
    for (size_t i = 0; i < viols.size(); ++i) {
        const auto& v = viols[i];
        ss << "    {\n      \"file\": \"" << esc(v.file) << "\",\n"
           << "      \"line\": " << v.line << ",\n"
           << "      \"rule\": \"" << esc(v.ruleId) << "\",\n"
           << "      \"message\": \"" << esc(v.message) << "\"\n    }"
           << (i + 1 < viols.size() ? "," : "") << "\n";
    }
    ss << "  ],\n  \"summary\": { \"total\": " << viols.size() << " }\n}\n";
    return ss.str();
}

std::string Reporter::toSarif(const std::vector<Violation>& viols) {
    std::ostringstream ss;
    ss << "{\n  \"version\": \"2.1.0\",\n"
       << "  \"$schema\": \"https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json\",\n"
       << "  \"runs\": [\n    {\n      \"tool\": { \"driver\": { \"name\": \"misra_check\", \"version\": \"1.0.0\" } },\n"
       << "      \"results\": [\n";
    for (size_t i = 0; i < viols.size(); ++i) {
        const auto& v = viols[i];
        ss << "        {\n          \"ruleId\": \"" << esc(v.ruleId) << "\",\n"
           << "          \"message\": { \"text\": \"" << esc(v.message) << "\" },\n"
           << "          \"locations\": [{ \"physicalLocation\": { \"artifactLocation\": { \"uri\": \""
           << esc(v.file) << "\" }, \"region\": { \"startLine\": " << v.line << " } } }]\n        }"
           << (i + 1 < viols.size() ? "," : "") << "\n";
    }
    ss << "      ]\n    }\n  ]\n}\n";
    return ss.str();
}

void Reporter::print(const std::vector<Violation>& viols, OutputFormat fmt, std::ostream& out) {
    if (fmt == OutputFormat::Text) out << toText(viols);
    else if (fmt == OutputFormat::Json) out << toJson(viols);
    else if (fmt == OutputFormat::Sarif) out << toSarif(viols);
}
