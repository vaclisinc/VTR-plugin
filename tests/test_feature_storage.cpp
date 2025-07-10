#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "../Source/SpectrumAnalyzer.h"
#include <iostream>
#include <random>
#include <thread>
#include <chrono>

class FeatureStorageTest
{
public:
    void runTest()
    {
        std::cout << "Testing VTR3 Feature Storage System..." << std::endl;
        
        // Create spectrum analyzer
        SpectrumAnalyzer analyzer;
        
        // Audio setup
        const double sampleRate = 48000.0;
        const int bufferSize = 256;
        const int numChannels = 2;
        
        analyzer.prepare(sampleRate, bufferSize);
        
        // Enable feature extraction
        analyzer.enableFeatureExtraction(true);
        analyzer.setFeatureUpdateRate(5.0f); // 5 Hz update rate
        
        std::cout << "✓ Feature extraction enabled" << std::endl;
        
        // Create test audio buffer
        juce::AudioBuffer<float> inputBuffer(numChannels, bufferSize);
        juce::AudioBuffer<float> outputBuffer(numChannels, bufferSize);
        
        // Generate random audio data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dis(0.0f, 0.1f);
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            for (int sample = 0; sample < bufferSize; ++sample)
            {
                float value = dis(gen);
                inputBuffer.setSample(channel, sample, value);
                outputBuffer.setSample(channel, sample, value);
            }
        }
        
        std::cout << "✓ Test audio generated" << std::endl;
        
        // Process multiple blocks to trigger feature extraction
        int blocksProcessed = 0;
        int featuresExtracted = 0;
        
        for (int i = 0; i < 50; ++i) // Process 50 blocks
        {
            // Process block through spectrum analyzer
            analyzer.processBlock(inputBuffer, outputBuffer);
            blocksProcessed++;
            
            // Check if new features are available
            if (analyzer.hasNewFeatures())
            {
                auto features = analyzer.getLatestFeatures();
                featuresExtracted++;
                
                std::cout << "Features extracted #" << featuresExtracted << ": [";
                for (size_t j = 0; j < std::min(size_t(5), features.size()); ++j)
                {
                    std::cout << features[j];
                    if (j < 4) std::cout << ", ";
                }
                std::cout << "...] (17 total)" << std::endl;
                
                // Verify feature vector size
                if (features.size() != 17)
                {
                    std::cout << "❌ ERROR: Feature vector size = " << features.size() << ", expected 17" << std::endl;
                    std::exit(1);
                }
            }
            
            // Small delay to simulate real-time processing
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "✓ Processed " << blocksProcessed << " blocks" << std::endl;
        std::cout << "✓ Extracted " << featuresExtracted << " feature vectors" << std::endl;
        
        // Test feature extraction disable
        analyzer.enableFeatureExtraction(false);
        bool hadFeaturesBeforeDisable = analyzer.hasNewFeatures();
        
        // Process more blocks - should not extract features
        for (int i = 0; i < 10; ++i)
        {
            analyzer.processBlock(inputBuffer, outputBuffer);
        }
        
        // Should not have new features after disabling
        bool hasNewFeaturesAfterDisable = analyzer.hasNewFeatures();
        
        if (hasNewFeaturesAfterDisable == hadFeaturesBeforeDisable)
        {
            std::cout << "✓ Feature extraction properly disabled" << std::endl;
        }
        else
        {
            std::cout << "❌ ERROR: Feature extraction not properly disabled" << std::endl;
            std::exit(1);
        }
        
        if (featuresExtracted > 0)
        {
            std::cout << "✓ VTR3 Feature Storage Test PASSED!" << std::endl;
        }
        else
        {
            std::cout << "❌ VTR3 Feature Storage Test FAILED - No features extracted" << std::endl;
            std::exit(1);
        }
    }
};

int main()
{
    std::cout << "VTR3 Feature Storage Test" << std::endl;
    std::cout << "=========================" << std::endl;
    
    try
    {
        FeatureStorageTest test;
        test.runTest();
        std::cout << "All tests completed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cout << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}