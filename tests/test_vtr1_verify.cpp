#include <iostream>
#include <fstream>
#include <string>
#include <regex>

// Test to verify VTR1 changes by reading the actual header file

int main() {
    std::cout << "=== VTR1 Verification: EQ Band Expansion ===\n";
    
    // Read the EQBand.h file to check CURRENT_BANDS value
    std::ifstream file("../Source/DSP/EQBand.h");
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open EQBand.h\n";
        return 1;
    }
    
    std::string line;
    std::regex bandsRegex("static constexpr int CURRENT_BANDS = (\\d+);");
    int foundBands = -1;
    
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, bandsRegex)) {
            foundBands = std::stoi(match[1]);
            break;
        }
    }
    
    file.close();
    
    if (foundBands == -1) {
        std::cerr << "ERROR: Could not find CURRENT_BANDS definition\n";
        return 1;
    }
    
    std::cout << "Found CURRENT_BANDS = " << foundBands << "\n";
    
    if (foundBands == 5) {
        std::cout << "✓ SUCCESS: Plugin has been expanded to 5 bands!\n";
        std::cout << "✓ VTR1 task completed successfully\n";
        return 0;
    } else {
        std::cerr << "✗ FAIL: Expected 5 bands, but found " << foundBands << "\n";
        return 1;
    }
}