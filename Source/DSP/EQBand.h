#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_eq/chowdsp_eq.h>
#include "../Parameters/ParameterManager.h"

namespace DynamicEQ {

/**
 * Filter types supported by the EQ band
 */
enum class FilterType
{
    Bell = 0,
    HighShelf,
    LowShelf,
    HighPass,
    LowPass
};

/**
 * Simple stereo filter wrappers for each filter type
 * Clean, type-safe, and maintainable
 */
class StereoBellFilter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) { filterL.prepare(spec); filterR.prepare(spec); }
    void reset() { filterL.reset(); filterR.reset(); }
    void setCutoffFrequency(float freq) { filterL.setCutoffFrequency(freq); filterR.setCutoffFrequency(freq); }
    void setGainDecibels(float gainDB) { filterL.setGainDecibels(gainDB); filterR.setGainDecibels(gainDB); }
    void setQValue(float q) { filterL.setQValue(q); filterR.setQValue(q); }
    void processStereo(float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i) {
            left[i] = filterL.processSample(0, left[i]);
            if (right != left) right[i] = filterR.processSample(0, right[i]);
        }
    }
private:
    chowdsp::SVFBell<float> filterL, filterR;
};

class StereoHighShelfFilter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) { filterL.prepare(spec); filterR.prepare(spec); }
    void reset() { filterL.reset(); filterR.reset(); }
    void setCutoffFrequency(float freq) { filterL.setCutoffFrequency(freq); filterR.setCutoffFrequency(freq); }
    void setGainDecibels(float gainDB) { filterL.setGainDecibels(gainDB); filterR.setGainDecibels(gainDB); }
    void setQValue(float q) { filterL.setQValue(q); filterR.setQValue(q); }
    void processStereo(float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i) {
            left[i] = filterL.processSample(0, left[i]);
            if (right != left) right[i] = filterR.processSample(0, right[i]);
        }
    }
private:
    chowdsp::SVFHighShelf<float> filterL, filterR;
};

class StereoLowShelfFilter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) { filterL.prepare(spec); filterR.prepare(spec); }
    void reset() { filterL.reset(); filterR.reset(); }
    void setCutoffFrequency(float freq) { filterL.setCutoffFrequency(freq); filterR.setCutoffFrequency(freq); }
    void setGainDecibels(float gainDB) { filterL.setGainDecibels(gainDB); filterR.setGainDecibels(gainDB); }
    void setQValue(float q) { filterL.setQValue(q); filterR.setQValue(q); }
    void processStereo(float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i) {
            left[i] = filterL.processSample(0, left[i]);
            if (right != left) right[i] = filterR.processSample(0, right[i]);
        }
    }
private:
    chowdsp::SVFLowShelf<float> filterL, filterR;
};

class StereoHighPassFilter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) { filterL.prepare(spec); filterR.prepare(spec); }
    void reset() { filterL.reset(); filterR.reset(); }
    void setCutoffFrequency(float freq) { filterL.setCutoffFrequency(freq); filterR.setCutoffFrequency(freq); }
    void setQValue(float q) { filterL.setQValue(q); filterR.setQValue(q); }
    void processStereo(float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i) {
            left[i] = filterL.processSample(0, left[i]);
            if (right != left) right[i] = filterR.processSample(0, right[i]);
        }
    }
private:
    chowdsp::SVFHighpass<float> filterL, filterR;
};

class StereoLowPassFilter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) { filterL.prepare(spec); filterR.prepare(spec); }
    void reset() { filterL.reset(); filterR.reset(); }
    void setCutoffFrequency(float freq) { filterL.setCutoffFrequency(freq); filterR.setCutoffFrequency(freq); }
    void setQValue(float q) { filterL.setQValue(q); filterR.setQValue(q); }
    void processStereo(float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i) {
            left[i] = filterL.processSample(0, left[i]);
            if (right != left) right[i] = filterR.processSample(0, right[i]);
        }
    }
private:
    chowdsp::SVFLowpass<float> filterL, filterR;
};


/**
 * Professional EQ Band implementation
 * Clean, maintainable, and ready for multi-band expansion
 */
class EQBand
{
public:
    EQBand() = default;
    ~EQBand() = default;
    
    // Setup and configuration
    void setup(const juce::String& freqID, const juce::String& gainID, 
               const juce::String& qID, const juce::String& typeID, ParameterManager* paramManager);
    void prepare(double sampleRate, int samplesPerBlock);
    
    // Real-time processing
    void updateParameters();
    void processBuffer(juce::AudioBuffer<float>& buffer);
    
    // Parameter access
    float getCurrentFrequency() const;
    float getCurrentGain() const;
    float getCurrentQ() const;
    FilterType getCurrentFilterType() const;
    
    // Multi-band expansion support
    void setBandIndex(int bandIndex) { currentBandIndex = bandIndex; }
    int getBandIndex() const { return currentBandIndex; }

private:
    // Clean DSP implementation using typed stereo filter wrappers
    StereoBellFilter bellFilter;
    StereoHighShelfFilter highShelfFilter;
    StereoLowShelfFilter lowShelfFilter;
    StereoHighPassFilter highPassFilter;
    StereoLowPassFilter lowPassFilter;
    double currentSampleRate = 44100.0;
    
    // Cached parameter indices for efficiency
    struct ParameterIndices
    {
        int frequency = -1;
        int gain = -1;
        int q = -1;
        int type = -1;
        bool isValid() const { return frequency >= 0 && gain >= 0 && q >= 0 && type >= 0; }
    } paramIndices;
    
    // Parameter management
    juce::String freqParamID, gainParamID, qParamID, typeParamID;
    ParameterManager* manager = nullptr;
    int currentBandIndex = 0;  // For multi-band support
    
    // Current values for external access
    float lastFrequency = 1000.0f;
    float lastGainDb = 0.0f;
    float lastQ = 1.0f;
    FilterType lastFilterType = FilterType::Bell;
    
    // Private helper methods
    void cacheParameterIndices();
    void updateFilterParameters(float frequency, float gainDb, float q, FilterType filterType);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQBand)
};

/**
 * Multi-band EQ foundation for future expansion
 * Ready to support unlimited bands with crossover filtering
 */
class MultiBandEQ
{
public:
    MultiBandEQ() = default;
    
    // Band management
    void setNumBands(int numBands);
    int getNumBands() const { return static_cast<int>(bands.size()); }
    
    // Band access
    EQBand* getBand(int bandIndex);
    const EQBand* getBand(int bandIndex) const;
    
    // Processing
    void prepare(double sampleRate, int samplesPerBlock);
    void processBuffer(juce::AudioBuffer<float>& buffer);
    
private:
    std::vector<std::unique_ptr<EQBand>> bands;
    double currentSampleRate = 44100.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiBandEQ)
};

} // namespace DynamicEQ