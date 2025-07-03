#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include "../Parameters/ParameterManager.h"

namespace DynamicEQ {

/**
 * Professional EQ Band implementation using JUCE DSP
 * Supports Bell/Peak filter with real-time parameter updates
 */
class EQBand
{
public:
    EQBand() = default;
    ~EQBand() = default;
    
    // Setup and configuration
    void setup(const juce::String& freqID, const juce::String& gainID, 
               const juce::String& qID, ParameterManager* paramManager);
    void prepare(double sampleRate, int samplesPerBlock);
    
    // Real-time processing
    void updateParameters();
    void processBuffer(juce::AudioBuffer<float>& buffer);
    
    // Parameter access
    float getCurrentFrequency() const;
    float getCurrentGain() const;
    float getCurrentQ() const;

private:
    // DSP components
    juce::dsp::IIR::Filter<float> filter;
    double currentSampleRate = 44100.0;
    
    // Parameter management
    juce::String freqParamID;
    juce::String gainParamID;
    juce::String qParamID;
    ParameterManager* manager = nullptr;
    
    // Current values for external access
    float lastFrequency = 1000.0f;
    float lastGainDb = 0.0f;
    float lastQ = 1.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQBand)
};

} // namespace DynamicEQ