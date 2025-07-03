#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "../Parameters/ParameterManager.h"

namespace DynamicEQ {

/**
 * High-quality gain processor with smart limiting
 * Features smooth parameter changes and automatic protection
 */
class GainProcessor
{
public:
    GainProcessor() = default;
    ~GainProcessor() = default;
    
    // Setup and configuration
    void setup(const juce::String& paramID, ParameterManager* paramManager);
    
    // Real-time processing
    void processBuffer(juce::AudioBuffer<float>& buffer);
    
    // Status queries
    float getCurrentGain() const;
    bool isSmoothing() const;

private:
    // Parameter management
    juce::String parameterID;
    ParameterManager* manager = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainProcessor)
};

} // namespace DynamicEQ