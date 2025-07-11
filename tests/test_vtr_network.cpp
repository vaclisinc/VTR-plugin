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
    
    // Use actual test vector from your training data
    std::vector<float> testFeatures = {
        -0.11562023311853409f,
        1.1053447723388672f,
        0.8938038945198059f,
        1.9258779287338257f,
        -0.6831626892089844f,
        0.5091536641120911f,
        -1.8066099882125854f,
        0.4456981420516968f,
        -1.5267255306243896f,
        0.6359248161315918f,
        0.6453673839569092f,
        -0.648241400718689f,
        1.242024302482605f,
        -1.6386936902999878f,
        0.08051995933055878f,
        0.18829967081546783f,
        0.9597957730293274f
    };
    
    std::cout << "Using actual training data test vector" << std::endl;
    std::cout << "Expected output from training: [21.69, 312.08, 126.56, -368.52, -105.29]" << std::endl;
    
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