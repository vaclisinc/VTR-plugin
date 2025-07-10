#include "VTRNetwork.h"
#include <juce_core/juce_core.h>
#include <fstream>
#include <cmath>
#include <algorithm>

VTRNetwork::VTRNetwork()
    : modelLoaded_(false)
{
    scaler_ = std::make_unique<StandardScaler>();
    layer1_ = std::make_unique<LinearLayer>();
    layer2_ = std::make_unique<LinearLayer>();
    layer3_ = std::make_unique<LinearLayer>();
}

VTRNetwork::~VTRNetwork() = default;

bool VTRNetwork::loadModel(const std::string& modelWeightsPath, const std::string& scalerParamsPath)
{
    // Load scaler parameters
    if (!scaler_->loadParameters(scalerParamsPath))
    {
        juce::Logger::writeToLog("Failed to load scaler parameters from: " + scalerParamsPath);
        return false;
    }
    
    // Load model weights
    if (!loadWeightsFromJSON(modelWeightsPath))
    {
        juce::Logger::writeToLog("Failed to load model weights from: " + modelWeightsPath);
        return false;
    }
    
    modelLoaded_ = true;
    juce::Logger::writeToLog("VTR model loaded successfully");
    return true;
}

std::vector<float> VTRNetwork::predict(const std::vector<float>& features)
{
    if (!modelLoaded_ || features.size() != INPUT_SIZE)
    {
        return std::vector<float>(OUTPUT_SIZE, 0.0f);
    }
    
    // 1. Normalize features using StandardScaler
    auto normalizedFeatures = scaler_->transform(features);
    
    // 2. Forward pass through network
    // Layer 1: Linear -> ReLU
    auto layer1Output = layer1_->forward(normalizedFeatures);
    auto layer1Activated = applyReLU(layer1Output);
    
    // Layer 2: Linear -> ReLU
    auto layer2Output = layer2_->forward(layer1Activated);
    auto layer2Activated = applyReLU(layer2Output);
    
    // Layer 3: Linear (output layer, no activation)
    auto output = layer3_->forward(layer2Activated);
    
    return output;
}

bool VTRNetwork::isModelLoaded() const
{
    return modelLoaded_;
}

int VTRNetwork::getInputDimensions() const
{
    return INPUT_SIZE;
}

int VTRNetwork::getOutputDimensions() const
{
    return OUTPUT_SIZE;
}

std::vector<float> VTRNetwork::applyReLU(const std::vector<float>& input)
{
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i)
    {
        output[i] = std::max(0.0f, input[i]);
    }
    return output;
}

