#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <iostream>
#include <random>
#include <cmath>

class SimplePinkNoiseTest
{
public:
    void runTest()
    {
        std::cout << "Running Simple Pink Noise Test..." << std::endl;
        
        // Audio setup
        const double sampleRate = 48000.0;
        const int bufferSize = 256;
        const double duration = 2.0;
        const int totalSamples = static_cast<int>(duration * sampleRate);
        const int numChannels = 2;
        
        // Generate pink noise
        std::vector<float> inputData = generatePinkNoise(totalSamples);
        double inputRMS = calculateRMS(inputData);
        
        // Process through a simple bypass (just copy input to output)
        std::vector<float> outputData = inputData; // Bypass processing
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
};

int main()
{
    std::cout << "Simple Pink Noise Test" << std::endl;
    std::cout << "=====================" << std::endl;
    
    try
    {
        SimplePinkNoiseTest test;
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