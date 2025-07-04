#include "EQBand.h"

namespace DynamicEQ {

void EQBand::setup(const juce::String& freqID, const juce::String& gainID, 
                   const juce::String& qID, const juce::String& typeID, ParameterManager* paramManager)
{
    freqParamID = freqID;
    gainParamID = gainID;
    qParamID = qID;
    typeParamID = typeID;
    manager = paramManager;
    
    // Cache parameter indices for efficiency
    cacheParameterIndices();
}

void EQBand::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // Create ProcessSpec for chowdsp filters
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;
    
    // Prepare all StereoFilter instances (much cleaner!)
    bellFilter.prepare(spec);
    highShelfFilter.prepare(spec);
    lowShelfFilter.prepare(spec);
    highPassFilter.prepare(spec);
    lowPassFilter.prepare(spec);
    
    // Reset all internal states
    bellFilter.reset();
    highShelfFilter.reset();
    lowShelfFilter.reset();
    highPassFilter.reset();
    lowPassFilter.reset();
}

void EQBand::updateParameters()
{
    if (manager == nullptr || !paramIndices.isValid()) 
        return;
    
    // Get current parameter values using cached indices (efficient!)
    float frequency = manager->parameterPointers[paramIndices.frequency]->load();
    float gainDb = manager->parameterPointers[paramIndices.gain]->load();    
    float q = manager->parameterPointers[paramIndices.q]->load();
    float rawTypeValue = manager->parameterPointers[paramIndices.type]->load();
    int filterTypeInt = static_cast<int>(rawTypeValue);
    
    DBG("EQBand::updateParameters - rawTypeValue=" << rawTypeValue << ", filterTypeInt=" << filterTypeInt);
    
    // Safety checks
    if (!std::isfinite(frequency) || !std::isfinite(gainDb) || !std::isfinite(q))
        return;
        
    // Clamp to safe ranges
    frequency = juce::jlimit(20.0f, 20000.0f, frequency);
    gainDb = juce::jlimit(-12.0f, 12.0f, gainDb);
    q = juce::jlimit(0.1f, 10.0f, q);
    filterTypeInt = juce::jlimit(0, 4, filterTypeInt);
    
    // Store for external access
    lastFrequency = frequency;
    lastGainDb = gainDb;
    lastQ = q;
    lastFilterType = static_cast<FilterType>(filterTypeInt);
    
    DBG("EQBand::updateParameters - Setting filter type to: " << static_cast<int>(lastFilterType));
    
    // Update filter parameters (clean and maintainable!)
    updateFilterParameters(frequency, gainDb, q, lastFilterType);
}