bool VTRNetwork::loadWeightsFromJSON(const std::string& jsonPath)
{
    try
    {
        juce::File jsonFile(jsonPath);
        if (!jsonFile.existsAsFile())
        {
            juce::Logger::writeToLog("Model weights file not found: " + jsonPath);
            return false;
        }
        
        auto jsonText = jsonFile.loadFileAsString();
        auto json = juce::JSON::parse(jsonText);
        
        if (!json.isObject())
        {
            juce::Logger::writeToLog("Invalid JSON format in model weights file");
            return false;
        }
        
        auto* jsonObj = json.getDynamicObject();
        if (jsonObj == nullptr)
        {
            return false;
        }
        
        // Load layer 1 weights (17 -> 64)
        auto layer0 = jsonObj->getProperty("layer_0");
        if (layer0.isObject())
        {
            auto* layer0Obj = layer0.getDynamicObject();
            auto weights = layer0Obj->getProperty("weight");
            auto biases = layer0Obj->getProperty("bias");
            
            std::vector<std::vector<float>> weightMatrix;
            std::vector<float> biasVector;
            
            // Parse weights (64 x 17 matrix)
            if (weights.isArray())
            {
                auto* weightsArray = weights.getArray();
                for (int i = 0; i < weightsArray->size(); ++i)
                {
                    auto row = weightsArray->getUnchecked(i);
                    if (row.isArray())
                    {
                        auto* rowArray = row.getArray();
                        std::vector<float> weightRow;
                        for (int j = 0; j < rowArray->size(); ++j)
                        {
                            weightRow.push_back(static_cast<float>(rowArray->getUnchecked(j)));
                        }
                        weightMatrix.push_back(weightRow);
                    }
                }
            }
            
            // Parse biases
            if (biases.isArray())
            {
                auto* biasArray = biases.getArray();
                for (int i = 0; i < biasArray->size(); ++i)
                {
                    biasVector.push_back(static_cast<float>(biasArray->getUnchecked(i)));
                }
            }
            
            if (!layer1_->loadWeights(weightMatrix, biasVector))
            {
                return false;
            }
        }
        
        // Load layer 2 weights (64 -> 64)
        auto layer1 = jsonObj->getProperty("layer_1");
        if (layer1.isObject())
        {
            auto* layer1Obj = layer1.getDynamicObject();
            auto weights = layer1Obj->getProperty("weight");
            auto biases = layer1Obj->getProperty("bias");
            
            std::vector<std::vector<float>> weightMatrix;
            std::vector<float> biasVector;
            
            // Parse weights and biases similar to layer 1
            if (weights.isArray())
            {
                auto* weightsArray = weights.getArray();
                for (int i = 0; i < weightsArray->size(); ++i)
                {
                    auto row = weightsArray->getUnchecked(i);
                    if (row.isArray())
                    {
                        auto* rowArray = row.getArray();
                        std::vector<float> weightRow;
                        for (int j = 0; j < rowArray->size(); ++j)
                        {
                            weightRow.push_back(static_cast<float>(rowArray->getUnchecked(j)));
                        }
                        weightMatrix.push_back(weightRow);
                    }
                }
            }
            
            if (biases.isArray())
            {
                auto* biasArray = biases.getArray();
                for (int i = 0; i < biasArray->size(); ++i)
                {
                    biasVector.push_back(static_cast<float>(biasArray->getUnchecked(i)));
                }
            }
            
            if (!layer2_->loadWeights(weightMatrix, biasVector))
            {
                return false;
            }
        }
        
        // Load layer 3 weights (64 -> 5)
        auto layer2 = jsonObj->getProperty("layer_2");
        if (layer2.isObject())
        {
            auto* layer2Obj = layer2.getDynamicObject();
            auto weights = layer2Obj->getProperty("weight");
            auto biases = layer2Obj->getProperty("bias");
            
            std::vector<std::vector<float>> weightMatrix;
            std::vector<float> biasVector;
            
            // Parse weights and biases similar to previous layers
            if (weights.isArray())
            {
                auto* weightsArray = weights.getArray();
                for (int i = 0; i < weightsArray->size(); ++i)
                {
                    auto row = weightsArray->getUnchecked(i);
                    if (row.isArray())
                    {
                        auto* rowArray = row.getArray();
                        std::vector<float> weightRow;
                        for (int j = 0; j < rowArray->size(); ++j)
                        {
                            weightRow.push_back(static_cast<float>(rowArray->getUnchecked(j)));
                        }
                        weightMatrix.push_back(weightRow);
                    }
                }
            }
            
            if (biases.isArray())
            {
                auto* biasArray = biases.getArray();
                for (int i = 0; i < biasArray->size(); ++i)
                {
                    biasVector.push_back(static_cast<float>(biasArray->getUnchecked(i)));
                }
            }
            
            if (!layer3_->loadWeights(weightMatrix, biasVector))
            {
                return false;
            }
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("Exception loading model weights: " + juce::String(e.what()));
        return false;
    }
}

// StandardScaler implementation
VTRNetwork::StandardScaler::StandardScaler()
    : loaded_(false)
{
}

bool VTRNetwork::StandardScaler::loadParameters(const std::string& jsonPath)
{
    try
    {
        juce::File jsonFile(jsonPath);
        if (!jsonFile.existsAsFile())
        {
            juce::Logger::writeToLog("Scaler parameters file not found: " + jsonPath);
            return false;
        }
        
        auto jsonText = jsonFile.loadFileAsString();
        auto json = juce::JSON::parse(jsonText);
        
        if (!json.isObject())
        {
            juce::Logger::writeToLog("Invalid JSON format in scaler parameters file");
            return false;
        }
        
        auto* jsonObj = json.getDynamicObject();
        if (jsonObj == nullptr)
        {
            return false;
        }
        
        // Load mean values
        auto meanVar = jsonObj->getProperty("mean");
        if (meanVar.isArray())
        {
            auto* meanArray = meanVar.getArray();
            mean_.clear();
            for (int i = 0; i < meanArray->size(); ++i)
            {
                mean_.push_back(static_cast<float>(meanArray->getUnchecked(i)));
            }
        }
        
        // Load std values
        auto stdVar = jsonObj->getProperty("std");
        if (stdVar.isArray())
        {
            auto* stdArray = stdVar.getArray();
            std_.clear();
            for (int i = 0; i < stdArray->size(); ++i)
            {
                std_.push_back(static_cast<float>(stdArray->getUnchecked(i)));
            }
        }
        
        // Verify dimensions
        if (mean_.size() != std_.size() || mean_.size() != INPUT_SIZE)
        {
            juce::Logger::writeToLog("Scaler parameters dimension mismatch");
            return false;
        }
        
        loaded_ = true;
        return true;
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("Exception loading scaler parameters: " + juce::String(e.what()));
        return false;
    }
}

std::vector<float> VTRNetwork::StandardScaler::transform(const std::vector<float>& features)
{
    if (!loaded_ || features.size() != mean_.size())
    {
        return features; // Return original if not loaded or size mismatch
    }
    
    std::vector<float> normalized(features.size());
    for (size_t i = 0; i < features.size(); ++i)
    {
        // Normalize: (x - mean) / std
        normalized[i] = (features[i] - mean_[i]) / std_[i];
    }
    
    return normalized;
}

bool VTRNetwork::StandardScaler::isLoaded() const
{
    return loaded_;
}

// LinearLayer implementation
VTRNetwork::LinearLayer::LinearLayer()
    : loaded_(false)
{
}

bool VTRNetwork::LinearLayer::loadWeights(const std::vector<std::vector<float>>& weights, const std::vector<float>& biases)
{
    if (weights.empty() || biases.empty())
    {
        return false;
    }
    
    // Verify dimensions
    if (weights.size() != biases.size())
    {
        juce::Logger::writeToLog("Weight and bias dimension mismatch");
        return false;
    }
    
    weights_ = weights;
    biases_ = biases;
    loaded_ = true;
    
    return true;
}

std::vector<float> VTRNetwork::LinearLayer::forward(const std::vector<float>& input)
{
    if (!loaded_ || input.empty())
    {
        return {};
    }
    
    std::vector<float> output(weights_.size());
    
    // Matrix multiplication: output = weights * input + biases
    for (size_t i = 0; i < weights_.size(); ++i)
    {
        float sum = 0.0f;
        for (size_t j = 0; j < input.size() && j < weights_[i].size(); ++j)
        {
            sum += weights_[i][j] * input[j];
        }
        output[i] = sum + biases_[i];
    }
    
    return output;
}