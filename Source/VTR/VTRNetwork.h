#pragma once

#include <vector>
#include <string>
#include <memory>

class VTRNetwork
{
public:
    VTRNetwork();
    ~VTRNetwork();
    
    // Load model from JSON files
    bool loadModel(const std::string& modelWeightsPath, const std::string& scalerParamsPath);
    
    // Inference
    std::vector<float> predict(const std::vector<float>& features);
    
    // Model information
    bool isModelLoaded() const;
    int getInputDimensions() const;
    int getOutputDimensions() const;
    
private:
    // StandardScaler for feature normalization
    class StandardScaler
    {
    public:
        StandardScaler();
        bool loadParameters(const std::string& jsonPath);
        std::vector<float> transform(const std::vector<float>& features);
        bool isLoaded() const;
        
    private:
        std::vector<float> mean_;
        std::vector<float> std_;
        bool loaded_;
    };
    
    // Neural network layer
    class LinearLayer
    {
    public:
        LinearLayer();
        bool loadWeights(const std::vector<std::vector<float>>& weights, const std::vector<float>& biases);
        std::vector<float> forward(const std::vector<float>& input);
        
    private:
        std::vector<std::vector<float>> weights_;
        std::vector<float> biases_;
        bool loaded_;
    };
    
    // Network components
    std::unique_ptr<StandardScaler> scaler_;
    std::unique_ptr<LinearLayer> layer1_;
    std::unique_ptr<LinearLayer> layer2_;
    std::unique_ptr<LinearLayer> layer3_;
    
    // Model state
    bool modelLoaded_;
    
    // Helper functions
    std::vector<float> applyReLU(const std::vector<float>& input);
    bool loadWeightsFromJSON(const std::string& jsonPath);
    
    // Network architecture constants
    static constexpr int INPUT_SIZE = 17;
    static constexpr int HIDDEN_SIZE = 64;
    static constexpr int OUTPUT_SIZE = 5;
};