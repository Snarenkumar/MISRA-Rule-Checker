#include "Reporter.h"  // Includes the Reporter class
#include <iostream>    // Provides cout and cerr for input/output
#include <fstream>     // Used to read files
#include <sstream>     // Used to store file contents in a string stream
#include <string>      // Used to store text

int main(int argc, char* argv[]) { // argc = argument count; argv = command-line arguments

    if (argc < 2) { // Checks whether a file name is provided
        std::cout << "Usage: misra_check <file.cpp> [--json] [--sarif]\n"; // Shows usage instructions
        return 1; // Stops with an error status
    }

    std::string file; // Stores the input file name
    OutputFormat fmt = OutputFormat::Text; // Sets Text as the default output format

    for (int i = 1; i < argc; ++i) { // Checks each command-line argument
        std::string arg = argv[i]; // Stores the current argument

        if (arg == "--json") // Checks whether JSON output is requested
            fmt = OutputFormat::Json; // Selects JSON format
        else if (arg == "--sarif") // Checks whether SARIF output is requested
            fmt = OutputFormat::Sarif; // Selects SARIF format
        else if (file.empty()) // Checks whether the file name is not yet set
            file = arg; // Saves the file name
    }

    std::ifstream in(file); // Opens the file for reading

    if (!in.is_open()) { // Checks whether the file opened successfully
        std::cerr << "Error: cannot open " << file << "\n"; // Displays an error message
        return 1; // Stops because the file could not be opened
    }

    std::stringstream buf; // Creates a stream to store text in memory
    buf << in.rdbuf(); // Copies the entire file contents into buf

    RuleEngine engine; // Creates an object to check the source code
    auto viols = engine.run(file, buf.str()); // Checks the code and stores detected violations

    Reporter::print(viols, fmt); // Displays the results in Text, JSON, or SARIF format

    return viols.empty() ? 0 : 1; // Returns 0 if no violations are found; otherwise returns 1
}