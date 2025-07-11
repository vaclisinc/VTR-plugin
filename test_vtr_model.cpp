#include "Source/VTR/VTRNetwork.h"
#include <iostream>
#include <vector>

int main()
{
    std::cout << "Testing VTR Model..." << std::endl;
    
    // Create VTR network
    VTRNetwork network;
    
    // Test data - use your actual training data here
    std::vector<float> testFeatures = {
        0.1f, 1500.0f, 0.5f, -0.2f, 0.8f, -0.1f, 0.4f, 
        -0.3f, 0.2f, -0.1f, 0.1f, 0.0f, -0.2f, 0.1f, 
        0.3f, 800.0f, 0.7f  // 17 features total
    };
    
    std::cout << "Input features: ";
    for (float f : testFeatures) {
        std::cout << f << " ";
    }
    std::cout << std::endl;
    
    // Test without loading model first
    std::cout << "Testing without loaded model..." << std::endl;
    auto result1 = network.predict(testFeatures);
    std::cout << "Result size: " << result1.size() << std::endl;
    if (result1.size() == 5) {
        std::cout << "Predictions: ";
        for (float p : result1) {
            std::cout << p << " ";
        }
        std::cout << std::endl;
    }
    
    // Try to load model (will fail if files don't exist)
    std::cout << "Attempting to load model..." << std::endl;
    bool loaded = network.loadModel("model_weights.json", "scaler_params.json");
    std::cout << "Model loaded: " << (loaded ? "YES" : "NO") << std::endl;
    
    if (loaded) {
        std::cout << "Testing with loaded model..." << std::endl;
        auto result2 = network.predict(testFeatures);
        std::cout << "Result size: " << result2.size() << std::endl;
        if (result2.size() == 5) {
            std::cout << "Predictions: ";
            for (float p : result2) {
                std::cout << p << " ";
            }
            std::cout << std::endl;
        }
    }
    
    return 0;
}