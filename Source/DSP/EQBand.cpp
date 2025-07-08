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

void EQBand::setupDynamics(const juce::String& thresholdID, const juce::String& ratioID,
                          const juce::String& attackID, const juce::String& releaseID,
                          const juce::String& kneeID, const juce::String& detectionID,
                          const juce::String& modeID, const juce::String& bypassID)
{
    thresholdParamID = thresholdID;
    ratioParamID = ratioID;
    attackParamID = attackID;
    releaseParamID = releaseID;
    kneeParamID = kneeID;
    detectionParamID = detectionID;
    modeParamID = modeID;
    bypassParamID = bypassID;
    
    dynamicsEnabled = true;
    cacheDynamicsParameterIndices();
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
    
    // Prepare dynamics processing if enabled
    if (dynamicsEnabled)
    {
        juce::dsp::ProcessSpec compressorSpec;
        compressorSpec.sampleRate = sampleRate;
        compressorSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        compressorSpec.numChannels = 2;
        
        compressor.prepare(compressorSpec);
        
        // Prepare temporary buffers for dynamics processing
        compressorBuffer.setSize(2, samplesPerBlock);
        keyInputBuffer.setSize(2, samplesPerBlock);
    }
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
    
    // Update filter parameters (clean and maintainable!)
    updateFilterParameters(frequency, gainDb, q, lastFilterType);
    
    // Update dynamics parameters if enabled
    if (dynamicsEnabled)
    {
        updateDynamicsParameters();
    }
}

