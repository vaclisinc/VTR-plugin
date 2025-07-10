#include <iostream>
#include <vector>
#include <cmath>
#include "../Source/VTR/VTRNetwork.h"
#include <juce_core/juce_core.h>

// Test data from exported model test vectors
const std::vector<float> INPUT_FEATURES = {
    -0.11562023311853409f, 1.1053447723388672f, 0.8938038945198059f, 1.9258779287338257f,
    -0.6831626892089844f, 0.5091536641120911f, -1.8066099882125854f, 0.4456981420516968f,
    -1.5267255306243896f, 0.6359248161315918f, 0.6453673839569092f, -0.648241400718689f,
    1.242024302482605f, -1.6386936902999878f, 0.08051995933055878f, 0.18829967081546783f,
    0.9597957730293274f
};

const std::vector<float> EXPECTED_OUTPUT = {
    21.690460205078125f, 312.0833435058594f, 126.55596160888672f, 
    -368.5224609375f, -105.28917694091797f
};

// Tolerance for neural network output (0.1% tolerance)
const float NN_TOLERANCE = 0.001f;

bool compareVectors(const std::vector<float>& vec1, const std::vector<float>& vec2, 
                   const std::string& name, float tolerance = NN_TOLERANCE)
{
    if (vec1.size() != vec2.size())
    {
        std::cout << "❌ " << name << " size mismatch: " << vec1.size() 
                  << " vs " << vec2.size() << std::endl;
        return false;
    }
    
    bool allMatch = true;
    float maxError = 0.0f;
    
    std::cout << "Comparing " << name << ":" << std::endl;
    for (size_t i = 0; i < vec1.size(); ++i)
    {
        float error = std::abs(vec1[i] - vec2[i]);
        float relativeError = error / (std::abs(vec2[i]) + 1e-8f);
        
        maxError = std::max(maxError, relativeError);
        
        std::cout << "  [" << i << "] Got: " << vec1[i] << ", Expected: " << vec2[i] 
                  << ", Error: " << (relativeError * 100.0f) << "%" << std::endl;
        
        if (relativeError > tolerance)
        {
            allMatch = false;
        }
    }
    
    if (allMatch)
    {
        std::cout << "✓ " << name << " validation PASSED (max error: " 
                  << (maxError * 100.0f) << "%)" << std::endl;
    }
    else
    {
        std::cout << "❌ " << name << " validation FAILED (max error: " 
                  << (maxError * 100.0f) << "%)" << std::endl;
    }
    
    return allMatch;
}

bool testNetworkInference()
{
    std::cout << "VTR18: Neural Network Inference Validation" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    try
    {
        VTRNetwork network;
        
        // Load model weights
        juce::File modelFile("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/model_weights.json");
        juce::File scalerFile("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/scaler_params.json");
        
        if (!modelFile.exists())
        {
            std::cout << "❌ Model weights file not found: " << modelFile.getFullPathName() << std::endl;
            return false;
        }
        
        if (!scalerFile.exists())
        {
            std::cout << "❌ Scaler parameters file not found: " << scalerFile.getFullPathName() << std::endl;
            return false;
        }
        
        // Load the model
        bool loaded = network.loadModel(modelFile.getFullPathName().toStdString(),
                                      scalerFile.getFullPathName().toStdString());
        
        if (!loaded)
        {
            std::cout << "❌ Failed to load VTR model" << std::endl;
            return false;
        }
        
        std::cout << "✓ VTR model loaded successfully" << std::endl;
        
        // Run inference on test vector
        auto predictions = network.predict(INPUT_FEATURES);
        
        std::cout << "Input features: [";
        for (size_t i = 0; i < INPUT_FEATURES.size(); ++i)
        {
            std::cout << INPUT_FEATURES[i];
            if (i < INPUT_FEATURES.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        
        // Compare predictions with expected output
        bool inferenceValid = compareVectors(predictions, EXPECTED_OUTPUT, 
                                           "Neural network inference", 0.01f); // 1% tolerance
        
        if (!inferenceValid)
        {
            std::cout << "❌ Neural network predictions don't match expected values" << std::endl;
            return false;
        }
        
        // Test network architecture validation
        std::cout << std::endl << "Testing network architecture:" << std::endl;
        
        // Check input size
        if (INPUT_FEATURES.size() != 17)
        {
            std::cout << "❌ Expected input size 17, got " << INPUT_FEATURES.size() << std::endl;
            return false;
        }
        std::cout << "✓ Input size validation (17 features)" << std::endl;
        
        // Check output size
        if (predictions.size() != 5)
        {
            std::cout << "❌ Expected output size 5, got " << predictions.size() << std::endl;
            return false;
        }
        std::cout << "✓ Output size validation (5 EQ bands)" << std::endl;
        
        std::cout << "✓ Network architecture validation PASSED" << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ Network validation failed with exception: " << e.what() << std::endl;
        return false;
    }
}

int main()
{
    std::cout << "VTR18: Neural Network Inference Accuracy Validation" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    bool success = testNetworkInference();
    
    if (success)
    {
        std::cout << std::endl << "🎉 VTR18 Neural network validation PASSED!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << std::endl << "❌ VTR18 Neural network validation FAILED!" << std::endl;
        return 1;
    }
}