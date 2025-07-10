#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

class SpectrumAnalyzer
{
public:
    SpectrumAnalyzer();
    
    void prepare(double sampleRate, int samplesPerBlock);
    void processBlock(const juce::AudioBuffer<float>& inputBuffer, const juce::AudioBuffer<float>& outputBuffer);
    
    // Get spectrum data for visualization
    std::vector<float> getInputSpectrum() const;
    std::vector<float> getOutputSpectrum() const;
    
    // VTR3 Feature extraction methods
    std::vector<float> extractFeatures(const std::vector<float>& audioData, double sampleRate);
    std::vector<float> extractMFCC(const std::vector<float>& powerSpectrum, double sampleRate);
    float extractSpectralCentroid(const std::vector<float>& powerSpectrum, double sampleRate);
    float extractSpectralBandwidth(const std::vector<float>& powerSpectrum, double sampleRate);
    float extractSpectralRolloff(const std::vector<float>& powerSpectrum, double sampleRate);
    float extractRMSEnergy(const std::vector<float>& audioData);
    std::vector<float> computeMelFilterbank(const std::vector<float>& powerSpectrum, double sampleRate);
    std::vector<float> computeDCT(const std::vector<float>& melEnergies);
    
    // VTR3 Feature storage and retrieval
    std::vector<float> getLatestFeatures() const;
    bool hasNewFeatures() const;
    void enableFeatureExtraction(bool enable);
    void setFeatureUpdateRate(float rateHz);
    
    // Public access to sample rate for frequency calculations
    double getSampleRate() const { return sampleRate; }
    
    // Configuration
    static constexpr int FFT_SIZE = 2048;
    static constexpr int FFT_ORDER = 11; // 2^11 = 2048
    static constexpr float UPDATE_RATE_HZ = 30.0f;
    static constexpr float PEAK_HOLD_TIME_SECONDS = 2.0f;
    
    // VTR3 Feature extraction constants
    static constexpr int NUM_MEL_FILTERS = 26;
    static constexpr int NUM_MFCC_COEFFS = 13;
    static constexpr int TOTAL_FEATURES = 17; // 13 MFCC + 1 spectral centroid + 1 RMS + 2 additional
    
private:
    void performFFT(const juce::AudioBuffer<float>& buffer, std::vector<float>& spectrumData);
    void applyHannWindow(std::vector<float>& data);
    void updatePeakHold(std::vector<float>& spectrum, std::vector<float>& peakHold);
    
    // VTR3 Helper methods
    float melScale(float frequency);
    float invMelScale(float mel);
    std::vector<float> computePowerSpectrum(const std::vector<float>& audioData);
    void extractAndStoreFeatures();
    
    // FFT processing
    juce::dsp::FFT fft;
    
    // Audio data buffers
    std::vector<float> inputFifo, outputFifo;
    std::vector<float> inputFFTData, outputFFTData;
    
    // Spectrum data with peak hold
    std::vector<float> inputSpectrum, outputSpectrum;
    std::vector<float> inputPeakHold, outputPeakHold;
    std::vector<float> inputPeakTimer, outputPeakTimer;
    
    // Thread safety
    mutable std::mutex spectrumMutex;
    
    // Configuration
    double sampleRate = 44100.0;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    
    // Peak hold decay
    float peakDecayRate = 0.0f;
    
    // VTR3 Feature extraction state
    std::vector<float> latestFeatures;
    std::atomic<bool> newFeaturesAvailable{false};
    std::atomic<bool> featureExtractionEnabled{false};
    float featureUpdateRateHz = 10.0f; // 10 Hz default
    int featureUpdateCounter = 0;
    int featureUpdateInterval = 1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};