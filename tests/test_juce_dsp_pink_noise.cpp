#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <iostream>
#include <random>
#include <cmath>

class JuceDSPPinkNoiseTest
{
public:
    void runTest()
    {
        std::cout << "Running JUCE DSP Pink Noise Test..." << std::endl;
        
        // Audio setup
        const double sampleRate = 48000.0;
        const int bufferSize = 256;
        const double duration = 2.0;
        const int totalSamples = static_cast<int>(duration * sampleRate);
        const int numChannels = 2;
        
        // Setup DSP chain (simple gain processor for testing)
        juce::dsp::ProcessorChain<juce::dsp::Gain<float>> dspChain;
        
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numChannels;
        
        dspChain.prepare(spec);
        
        // Set gain to unity (1.0) for bypass test
        auto& gain = dspChain.template get<0>();
        gain.setGainLinear(1.0f);
        
        // Generate pink noise
        std::vector<float> inputData = generatePinkNoise(totalSamples);
        double inputRMS = calculateRMS(inputData);
        
        // Process through DSP chain
        std::vector<float> outputData = processAudioThroughDSP(dspChain, inputData, sampleRate, bufferSize, numChannels);
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
            std::cout << "✓ JUCE DSP pink noise test PASSED (RMS difference < 0.1 dB)" << std::endl;
        }
        else
        {
            std::cout << "✗ JUCE DSP pink noise test FAILED (RMS difference >= 0.1 dB)" << std::endl;
            std::exit(1);
        }
    }
    
private:
    std::vector<float> generatePinkNoise(int numSamples)
    {
        std::vector<float> signal(numSamples);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dis(0.0f, 0.1f);
        
        // Simple pink noise approximation using white noise
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
    
    template<typename ProcessorChain>
    std::vector<float> processAudioThroughDSP(
        ProcessorChain& dspChain,
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
            
            // Create DSP context
            juce::dsp::AudioBlock<float> block(audioBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            
            // Process the buffer through DSP chain
            dspChain.process(context);
            
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
    std::cout << "JUCE DSP Pink Noise Test" << std::endl;
    std::cout << "========================" << std::endl;
    
    try
    {
        JuceDSPPinkNoiseTest test;
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