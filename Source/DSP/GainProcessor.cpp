#include "GainProcessor.h"
#include <cmath>

namespace DynamicEQ {

void GainProcessor::setup(const juce::String& paramID, ParameterManager* paramManager)
{
    parameterID = paramID;
    manager = paramManager;
}

void GainProcessor::processBuffer(juce::AudioBuffer<float>& buffer)
{
    auto* smoothedGain = manager->getSmoothedValue(parameterID);
    if (smoothedGain == nullptr) return;
    
    if (smoothedGain->isSmoothing())
    {
        // Apply smoothed gain when parameters are changing
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float gainValue = smoothedGain->getNextValue();
                
                // Safety checks to prevent crashes
                if (std::isfinite(gainValue) && gainValue > 0.0f)
                {
                    float result = channelData[sample] * gainValue;
                    
                    // Smart limiting - only engage when signal is actually hot
                    if (std::abs(result) > 0.95f)
                    {
                        // Apply soft limiting only when needed
                        channelData[sample] = std::tanh(result * 0.85f);
                    }
                    else
                    {
                        // Pass through unmodified for normal levels
                        channelData[sample] = result;
                    }
                }
            }
            
            // Reset for next channel
            smoothedGain->skip(-buffer.getNumSamples());
        }
        
        // Skip for next buffer
        smoothedGain->skip(buffer.getNumSamples());
    }
    else
    {
        // Apply constant gain when not smoothing - more efficient
        float constantGain = smoothedGain->getCurrentValue();
        
        // Safety checks for constant gain
        if (constantGain != 1.0f && std::isfinite(constantGain) && constantGain > 0.0f)
        {
            // Check if this gain level will cause clipping
            bool needsLimiting = (constantGain > 3.0f);  // Roughly +9.5dB
            
            if (needsLimiting)
            {
                // Apply gain with smart limiting for high gain settings
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    {
                        float result = channelData[sample] * constantGain;
                        
                        if (std::abs(result) > 0.95f)
                        {
                            channelData[sample] = std::tanh(result * 0.85f);
                        }
                        else
                        {
                            channelData[sample] = result;
                        }
                    }
                }
            }
            else
            {
                // Normal gain - no limiting needed
                buffer.applyGain(constantGain);
            }
        }
    }
}

float GainProcessor::getCurrentGain() const
{
    auto* smoothedGain = manager->getSmoothedValue(parameterID);
    return smoothedGain ? smoothedGain->getCurrentValue() : 1.0f;
}

bool GainProcessor::isSmoothing() const
{
    auto* smoothedGain = manager->getSmoothedValue(parameterID);
    return smoothedGain ? smoothedGain->isSmoothing() : false;
}

} // namespace DynamicEQ