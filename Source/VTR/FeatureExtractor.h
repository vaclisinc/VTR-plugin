#pragma once

#include <vector>
#include <memory>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#ifdef HAVE_LIBXTRACT
#include <xtract/libxtract.h>
#endif

#include "PythonFeatureExtractor.h"

/**
 * Independent feature extraction class for VTR audio processing
 * Supports both JUCE and LibXtract backends for comparison and retraining
 */
class FeatureExtractor
{
public:
    enum class Backend
    {
        JUCE_BASED,
        LIBXTRACT_BASED,
        PYTHON_LIBROSA  // Direct Python librosa integration
    };
    
    FeatureExtractor();
    ~FeatureExtractor();
    
    // Initialize with processing parameters
    void initialize(double sampleRate, int fftSize = 512, Backend backend = Backend::JUCE_BASED);
    
    // Set backend for feature extraction
    void setBackend(Backend backend);
    Backend getBackend() const { return currentBackend_; }
    
    // Extract complete feature vector (17 dimensions)
    // Order: [spectral_centroid, spectral_bandwidth, spectral_rolloff, mfcc_1...mfcc_13, rms_energy]
    std::vector<float> extractFeatures(const std::vector<float>& audioData);
    
    // Individual feature extraction methods
    std::vector<float> extractMFCC(const std::vector<float>& audioData, int numCoeffs = 13);
    float extractSpectralCentroid(const std::vector<float>& audioData);
    float extractSpectralBandwidth(const std::vector<float>& audioData);
    float extractSpectralRolloff(const std::vector<float>& audioData, float rolloffPercent = 0.85f);
    float extractRMSEnergy(const std::vector<float>& audioData);
    
    // Utility method for loading audio files
    static std::vector<float> loadAudioFile(const std::string& filePath, double targetSampleRate = 44100.0);
    
private:
    // Core processing methods
    std::vector<float> computePowerSpectrum(const std::vector<float>& audioData);
    void applyHannWindow(std::vector<float>& data);
    
    // JUCE-based implementations
    std::vector<float> extractMFCC_JUCE(const std::vector<float>& powerSpectrum);
    float extractSpectralCentroid_JUCE(const std::vector<float>& powerSpectrum);
    float extractSpectralBandwidth_JUCE(const std::vector<float>& powerSpectrum);
    float extractSpectralRolloff_JUCE(const std::vector<float>& powerSpectrum, float rolloffPercent);
    
    // JUCE mel filterbank and DCT
    std::vector<float> computeMelFilterbank_JUCE(const std::vector<float>& powerSpectrum);
    std::vector<float> computeDCT_JUCE(const std::vector<float>& melEnergies);
    float melScale(float frequency);
    float invMelScale(float mel);
    
#ifdef HAVE_LIBXTRACT
    // LibXtract initialization and implementations
    void initializeLibXtract();
    std::vector<float> extractMFCC_LibXtract(const std::vector<float>& audioData, int numCoeffs);
    float extractSpectralCentroid_LibXtract(const std::vector<float>& audioData);
    float extractSpectralBandwidth_LibXtract(const std::vector<float>& audioData);
    float extractSpectralRolloff_LibXtract(const std::vector<float>& audioData, float rolloffPercent);
#endif
    
    // Processing parameters
    double sampleRate_;
    int fftSize_;
    bool isInitialized_;
    Backend currentBackend_;
    
    // FFT processing
    std::unique_ptr<juce::dsp::FFT> fft_;
    
#ifdef HAVE_LIBXTRACT
    // LibXtract state
    double* window_;
    std::vector<std::vector<double>> melFilters_;
    std::vector<double*> melFilterPtrs_;
    std::vector<double> workBufferDouble_;
    std::vector<double> melEnergies_;
    std::vector<double> allMFCCCoeffs_;
    bool libxtractInitialized_;
#endif
    
    // Python librosa backend
    std::unique_ptr<PythonFeatureExtractor> pythonExtractor_;
    
    // Working buffers
    std::vector<float> workBuffer_;
    std::vector<std::complex<float>> fftBuffer_;
    
    // Constants
    static constexpr int NUM_MFCC_COEFFS = 13;
    static constexpr int NUM_MEL_FILTERS = 26;
    static constexpr int FEATURE_VECTOR_SIZE = 17;
};