void EQBand::processBuffer(juce::AudioBuffer<float>& buffer)
{
    // Get channel pointers
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : leftChannel;
    int numSamples = buffer.getNumSamples();
    
    // Process through the active filter type first
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
    
    // Apply dynamics processing after EQ if enabled
    if (dynamicsEnabled && !lastDynamicsBypass)
    {
        processDynamicsBlock(buffer);
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

float EQBand::getCurrentThreshold() const
{
    return lastThreshold;
}

float EQBand::getCurrentRatio() const
{
    return lastRatio;
}

float EQBand::getCurrentAttack() const
{
    return lastAttack;
}

float EQBand::getCurrentRelease() const
{
    return lastRelease;
}

float EQBand::getCurrentKnee() const
{
    return lastKnee;
}

DetectionType EQBand::getCurrentDetectionType() const
{
    return lastDetectionType;
}

DynamicsMode EQBand::getCurrentDynamicsMode() const
{
    return lastDynamicsMode;
}

bool EQBand::isDynamicsBypassed() const
{
    return lastDynamicsBypass;
}

float EQBand::getCurrentGainReduction() const
{
    return lastGainReduction;
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
    
    // Parameter indices cached successfully
}

void EQBand::cacheDynamicsParameterIndices()
{
    if (manager == nullptr || !dynamicsEnabled) return;
    
    // Cache dynamics parameter indices for efficiency
    for (size_t i = 0; i < manager->parameterIDs.size(); ++i)
    {
        if (manager->parameterIDs[i] == thresholdParamID) dynamicsParamIndices.threshold = static_cast<int>(i);
        if (manager->parameterIDs[i] == ratioParamID) dynamicsParamIndices.ratio = static_cast<int>(i);
        if (manager->parameterIDs[i] == attackParamID) dynamicsParamIndices.attack = static_cast<int>(i);
        if (manager->parameterIDs[i] == releaseParamID) dynamicsParamIndices.release = static_cast<int>(i);
        if (manager->parameterIDs[i] == kneeParamID) dynamicsParamIndices.knee = static_cast<int>(i);
        if (manager->parameterIDs[i] == detectionParamID) dynamicsParamIndices.detection = static_cast<int>(i);
        if (manager->parameterIDs[i] == modeParamID) dynamicsParamIndices.mode = static_cast<int>(i);
        if (manager->parameterIDs[i] == bypassParamID) dynamicsParamIndices.bypass = static_cast<int>(i);
    }
}

void EQBand::updateFilterParameters(float frequency, float gainDb, float q, FilterType filterType)
{
    // Clean, maintainable filter parameter updates using correct chowdsp API
    switch (filterType)
    {
        case FilterType::Bell:
            bellFilter.setCutoffFrequency(frequency);
            bellFilter.setGainDecibels(gainDb);
            bellFilter.setQValue(q);
            break;
            
        case FilterType::HighShelf:
            highShelfFilter.setCutoffFrequency(frequency);
            highShelfFilter.setGainDecibels(gainDb);
            highShelfFilter.setQValue(q);
            break;
            
        case FilterType::LowShelf:
            lowShelfFilter.setCutoffFrequency(frequency);
            lowShelfFilter.setGainDecibels(gainDb);
            lowShelfFilter.setQValue(q);
            break;
            
        case FilterType::HighPass:
            highPassFilter.setCutoffFrequency(frequency);
            highPassFilter.setQValue(1.0f / juce::MathConstants<float>::sqrt2); // Butterworth Q
            break;
            
        case FilterType::LowPass:
            lowPassFilter.setCutoffFrequency(frequency);
            lowPassFilter.setQValue(1.0f / juce::MathConstants<float>::sqrt2); // Butterworth Q
            break;
    }
}

void EQBand::updateDynamicsParameters()
{
    if (manager == nullptr || !dynamicsEnabled || !dynamicsParamIndices.isValid()) 
        return;
    
    // Get current dynamics parameter values
    float threshold = manager->parameterPointers[dynamicsParamIndices.threshold]->load();
    float ratio = manager->parameterPointers[dynamicsParamIndices.ratio]->load();
    float attack = manager->parameterPointers[dynamicsParamIndices.attack]->load();
    float release = manager->parameterPointers[dynamicsParamIndices.release]->load();
    float knee = manager->parameterPointers[dynamicsParamIndices.knee]->load();
    float detection = manager->parameterPointers[dynamicsParamIndices.detection]->load();
    float mode = manager->parameterPointers[dynamicsParamIndices.mode]->load();
    float bypass = manager->parameterPointers[dynamicsParamIndices.bypass]->load();
    
    // Safety checks
    if (!std::isfinite(threshold) || !std::isfinite(ratio) || !std::isfinite(attack) || !std::isfinite(release))
        return;
    
    // Clamp to safe ranges
    threshold = juce::jlimit(-60.0f, 0.0f, threshold);
    ratio = juce::jlimit(1.0f, 100.0f, ratio); // 100+ will be treated as limiting
    attack = juce::jlimit(0.1f, 300.0f, attack);
    release = juce::jlimit(1.0f, 3000.0f, release);
    knee = juce::jlimit(0.0f, 10.0f, knee);
    
    // Store for external access
    lastThreshold = threshold;
    lastRatio = ratio;
    lastAttack = attack;
    lastRelease = release;
    lastKnee = knee;
    lastDetectionType = static_cast<DetectionType>(juce::jlimit(0, 2, static_cast<int>(detection)));
    lastDynamicsMode = static_cast<DynamicsMode>(juce::jlimit(0, 3, static_cast<int>(mode)));
    lastDynamicsBypass = bypass >= 0.5f;
    
    // Update chowdsp compressor parameters
    compressor.params.thresholdDB = threshold;
    compressor.params.ratio = ratio;
    compressor.params.attackMs = attack;
    compressor.params.releaseMs = release;
    compressor.params.kneeDB = knee;
    
    // Set detection type (chowdsp supports Peak and RMS modes)
    if (lastDetectionType == DetectionType::Peak)
        compressor.levelDetector.setMode(0); // Peak detector
    else
        compressor.levelDetector.setMode(2); // RMS detector (Blend uses RMS as fallback)
    
    // Note: chowdsp compressor doesn't directly support our custom modes (Expansive, DeEsser, Gate)
    // For now, we'll use the standard compressive mode and handle custom modes separately if needed
}

void EQBand::processDynamicsBlock(juce::AudioBuffer<float>& buffer)
{
    if (!dynamicsEnabled || lastDynamicsBypass)
        return;
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Resize temporary buffers if needed
    compressorBuffer.setSize(numChannels, numSamples, false, false, true);
    keyInputBuffer.setSize(numChannels, numSamples, false, false, true);
    
    // Copy EQ-processed audio to compressor buffers
    for (int ch = 0; ch < numChannels; ++ch)
    {
        compressorBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        keyInputBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples); // Use same signal for key input (no sidechain)
    }
    
    // Process through chowdsp compressor
    chowdsp::BufferView<float> mainBufferView(compressorBuffer);
    chowdsp::BufferView<const float> keyInputView(keyInputBuffer);
    
    compressor.processBlock(mainBufferView, keyInputView);
    
    // Copy processed audio back to original buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.copyFrom(ch, 0, compressorBuffer, ch, 0, numSamples);
    }
    
    // Store gain reduction for metering (simplified - would need access to chowdsp internals for exact GR)
    // For now, we'll estimate based on level difference
    lastGainReduction = 0.0f; // TODO: Extract actual gain reduction from chowdsp compressor
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
    if (!valueTreeState || bandIndex < 0 || bandIndex >= static_cast<int>(bands.size()))
        return true;  // Default enabled when no state available
    
    juce::String paramID = "eq_enable_band" + juce::String(bandIndex);
    if (auto* param = dynamic_cast<juce::AudioParameterBool*>(valueTreeState->getParameter(paramID)))
    {
        return param->get();
    }
    return true;  // Default enabled
}

bool MultiBandEQ::isBandSoloed(int bandIndex) const
{
    if (!valueTreeState || bandIndex < 0 || bandIndex >= static_cast<int>(bands.size()))
        return false;  // Default not soloed when no state available
    
    juce::String paramID = "eq_solo_band" + juce::String(bandIndex);
    if (auto* param = dynamic_cast<juce::AudioParameterBool*>(valueTreeState->getParameter(paramID)))
    {
        return param->get();
    }
    return false;  // Default not soloed
}

} // namespace DynamicEQ