void EQBand::processBuffer(juce::AudioBuffer<float>& buffer)
{
    // Process through appropriate StereoFilter (clean and efficient!)
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : leftChannel;
    int numSamples = buffer.getNumSamples();
    
    // Process through the active filter type
    switch (lastFilterType)
    {
        case FilterType::Bell:
            bellFilter.processStereo(leftChannel, rightChannel, numSamples);
            break;
        case FilterType::HighShelf:
            highShelfFilter.processStereo(leftChannel, rightChannel, numSamples);
            break;
        case FilterType::LowShelf:
            lowShelfFilter.processStereo(leftChannel, rightChannel, numSamples);
            break;
        case FilterType::HighPass:
            highPassFilter.processStereo(leftChannel, rightChannel, numSamples);
            break;
        case FilterType::LowPass:
            lowPassFilter.processStereo(leftChannel, rightChannel, numSamples);
            break;
    }
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

FilterType EQBand::getCurrentFilterType() const
{
    return lastFilterType;
}

void EQBand::cacheParameterIndices()
{
    if (manager == nullptr) return;
    
    // Cache parameter indices to eliminate string lookups in real-time processing
    for (size_t i = 0; i < manager->parameterIDs.size(); ++i)
    {
        if (manager->parameterIDs[i] == freqParamID) paramIndices.frequency = static_cast<int>(i);
        if (manager->parameterIDs[i] == gainParamID) paramIndices.gain = static_cast<int>(i);
        if (manager->parameterIDs[i] == qParamID) paramIndices.q = static_cast<int>(i);
        if (manager->parameterIDs[i] == typeParamID) paramIndices.type = static_cast<int>(i);
    }
    
    DBG("EQBand parameter indices cached: freq=" << paramIndices.frequency 
        << ", gain=" << paramIndices.gain << ", q=" << paramIndices.q 
        << ", type=" << paramIndices.type);
}

void EQBand::updateFilterParameters(float frequency, float gainDb, float q, FilterType filterType)
{
    // Clean, maintainable filter parameter updates using correct chowdsp API
    switch (filterType)
    {
        case FilterType::Bell:
            DBG("Updating Bell filter: freq=" << frequency << ", gain=" << gainDb << ", Q=" << q);
            bellFilter.setCutoffFrequency(frequency);
            bellFilter.setGainDecibels(gainDb);
            bellFilter.setQValue(q);
            break;
            
        case FilterType::HighShelf:
            DBG("Updating HighShelf filter: freq=" << frequency << ", gain=" << gainDb << ", Q=" << q);
            highShelfFilter.setCutoffFrequency(frequency);
            highShelfFilter.setGainDecibels(gainDb);
            highShelfFilter.setQValue(q);
            break;
            
        case FilterType::LowShelf:
            DBG("Updating LowShelf filter: freq=" << frequency << ", gain=" << gainDb << ", Q=" << q);
            lowShelfFilter.setCutoffFrequency(frequency);
            lowShelfFilter.setGainDecibels(gainDb);
            lowShelfFilter.setQValue(q);
            break;
            
        case FilterType::HighPass:
            DBG("Updating HighPass filter: freq=" << frequency);
            highPassFilter.setCutoffFrequency(frequency);
            highPassFilter.setQValue(1.0f / juce::MathConstants<float>::sqrt2); // Butterworth Q
            break;
            
        case FilterType::LowPass:
            DBG("Updating LowPass filter: freq=" << frequency);
            lowPassFilter.setCutoffFrequency(frequency);
            lowPassFilter.setQValue(1.0f / juce::MathConstants<float>::sqrt2); // Butterworth Q
            break;
    }
}

// Multi-band EQ implementation (foundation for future expansion)
void MultiBandEQ::setNumBands(int numBands)
{
    bands.clear();
    bands.reserve(numBands);
    
    for (int i = 0; i < numBands; ++i)
    {
        auto band = std::make_unique<EQBand>();
        band->setBandIndex(i);
        bands.push_back(std::move(band));
    }
}

EQBand* MultiBandEQ::getBand(int bandIndex)
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
        return bands[bandIndex].get();
    return nullptr;
}

const EQBand* MultiBandEQ::getBand(int bandIndex) const
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
        return bands[bandIndex].get();
    return nullptr;
}

void MultiBandEQ::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    for (auto& band : bands)
    {
        if (band)
            band->prepare(sampleRate, samplesPerBlock);
    }
}

void MultiBandEQ::processBuffer(juce::AudioBuffer<float>& buffer)
{
    // Update all band parameters first
    for (auto& band : bands)
    {
        if (band)
            band->updateParameters();
    }
    
    // Check if any band is soloed
    bool anyBandSoloed = false;
    for (int i = 0; i < static_cast<int>(bands.size()); ++i)
    {
        if (bands[i] && isBandSoloed(i))
        {
            anyBandSoloed = true;
            break;
        }
    }
    
    // Process through bands based on enable/solo state
    if (anyBandSoloed)
    {
        // Only process soloed bands
        for (int i = 0; i < static_cast<int>(bands.size()); ++i)
        {
            if (bands[i] && isBandSoloed(i))
            {
                bands[i]->processBuffer(buffer);
            }
        }
    }
    else
    {
        // Process all enabled bands
        for (int i = 0; i < static_cast<int>(bands.size()); ++i)
        {
            if (bands[i] && isBandEnabled(i))
            {
                bands[i]->processBuffer(buffer);
            }
        }
    }
}

bool MultiBandEQ::isBandEnabled(int bandIndex) const
{
    if (!parameterManager || bandIndex < 0 || bandIndex >= static_cast<int>(bands.size()))
        return false;
    
    juce::String paramID = "eq_enable_band" + juce::String(bandIndex);
    auto* smoothedValue = parameterManager->getSmoothedValue(paramID);
    return smoothedValue ? (smoothedValue->getCurrentValue() > 0.5f) : true;  // Default enabled
}

bool MultiBandEQ::isBandSoloed(int bandIndex) const
{
    if (!parameterManager || bandIndex < 0 || bandIndex >= static_cast<int>(bands.size()))
        return false;
    
    juce::String paramID = "eq_solo_band" + juce::String(bandIndex);
    auto* smoothedValue = parameterManager->getSmoothedValue(paramID);
    return smoothedValue ? (smoothedValue->getCurrentValue() > 0.5f) : false;  // Default not soloed
}

} // namespace DynamicEQ