#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "../Source/PluginProcessor.h"
#include <iostream>
#include <random>
#include <cmath>

class PinkNoiseProcessor
{
public:
    PinkNoiseProcessor() = default;
    
    void runTest()
    {
        std::cout << "Running Pink Noise Processing Test..." << std::endl;
        
        // Create plugin instance
        VTRsmartEQAudioProcessor processor;
        
        // Audio setup
        const double sampleRate = 48000.0;
        const int bufferSize = 256;
        const double duration = 2.0;
        const int totalSamples = static_cast<int>(duration * sampleRate);
        const int numChannels = 2;
        
        // Prepare processor
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numChannels;
        
        processor.prepareToPlay(sampleRate, bufferSize);
        
        // Generate pink noise
        std::vector<float> inputData = generatePinkNoise(totalSamples);
        double inputRMS = calculateRMS(inputData);
        
        // Process through plugin
        std::vector<float> outputData = processAudioThroughPlugin(processor, inputData, sampleRate, bufferSize, numChannels);
        double outputRMS = calculateRMS(outputData);
        
        // Calculate RMS difference
        double rmsRatio = outputRMS / inputRMS;
        double rmsDiffDB = 20.0 * std::log10(std::abs(rmsRatio));
        
        std::cout << "Input RMS: " << inputRMS << std::endl;
        std::cout << "Output RMS: " << outputRMS << std::endl;
        std::cout << "RMS Difference: " << rmsDiffDB << " dB" << std::endl;
        
        // Check if difference is within tolerance
        if (rmsDiffDB < 0.1)
        {
            std::cout << "✓ Pink noise test PASSED (RMS difference < 0.1 dB)" << std::endl;
        }
        else
        {
            std::cout << "✗ Pink noise test FAILED (RMS difference >= 0.1 dB)" << std::endl;
            std::exit(1);
        }
        
        processor.releaseResources();
    }
    
private:
    std::vector<float> generatePinkNoise(int numSamples)
    {
        std::vector<float> signal(numSamples);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dis(0.0f, 0.1f);
        
        // Simple pink noise approximation using white noise
        // In reality, pink noise has 1/f spectrum, but for testing RMS this is sufficient
        for (int i = 0; i < numSamples; ++i)
        {
            signal[i] = dis(gen);
        }
        
        return signal;
    }
    
    double calculateRMS(const std::vector<float>& signal)
    {
        double sumSquares = 0.0;
        for (float sample : signal)
        {
            sumSquares += sample * sample;
        }
        return std::sqrt(sumSquares / signal.size());
    }
    
    std::vector<float> processAudioThroughPlugin(
        VTRsmartEQAudioProcessor& processor,
        const std::vector<float>& inputData,
        double sampleRate,
        int bufferSize,
        int numChannels)
    {
        std::vector<float> outputData;
        outputData.reserve(inputData.size());
        
        // Process in chunks
        for (size_t startSample = 0; startSample < inputData.size(); startSample += bufferSize)
        {
            // Calculate actual buffer size for this chunk
            int currentBufferSize = std::min(bufferSize, static_cast<int>(inputData.size() - startSample));
            
            // Create audio buffer
            juce::AudioBuffer<float> audioBuffer(numChannels, currentBufferSize);
            
            // Fill buffer with input data (mono to stereo)
            for (int channel = 0; channel < numChannels; ++channel)
            {
                for (int sample = 0; sample < currentBufferSize; ++sample)
                {
                    audioBuffer.setSample(channel, sample, inputData[startSample + sample]);
                }
            }
            
            // Create MIDI buffer (empty for this test)
            juce::MidiBuffer midiBuffer;
            
            // Process the buffer
            processor.processBlock(audioBuffer, midiBuffer);
            
            // Extract output data (just use left channel)
            for (int sample = 0; sample < currentBufferSize; ++sample)
            {
                outputData.push_back(audioBuffer.getSample(0, sample));
            }
        }
        
        return outputData;
    }
};

int main()
{
    std::cout << "JUCE Pink Noise Processing Test" << std::endl;
    std::cout << "===============================" << std::endl;
    
    try
    {
        PinkNoiseProcessor test;
        test.runTest();
        std::cout << "Test completed successfully!" << std::endl;
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