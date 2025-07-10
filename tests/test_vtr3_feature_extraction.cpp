#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../Source/SpectrumAnalyzer.h"

// Test functions
void testFeatureVectorSize()
{
    std::cout << "Testing feature vector size..." << std::endl;
    
    SpectrumAnalyzer analyzer;
    std::vector<float> testAudio(1024, 0.1f); // 1024 samples of 0.1 amplitude
    double sampleRate = 44100.0;
    
    auto features = analyzer.extractFeatures(testAudio, sampleRate);
    
    // Should be 17 elements
    assert(features.size() == 17);
    std::cout << "✓ Feature vector has correct size (17)" << std::endl;
}

void testMFCCSize()
{
    std::cout << "Testing MFCC coefficient count..." << std::endl;
    
    SpectrumAnalyzer analyzer;
    std::vector<float> powerSpectrum(512, 1.0f); // Mock power spectrum
    double sampleRate = 44100.0;
    
    auto mfcc = analyzer.extractMFCC(powerSpectrum, sampleRate);
    
    // Should be 13 MFCC coefficients
    assert(mfcc.size() == 13);
    std::cout << "✓ MFCC vector has correct size (13)" << std::endl;
}

void testSpectralCentroidNonZero()
{
    std::cout << "Testing spectral centroid calculation..." << std::endl;
    
    SpectrumAnalyzer analyzer;
    std::vector<float> powerSpectrum(512, 0.0f);
    
    // Create a power spectrum with energy concentrated at 1kHz
    // Calculate which bin corresponds to 1kHz
    double sampleRate = 44100.0;
    int targetBin = (int)((1000.0 * 2.0 * 512) / sampleRate);
    
    // Set energy only in a few bins around 1kHz
    for (int i = targetBin - 2; i <= targetBin + 2; ++i)
    {
        if (i >= 0 && i < 512)
        {
            powerSpectrum[i] = 1.0f;
        }
    }
    
    float centroid = analyzer.extractSpectralCentroid(powerSpectrum, sampleRate);
    
    std::cout << "Centroid value: " << centroid << " Hz (target bin: " << targetBin << ")" << std::endl;
    
    // Should be around 1000 Hz, but allow broader range for implementation differences
    if (centroid < 500.0f || centroid > 2000.0f)
    {
        std::cout << "❌ Spectral centroid not in expected range (500-2000 Hz)" << std::endl;
        throw std::runtime_error("Spectral centroid calculation failed");
    }
    std::cout << "✓ Spectral centroid calculated correctly (~1000 Hz)" << std::endl;
}

void testRMSEnergyNonZero()
{
    std::cout << "Testing RMS energy calculation..." << std::endl;
    
    SpectrumAnalyzer analyzer;
    std::vector<float> audioData(1024, 0.5f); // 1024 samples of 0.5 amplitude
    
    float rms = analyzer.extractRMSEnergy(audioData);
    
    std::cout << "RMS value: " << rms << std::endl;
    
    // Should be approximately 0.5, not 0
    if (rms < 0.4f || rms > 0.6f)
    {
        std::cout << "❌ RMS energy not in expected range (0.4-0.6)" << std::endl;
        throw std::runtime_error("RMS energy calculation failed");
    }
    std::cout << "✓ RMS energy calculated correctly (~0.5)" << std::endl;
}

void testMelFilterbankSize()
{
    std::cout << "Testing mel filterbank size..." << std::endl;
    
    SpectrumAnalyzer analyzer;
    std::vector<float> powerSpectrum(512, 1.0f);
    double sampleRate = 44100.0;
    
    auto melEnergies = analyzer.computeMelFilterbank(powerSpectrum, sampleRate);
    
    // Should be 26 mel filter banks (standard)
    assert(melEnergies.size() == 26);
    std::cout << "✓ Mel filterbank has correct size (26)" << std::endl;
}

void testDCTSize()
{
    std::cout << "Testing DCT coefficient count..." << std::endl;
    
    SpectrumAnalyzer analyzer;
    std::vector<float> melEnergies(26, 1.0f);
    
    auto dctCoeffs = analyzer.computeDCT(melEnergies);
    
    // Should be 13 DCT coefficients
    assert(dctCoeffs.size() == 13);
    std::cout << "✓ DCT has correct size (13)" << std::endl;
}

void testFeatureVectorNonZero()
{
    std::cout << "Testing feature vector contains non-zero values..." << std::endl;
    
    SpectrumAnalyzer analyzer;
    std::vector<float> testAudio(1024, 0.1f);
    double sampleRate = 44100.0;
    
    auto features = analyzer.extractFeatures(testAudio, sampleRate);
    
    // At least some features should be non-zero
    bool hasNonZero = std::any_of(features.begin(), features.end(), [](float f) { return f != 0.0f; });
    
    std::cout << "Feature vector: [";
    for (size_t i = 0; i < features.size(); ++i)
    {
        std::cout << features[i];
        if (i < features.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    if (!hasNonZero)
    {
        std::cout << "❌ Feature extraction returns all zeros" << std::endl;
        throw std::runtime_error("Feature extraction not working");
    }
    std::cout << "✓ Feature vector contains non-zero values" << std::endl;
}

int main()
{
    std::cout << "Running VTR3 Feature Extraction Tests..." << std::endl;
    std::cout << "=========================================" << std::endl;
    
    try
    {
        testFeatureVectorSize();
        testMFCCSize();
        testSpectralCentroidNonZero();
        testRMSEnergyNonZero();
        testMelFilterbankSize();
        testDCTSize();
        testFeatureVectorNonZero();
        
        std::cout << "=========================================" << std::endl;
        std::cout << "All VTR3 tests PASSED!" << std::endl;
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