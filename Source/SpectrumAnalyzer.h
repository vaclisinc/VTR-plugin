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
    
    // Public access to sample rate for frequency calculations
    double getSampleRate() const { return sampleRate; }
    
    // Configuration
    static constexpr int FFT_SIZE = 2048;
    static constexpr int FFT_ORDER = 11; // 2^11 = 2048
    static constexpr float UPDATE_RATE_HZ = 30.0f;
    static constexpr float PEAK_HOLD_TIME_SECONDS = 2.0f;
    
private:
    void performFFT(const juce::AudioBuffer<float>& buffer, std::vector<float>& spectrumData);
    void applyHannWindow(std::vector<float>& data);
    void updatePeakHold(std::vector<float>& spectrum, std::vector<float>& peakHold);
    
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};