#include "EQBand.h"

namespace DynamicEQ {

void EQBand::setup(const juce::String& freqID, const juce::String& gainID, 
                   const juce::String& qID, ParameterManager* paramManager)
{
    freqParamID = freqID;
    gainParamID = gainID;
    qParamID = qID;
    manager = paramManager;
}

void EQBand::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2; // Stereo
    
    filter.prepare(spec);
}

void EQBand::updateParameters()
{
    if (manager == nullptr) return;
    
    // Find parameter indices (could be optimized with caching)
    int freqIndex = -1, gainIndex = -1, qIndex = -1;
    for (size_t i = 0; i < manager->parameterIDs.size(); ++i)
    {
        if (manager->parameterIDs[i] == freqParamID) freqIndex = static_cast<int>(i);
        if (manager->parameterIDs[i] == gainParamID) gainIndex = static_cast<int>(i);
        if (manager->parameterIDs[i] == qParamID) qIndex = static_cast<int>(i);
    }
    
    if (freqIndex < 0 || gainIndex < 0 || qIndex < 0) return;
    
    // Get current parameter values (non-smoothed for frequency and Q)
    float frequency = manager->parameterPointers[freqIndex]->load();
    float gainDb = manager->parameterPointers[gainIndex]->load();    
    float q = manager->parameterPointers[qIndex]->load();         
    
    // Safety checks
    if (!std::isfinite(frequency) || !std::isfinite(gainDb) || !std::isfinite(q))
        return;
        
    // Clamp to safe ranges
    frequency = juce::jlimit(20.0f, 20000.0f, frequency);
    gainDb = juce::jlimit(-12.0f, 12.0f, gainDb);
    q = juce::jlimit(0.1f, 10.0f, q);
    
    // Store for external access
    lastFrequency = frequency;
    lastGainDb = gainDb;
    lastQ = q;
    
    // Update filter parameters - create bell filter coefficients
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, frequency, q, juce::Decibels::decibelsToGain(gainDb));
    filter.coefficients = coeffs;
}

void EQBand::processBuffer(juce::AudioBuffer<float>& buffer)
{
    // Process through JUCE DSP IIR filter
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);
    filter.process(context);
}

float EQBand::getCurrentFrequency() const
{
    return lastFrequency;
}

float EQBand::getCurrentGain() const
{
    return lastGainDb;
}

float EQBand::getCurrentQ() const
{
    return lastQ;
}

} // namespace DynamicEQ