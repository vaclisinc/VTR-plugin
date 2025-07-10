#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include "../Source/VTR/VTRNetwork.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// Test data from validation tests
const std::vector<float> TEST_FEATURES = {
    -0.11562023311853409f, 1.1053447723388672f, 0.8938038945198059f, 1.9258779287338257f,
    -0.6831626892089844f, 0.5091536641120911f, -1.8066099882125854f, 0.4456981420516968f,
    -1.5267255306243896f, 0.6359248161315918f, 0.6453673839569092f, -0.648241400718689f,
    1.242024302482605f, -1.6386936902999878f, 0.08051995933055878f, 0.18829967081546783f,
    0.9597957730293274f
};

const std::vector<float> EXPECTED_EQ_GAINS = {
    21.690460205078125f, 312.0833435058594f, 126.55596160888672f, 
    -368.5224609375f, -105.28917694091797f
};

class VTREndToEndTest
{
public:
    VTREndToEndTest() = default;
    
    bool runFullWorkflowTest()
    {
        std::cout << "VTR19: End-to-End Integration Test" << std::endl;
        std::cout << "===================================" << std::endl;
        
        try
        {
            // Test 1: Load VTR model
            if (!testVTRModelLoading())
            {
                return false;
            }
            
            // Test 2: Test feature processing workflow
            if (!testFeatureProcessingWorkflow())
            {
                return false;
            }
            
            // Test 3: Test complete VTR pipeline
            if (!testCompletePipeline())
            {
                return false;
            }
            
            // Test 4: Test EQ parameter validation
            if (!testEQParameterValidation())
            {
                return false;
            }
            
            std::cout << "✓ All end-to-end tests PASSED!" << std::endl;
            return true;
        }
        catch (const std::exception& e)
        {
            std::cout << "❌ End-to-end test failed with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
private:
    bool testVTRModelLoading()
    {
        std::cout << std::endl << "Test 1: VTR Model Loading" << std::endl;
        std::cout << "-------------------------" << std::endl;
        
        VTRNetwork network;
        
        // Check if model files exist
        juce::File modelFile("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/model_weights.json");
        juce::File scalerFile("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/scaler_params.json");
        
        if (!modelFile.exists())
        {
            std::cout << "❌ Model weights file not found" << std::endl;
            return false;
        }
        std::cout << "✓ Model weights file found" << std::endl;
        
        if (!scalerFile.exists())
        {
            std::cout << "❌ Scaler parameters file not found" << std::endl;
            return false;
        }
        std::cout << "✓ Scaler parameters file found" << std::endl;
        
        // Load the model
        bool loaded = network.loadModel(
            modelFile.getFullPathName().toStdString(),
            scalerFile.getFullPathName().toStdString()
        );
        
        if (!loaded)
        {
            std::cout << "❌ Failed to load VTR model" << std::endl;
            return false;
        }
        std::cout << "✓ VTR model loaded successfully" << std::endl;
        
        // Validate model architecture
        if (network.getInputDimensions() != 17)
        {
            std::cout << "❌ Expected 17 input dimensions, got " << network.getInputDimensions() << std::endl;
            return false;
        }
        std::cout << "✓ Input dimensions correct (17)" << std::endl;
        
        if (network.getOutputDimensions() != 5)
        {
            std::cout << "❌ Expected 5 output dimensions, got " << network.getOutputDimensions() << std::endl;
            return false;
        }
        std::cout << "✓ Output dimensions correct (5)" << std::endl;
        
        return true;
    }
    
    bool testVTRModelLoading(VaclisDynamicEQAudioProcessor& processor)
    {
        std::cout << std::endl << "Test 2: VTR Model Loading" << std::endl;
        std::cout << "-------------------------" << std::endl;
        
        // Check if model files exist
        juce::File modelFile("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/model_weights.json");
        juce::File scalerFile("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/scaler_params.json");
        
        if (!modelFile.exists())
        {
            std::cout << "❌ Model weights file not found: " << modelFile.getFullPathName() << std::endl;
            return false;
        }
        std::cout << "✓ Model weights file found" << std::endl;
        
        if (!scalerFile.exists())
        {
            std::cout << "❌ Scaler parameters file not found: " << scalerFile.getFullPathName() << std::endl;
            return false;
        }
        std::cout << "✓ Scaler parameters file found" << std::endl;
        
        // Load the model
        bool loaded = processor.loadVTRModel(
            modelFile.getFullPathName(),
            scalerFile.getFullPathName()
        );
        
        if (!loaded)
        {
            std::cout << "❌ Failed to load VTR model" << std::endl;
            return false;
        }
        std::cout << "✓ VTR model loaded successfully" << std::endl;
        
        // Test VTR network access
        auto& network = processor.getVTRNetwork();
        if (!network.isModelLoaded())
        {
            std::cout << "❌ VTR network reports model not loaded" << std::endl;
            return false;
        }
        std::cout << "✓ VTR network model validation passed" << std::endl;
        
        return true;
    }
    
    bool testAudioProcessingWorkflow(VaclisDynamicEQAudioProcessor& processor)
    {
        std::cout << std::endl << "Test 3: Audio Processing Workflow" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        
        // Setup audio processing
        const double sampleRate = 44100.0;
        const int bufferSize = 256;
        const int numChannels = 2;
        
        processor.prepareToPlay(sampleRate, bufferSize);
        std::cout << "✓ Processor prepared for playback" << std::endl;
        
        // Create test audio buffer with some content
        juce::AudioBuffer<float> audioBuffer(numChannels, bufferSize);
        
        // Fill buffer with a simple test signal (sine wave)
        for (int channel = 0; channel < numChannels; ++channel)
        {
            for (int sample = 0; sample < bufferSize; ++sample)
            {
                float value = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * sample / (float)sampleRate);
                audioBuffer.setSample(channel, sample, value * 0.1f);
            }
        }
        
        // Create empty MIDI buffer
        juce::MidiBuffer midiBuffer;
        
        // Process the buffer
        processor.processBlock(audioBuffer, midiBuffer);
        std::cout << "✓ Audio buffer processed successfully" << std::endl;
        
        // Check that output is not silence or corrupted
        bool hasValidOutput = false;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            for (int sample = 0; sample < bufferSize; ++sample)
            {
                float value = audioBuffer.getSample(channel, sample);
                if (std::abs(value) > 1e-6f && std::abs(value) < 10.0f) // Valid range
                {
                    hasValidOutput = true;
                    break;
                }
            }
            if (hasValidOutput) break;
        }
        
        if (!hasValidOutput)
        {
            std::cout << "❌ Audio output appears to be corrupted or silent" << std::endl;
            return false;
        }
        std::cout << "✓ Audio output validation passed" << std::endl;
        
        processor.releaseResources();
        std::cout << "✓ Resources released successfully" << std::endl;
        
        return true;
    }
    
