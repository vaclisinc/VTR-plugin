#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <atomic>

namespace DynamicEQ {

/**
 * Scalable parameter management system
 * Handles smoothing and conversion for unlimited parameters
 */
class ParameterManager
{
public:
    ParameterManager() = default;
    ~ParameterManager() = default;
    
    // Core functionality
    void addParameter(const juce::String& parameterID, juce::AudioProcessorValueTreeState& apvts);
    void prepare(double sampleRate, double smoothingTimeMs = 30.0);
    void updateAllTargets();
    
    // Access methods
    juce::SmoothedValue<float>* getSmoothedValue(const juce::String& parameterID);
    float getCurrentValue(const juce::String& parameterID);
    
    // Direct access for performance-critical code
    std::vector<std::atomic<float>*> parameterPointers;
    std::vector<juce::String> parameterIDs;

private:
    std::vector<juce::SmoothedValue<float>> smoothedValues;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterManager)
};

} // namespace DynamicEQ