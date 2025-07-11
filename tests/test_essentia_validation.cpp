#include <gtest/gtest.h>
#include "../Source/VTR/EssentiaFeatureExtractor.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <random>

class EssentiaValidationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        extractor = std::make_unique<EssentiaFeatureExtractor>();
        sampleRate = 44100.0;
        extractor->prepare(sampleRate);
    }
    
    void TearDown() override
    {
        extractor.reset();
    }
    
    std::vector<float> generateTestSignal(int numSamples, double frequency = 440.0)
    {
        std::vector<float> signal(numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            signal[i] = std::sin(2.0 * M_PI * frequency * i / sampleRate);
        }
        return signal;
    }
    
    std::vector<float> generateWhiteNoise(int numSamples, float amplitude = 0.1f)
    {
        std::vector<float> noise(numSamples);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dis(0.0f, amplitude);
        
        for (int i = 0; i < numSamples; ++i)
        {
            noise[i] = dis(gen);
        }
        return noise;
    }
    
    void saveFeaturesToFile(const std::vector<float>& features, const std::string& filename)
    {
        std::ofstream file(filename);
        for (size_t i = 0; i < features.size(); ++i)
        {
            file << "Feature " << i << ": " << features[i] << std::endl;
        }
        file.close();
    }
    
    std::unique_ptr<EssentiaFeatureExtractor> extractor;
    double sampleRate;
};

TEST_F(EssentiaValidationTest, ExtractFeaturesFromSineWave)
{
    // Generate a 1-second sine wave at 440 Hz
    int numSamples = static_cast<int>(sampleRate);
    std::vector<float> signal = generateTestSignal(numSamples, 440.0);
    
    // Extract features
    std::vector<float> features = extractor->extractFeatures(signal, sampleRate);
    
    // Verify we get the expected number of features
    EXPECT_EQ(features.size(), EssentiaFeatureExtractor::TOTAL_FEATURES);
    
    // Save features for manual inspection
    saveFeaturesToFile(features, "sine_wave_features.txt");
    
    // Basic sanity checks
    EXPECT_GT(features[0], 0.0f); // RMS should be positive
    EXPECT_GT(features[1], 0.0f); // Spectral centroid should be positive
    
    std::cout << "Sine wave (440 Hz) features:" << std::endl;
    for (size_t i = 0; i < features.size(); ++i)
    {
        std::cout << "  Feature " << i << ": " << features[i] << std::endl;
    }
}

TEST_F(EssentiaValidationTest, ExtractFeaturesFromWhiteNoise)
{
    // Generate white noise
    int numSamples = static_cast<int>(sampleRate);
    std::vector<float> noise = generateWhiteNoise(numSamples, 0.1f);
    
    // Extract features
    std::vector<float> features = extractor->extractFeatures(noise, sampleRate);
    
    // Verify we get the expected number of features
    EXPECT_EQ(features.size(), EssentiaFeatureExtractor::TOTAL_FEATURES);
    
    // Save features for manual inspection
    saveFeaturesToFile(features, "white_noise_features.txt");
    
    // Basic sanity checks
    EXPECT_GT(features[0], 0.0f); // RMS should be positive
    EXPECT_GT(features[1], 0.0f); // Spectral centroid should be positive
    
    std::cout << "White noise features:" << std::endl;
    for (size_t i = 0; i < features.size(); ++i)
    {
        std::cout << "  Feature " << i << ": " << features[i] << std::endl;
    }
}

TEST_F(EssentiaValidationTest, CompareMultipleFrequencies)
{
    std::vector<double> frequencies = {220.0, 440.0, 880.0, 1760.0};
    
    for (double freq : frequencies)
    {
        int numSamples = static_cast<int>(sampleRate);
        std::vector<float> signal = generateTestSignal(numSamples, freq);
        std::vector<float> features = extractor->extractFeatures(signal, sampleRate);
        
        EXPECT_EQ(features.size(), EssentiaFeatureExtractor::TOTAL_FEATURES);
        
        std::cout << "Frequency " << freq << " Hz features:" << std::endl;
        std::cout << "  RMS: " << features[0] << std::endl;
        std::cout << "  Spectral Centroid: " << features[1] << std::endl;
        std::cout << "  Spectral Bandwidth: " << features[2] << std::endl;
        std::cout << "  Spectral Rolloff: " << features[3] << std::endl;
        std::cout << "  MFCC[0]: " << features[4] << std::endl;
        std::cout << std::endl;
    }
}

TEST_F(EssentiaValidationTest, TestConsistency)
{
    // Generate the same signal twice and ensure features are identical
    int numSamples = static_cast<int>(sampleRate);
    std::vector<float> signal1 = generateTestSignal(numSamples, 440.0);
    std::vector<float> signal2 = generateTestSignal(numSamples, 440.0);
    
    std::vector<float> features1 = extractor->extractFeatures(signal1, sampleRate);
    std::vector<float> features2 = extractor->extractFeatures(signal2, sampleRate);
    
    EXPECT_EQ(features1.size(), features2.size());
    
    // Check that features are identical (within floating point tolerance)
    for (size_t i = 0; i < features1.size(); ++i)
    {
        EXPECT_NEAR(features1[i], features2[i], 1e-6f) 
            << "Feature " << i << " differs between identical signals";
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}