    bool testVTRParameterApplication(VaclisDynamicEQAudioProcessor& processor)
    {
        std::cout << std::endl << "Test 4: VTR Parameter Application" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        
        // Test applying VTR predictions (using test values)
        std::vector<float> testPredictions = {2.5f, -1.8f, 3.2f, -0.5f, 1.1f};
        
        // Get initial parameter values
        auto& parameters = processor.getValueTreeState();
        std::vector<float> initialGains;
        
        for (int band = 0; band < 5; ++band)
        {
            juce::String paramID = "eq_gain_band" + juce::String(band);
            if (auto* param = parameters.getParameter(paramID))
            {
                initialGains.push_back(param->getValue());
            }
            else
            {
                std::cout << "❌ Could not find parameter: " << paramID << std::endl;
                return false;
            }
        }
        std::cout << "✓ Retrieved initial EQ gain values" << std::endl;
        
        // Apply VTR predictions
        processor.applyVTRPredictions(testPredictions);
        std::cout << "✓ VTR predictions applied successfully" << std::endl;
        
        // Check that parameters were updated
        std::vector<float> updatedGains;
        bool parametersChanged = false;
        
        for (int band = 0; band < 5; ++band)
        {
            juce::String paramID = "eq_gain_band" + juce::String(band);
            if (auto* param = parameters.getParameter(paramID))
            {
                float newValue = param->getValue();
                updatedGains.push_back(newValue);
                
                if (std::abs(newValue - initialGains[band]) > 1e-6f)
                {
                    parametersChanged = true;
                }
                
                std::cout << "  Band " << band << ": " << initialGains[band] 
                          << " → " << newValue << std::endl;
            }
        }
        
        if (!parametersChanged)
        {
            std::cout << "❌ EQ parameters were not updated after VTR predictions" << std::endl;
            return false;
        }
        std::cout << "✓ EQ parameters updated successfully" << std::endl;
        
        return true;
    }
};

int main()
{
    std::cout << "VTR19: End-to-End Integration Testing" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    VTREndToEndTest test;
    bool success = test.runFullWorkflowTest();
    
    if (success)
    {
        std::cout << std::endl << "🎉 VTR19 End-to-end integration test PASSED!" << std::endl;
        std::cout << "All VTR functionality is working correctly!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << std::endl << "❌ VTR19 End-to-end integration test FAILED!" << std::endl;
        return 1;
    }
}