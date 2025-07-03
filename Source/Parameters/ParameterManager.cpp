#include "ParameterManager.h"
#include <juce_core/juce_core.h>

namespace DynamicEQ {

void ParameterManager::addParameter(const juce::String& parameterID, 
                                   juce::AudioProcessorValueTreeState& apvts)
{
    parameterIDs.push_back(parameterID);
    parameterPointers.push_back(apvts.getRawParameterValue(parameterID));
    smoothedValues.push_back(juce::SmoothedValue<float>());
}

void ParameterManager::prepare(double sampleRate, double smoothingTimeMs)
{
    for (size_t i = 0; i < smoothedValues.size(); ++i)
    {
        smoothedValues[i].reset(sampleRate, smoothingTimeMs / 1000.0);
        float currentValue = parameterPointers[i]->load();
        
        // Convert dB to linear for gain parameters, or use direct value for others
        if (parameterIDs[i].contains("gain"))
        {
            currentValue = juce::Decibels::decibelsToGain(currentValue);
        }
        // For freq/Q parameters, use direct values
        
        smoothedValues[i].setCurrentAndTargetValue(currentValue);
    }
}

void ParameterManager::updateAllTargets()
{
    for (size_t i = 0; i < smoothedValues.size(); ++i)
    {
        float paramValue = parameterPointers[i]->load();
        
        // Safety check parameter value
        if (std::isfinite(paramValue))
        {
            float targetValue = paramValue;
            
            // Convert dB to linear for gain parameters
            if (parameterIDs[i].contains("gain"))
            {
                targetValue = juce::Decibels::decibelsToGain(paramValue);
            }
            
            // Additional safety check after conversion
            if (std::isfinite(targetValue) && targetValue >= 0.0f)
            {
                smoothedValues[i].setTargetValue(targetValue);
            }
        }
    }
}

juce::SmoothedValue<float>* ParameterManager::getSmoothedValue(const juce::String& parameterID)
{
    auto it = std::find(parameterIDs.begin(), parameterIDs.end(), parameterID);
    if (it != parameterIDs.end())
    {
        size_t index = static_cast<size_t>(std::distance(parameterIDs.begin(), it));
        return &smoothedValues[index];
    }
    return nullptr;
}

float ParameterManager::getCurrentValue(const juce::String& parameterID)
{
    auto* smoothed = getSmoothedValue(parameterID);
    return smoothed ? smoothed->getCurrentValue() : 0.0f;
}

} // namespace DynamicEQ