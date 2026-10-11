#pragma once
#include "Rules.h"
#include <iostream>

enum class OutputFormat { Text, Json, Sarif };

// Formats violations as plain text, JSON, or SARIF 2.1.0
class Reporter {
public:
    static std::string toText(const std::vector<Violation>& viols);
    static std::string toJson(const std::vector<Violation>& viols);
    static std::string toSarif(const std::vector<Violation>& viols);
    static void print(const std::vector<Violation>& viols, OutputFormat fmt, std::ostream& out = std::cout);
};
