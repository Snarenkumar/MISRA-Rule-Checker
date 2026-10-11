#include "Reporter.h"
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cout << "Usage: misra_check <file.cpp> [--json] [--sarif]\n"; return 1; }
    std::string file;
    OutputFormat fmt = OutputFormat::Text;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--json") fmt = OutputFormat::Json;
        else if (arg == "--sarif") fmt = OutputFormat::Sarif;
        else if (file.empty()) file = arg;
    }
    std::ifstream in(file);
    if (!in.is_open()) { std::cerr << "Error: cannot open " << file << "\n"; return 1; }
    std::stringstream buf; buf << in.rdbuf();
    RuleEngine engine;
    auto viols = engine.run(file, buf.str());
    Reporter::print(viols, fmt);
    return viols.empty() ? 0 : 1;
}
