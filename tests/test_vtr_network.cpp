#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../Source/VTR/VTRNetwork.h"

// Test functions
void testNetworkCreation()
{
    std::cout << "Testing VTR network creation..." << std::endl;
    
    VTRNetwork network;
    
    // Should not be loaded initially
    assert(!network.isModelLoaded());
    assert(network.getInputDimensions() == 17);
    assert(network.getOutputDimensions() == 5);
    
    std::cout << "✓ VTR network created successfully" << std::endl;
}

void testModelLoading()
{
    std::cout << "Testing VTR model loading..." << std::endl;
    
    VTRNetwork network;
    
    // Test loading model files
    std::string modelPath = "../vtr-model/exported_model/model_weights.json";
    std::string scalerPath = "../vtr-model/exported_model/scaler_params.json";
    
    bool loaded = network.loadModel(modelPath, scalerPath);
    
    if (!loaded)
    {
        std::cout << "❌ Failed to load model files" << std::endl;
        std::cout << "Model path: " << modelPath << std::endl;
        std::cout << "Scaler path: " << scalerPath << std::endl;
        throw std::runtime_error("Model loading failed");
    }
    
    assert(network.isModelLoaded());
    
    std::cout << "✓ VTR model loaded successfully" << std::endl;
}

void testPrediction()
{
    std::cout << "Testing VTR prediction..." << std::endl;
    
    VTRNetwork network;
    
    // Load model
    std::string modelPath = "../vtr-model/exported_model/model_weights.json";
    std::string scalerPath = "../vtr-model/exported_model/scaler_params.json";
    
    if (!network.loadModel(modelPath, scalerPath))
    {
        std::cout << "❌ Failed to load model for prediction test" << std::endl;
        throw std::runtime_error("Model loading failed");
    }
    
    // Create test feature vector (17 dimensions)
    std::vector<float> testFeatures = {
        0.1f,        // RMS energy
        1000.0f,     // Spectral centroid
        5.0f,        // MFCC 1
        8.0f,        // MFCC 2
        3.0f,        // MFCC 3
        2.0f,        // MFCC 4
        1.5f,        // MFCC 5
        1.0f,        // MFCC 6
        1.0f,        // MFCC 7
        0.8f,        // MFCC 8
        0.7f,        // MFCC 9
        0.6f,        // MFCC 10
        0.5f,        // MFCC 11
        0.4f,        // MFCC 12
        0.4f,        // MFCC 13
        500.0f,      // Spectral bandwidth
        2000.0f      // Spectral rolloff
    };
    
    // Test prediction
    auto predictions = network.predict(testFeatures);
    
    // Should return 5 EQ parameters
    assert(predictions.size() == 5);
    
    std::cout << "Predictions: [";
    for (size_t i = 0; i < predictions.size(); ++i)
    {
        std::cout << predictions[i];
        if (i < predictions.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // Check that predictions are reasonable values (not NaN or inf)
    for (float pred : predictions)
    {
        assert(std::isfinite(pred));
    }
    
    std::cout << "✓ VTR prediction completed successfully" << std::endl;
}

void testDimensionMismatch()
{
    std::cout << "Testing prediction with wrong input dimensions..." << std::endl;
    
    VTRNetwork network;
    
    // Load model
    std::string modelPath = "../vtr-model/exported_model/model_weights.json";
    std::string scalerPath = "../vtr-model/exported_model/scaler_params.json";
    
    if (!network.loadModel(modelPath, scalerPath))
    {
        std::cout << "❌ Failed to load model for dimension test" << std::endl;
        throw std::runtime_error("Model loading failed");
    }
    
    // Test with wrong number of features
    std::vector<float> wrongFeatures = {1.0f, 2.0f, 3.0f}; // Only 3 features instead of 17
    
    auto predictions = network.predict(wrongFeatures);
    
    // Should return zeros when input dimension is wrong
    assert(predictions.size() == 5);
    
    // All predictions should be zero
    for (float pred : predictions)
    {
        assert(pred == 0.0f);
    }
    
    std::cout << "✓ Dimension mismatch handled correctly" << std::endl;
}

int main()
{
    std::cout << "Running VTR Network Tests..." << std::endl;
    std::cout << "=============================" << std::endl;
    
    try
    {
        testNetworkCreation();
        testModelLoading();
        testPrediction();
        testDimensionMismatch();
        
        std::cout << "=============================" << std::endl;
        std::cout << "All VTR Network tests PASSED!" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ Test FAILED: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cout << "❌ Test FAILED with unknown exception" << std::endl;
        return 1;
    }
}