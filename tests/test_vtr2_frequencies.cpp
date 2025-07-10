#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <cmath>

// Test to verify VTR2: Target frequencies configuration

bool checkFrequencies(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open " << filename << "\n";
        return false;
    }
    
    // Expected VTR target frequencies
    const std::vector<float> expectedFreqs = {80.0f, 240.0f, 2500.0f, 4000.0f, 10000.0f};
    
    std::string line;
    std::regex freqRegex("defaultFreqs.*=.*\\{([^}]+)\\}");
    bool found = false;
    
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, freqRegex)) {
            std::string freqStr = match[1];
            
            // Parse the frequencies
            std::vector<float> foundFreqs;
            std::regex numRegex("(\\d+\\.?\\d*)f?");
            auto nums_begin = std::sregex_iterator(freqStr.begin(), freqStr.end(), numRegex);
            auto nums_end = std::sregex_iterator();
            
            for (std::sregex_iterator i = nums_begin; i != nums_end; ++i) {
                std::smatch numMatch = *i;
                foundFreqs.push_back(std::stof(numMatch.str(1)));
            }
            
            // Compare frequencies
            if (foundFreqs.size() == expectedFreqs.size()) {
                bool allMatch = true;
                std::cout << "Found frequencies: ";
                for (size_t i = 0; i < foundFreqs.size(); ++i) {
                    std::cout << foundFreqs[i] << "Hz ";
                    if (std::abs(foundFreqs[i] - expectedFreqs[i]) > 0.1f) {
                        allMatch = false;
                    }
                }
                std::cout << "\n";
                
                if (allMatch) {
                    std::cout << "✓ Frequencies match VTR targets!\n";
                    found = true;
                } else {
                    std::cout << "✗ Frequencies do not match VTR targets\n";
                    std::cout << "Expected: 80Hz, 240Hz, 2500Hz, 4000Hz, 10000Hz\n";
                }
            }
            break;
        }
    }
    
    file.close();
    return found;
}

int main() {
    std::cout << "=== VTR2 Test: VTR Target Frequencies Configuration ===\n\n";
    
    bool processorOk = false;
    bool displayOk = false;
    
    // Check PluginProcessor.cpp
    std::cout << "Checking PluginProcessor.cpp...\n";
    processorOk = checkFrequencies("../Source/PluginProcessor.cpp");
    
    std::cout << "\nChecking FrequencyResponseDisplay.cpp...\n";
    displayOk = checkFrequencies("../Source/FrequencyResponseDisplay.cpp");
    
    std::cout << "\n";
    
    if (processorOk && displayOk) {
        std::cout << "✓ SUCCESS: VTR target frequencies correctly configured!\n";
        std::cout << "✓ VTR2 task completed successfully\n";
        return 0;
    } else {
        std::cout << "✗ FAIL: VTR target frequencies not properly configured\n";
        return 1;
    }
}