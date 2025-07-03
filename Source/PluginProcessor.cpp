#include "PluginProcessor.h"
#include "PluginEditor.h"

VaclisDynamicEQAudioProcessor::VaclisDynamicEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
       parameters (*this, nullptr, "Parameters", createParameterLayout())
{
    // Setup scalable parameter management
    parameterManager.addParameter("input_gain", parameters);
    parameterManager.addParameter("output_gain", parameters);
    
    // Setup gain processors with new system
    inputGain.setup("input_gain", &parameterManager);
    outputGain.setup("output_gain", &parameterManager);
}

VaclisDynamicEQAudioProcessor::~VaclisDynamicEQAudioProcessor()
{
}

void VaclisDynamicEQAudioProcessor::addGainParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                     const juce::String& parameterID,
                                                     const juce::String& parameterName,
                                                     float defaultValue)
{
    auto gainRange = juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f);
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        gainRange,
        defaultValue,
        "dB",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
}

void VaclisDynamicEQAudioProcessor::addFrequencyParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                          const juce::String& parameterID,
                                                          const juce::String& parameterName,
                                                          float defaultValue)
{
    auto freqRange = juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f);
    freqRange.setSkewForCentre(1000.0f);  // Logarithmic scaling centered on 1kHz
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        freqRange,
        defaultValue,
        "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (value >= 1000.0f)
                return juce::String(value / 1000.0f, 1) + " kHz";
            else
                return juce::String(value, 0) + " Hz";
        },
        [](const juce::String& text) { 
            if (text.contains("k"))
                return text.getFloatValue() * 1000.0f;
            else
                return text.getFloatValue(); 
        }
    ));
}

void VaclisDynamicEQAudioProcessor::addQParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                  const juce::String& parameterID,
                                                  const juce::String& parameterName,
                                                  float defaultValue)
{
    auto qRange = juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f);
    qRange.setSkewForCentre(1.0f);  // Logarithmic scaling centered on 1.0
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        qRange,
        defaultValue,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2); },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
}

juce::AudioProcessorValueTreeState::ParameterLayout VaclisDynamicEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Use reusable helper for all gain parameters
    addGainParameter(layout, "input_gain", "Input Gain", 0.0f);
    addGainParameter(layout, "output_gain", "Output Gain", 0.0f);
    
    // Future EQ bands will be easy to add:
    // addGainParameter(layout, "band1_gain", "Band 1 Gain", 0.0f);
    // addFrequencyParameter(layout, "band1_freq", "Band 1 Frequency", 1000.0f);
    // addQParameter(layout, "band1_q", "Band 1 Q", 1.0f);

    return layout;
}

const juce::String VaclisDynamicEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VaclisDynamicEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VaclisDynamicEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VaclisDynamicEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VaclisDynamicEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VaclisDynamicEQAudioProcessor::getNumPrograms()
{
    return 1;
}

int VaclisDynamicEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VaclisDynamicEQAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String VaclisDynamicEQAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void VaclisDynamicEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

// ParameterManager implementation
void VaclisDynamicEQAudioProcessor::ParameterManager::addParameter(const juce::String& parameterID, 
                                                                   juce::AudioProcessorValueTreeState& apvts)
{
    parameterIDs.push_back(parameterID);
    parameterPointers.push_back(apvts.getRawParameterValue(parameterID));
    smoothedValues.push_back(juce::SmoothedValue<float>());
}

void VaclisDynamicEQAudioProcessor::ParameterManager::prepare(double sampleRate, double smoothingTimeMs)
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
        
        smoothedValues[i].setCurrentAndTargetValue(currentValue);
    }
}

void VaclisDynamicEQAudioProcessor::ParameterManager::updateAllTargets()
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

juce::SmoothedValue<float>* VaclisDynamicEQAudioProcessor::ParameterManager::getSmoothedValue(const juce::String& parameterID)
{
    auto it = std::find(parameterIDs.begin(), parameterIDs.end(), parameterID);
    if (it != parameterIDs.end())
    {
        size_t index = static_cast<size_t>(std::distance(parameterIDs.begin(), it));
        return &smoothedValues[index];
    }
    return nullptr;
}

float VaclisDynamicEQAudioProcessor::ParameterManager::getCurrentValue(const juce::String& parameterID)
{
    auto* smoothed = getSmoothedValue(parameterID);
    return smoothed ? smoothed->getCurrentValue() : 0.0f;
}

// GainProcessor implementation with new system
void VaclisDynamicEQAudioProcessor::GainProcessor::setup(const juce::String& paramID, ParameterManager* paramManager)
{
    parameterID = paramID;
    manager = paramManager;
}

void VaclisDynamicEQAudioProcessor::GainProcessor::processBuffer(juce::AudioBuffer<float>& buffer)
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

void VaclisDynamicEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    
    // Prepare scalable parameter system
    parameterManager.prepare(sampleRate, 30.0);  // 30ms smoothing
}

void VaclisDynamicEQAudioProcessor::releaseResources()
{
    // Release any resources that were allocated in prepareToPlay()
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VaclisDynamicEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Support both mono and stereo configurations
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;

    // Input and output layouts should match
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
  #endif
}
#endif

void VaclisDynamicEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have input channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Modular processing chain - easy to expand
    updateParameterSmoothers();
    processInputGain(buffer);
    processEQ(buffer);           // Future EQ bands go here
    processOutputGain(buffer);
}

void VaclisDynamicEQAudioProcessor::updateParameterSmoothers()
{
    // Update all parameters with one call - scales to 50+ parameters automatically!
    parameterManager.updateAllTargets();
    
    // Optional: Log warning for dangerous levels (useful for debugging)
    if (checkForDangerousGainLevels())
    {
        // Could add DBG() warning here for development
    }
}

bool VaclisDynamicEQAudioProcessor::checkForDangerousGainLevels() const
{
    // Safe access to parameter values through ParameterManager
    if (parameterManager.parameterPointers.size() >= 2)
    {
        float inputGainDb = parameterManager.parameterPointers[0]->load();   // input_gain
        float outputGainDb = parameterManager.parameterPointers[1]->load();  // output_gain
        float totalGainDb = inputGainDb + outputGainDb;
        
        // Warn if total gain exceeds +18dB (roughly 8x amplification)
        return totalGainDb > 18.0f;
    }
    return false;
}

void VaclisDynamicEQAudioProcessor::processInputGain(juce::AudioBuffer<float>& buffer)
{
    inputGain.processBuffer(buffer);  // One line - much cleaner!
}

void VaclisDynamicEQAudioProcessor::processEQ(juce::AudioBuffer<float>& buffer)
{
    // Placeholder for future EQ processing
    juce::ignoreUnused(buffer);
    
    // Future implementation will be clean and scalable:
    // for (auto& band : eqBands) {
    //     band.updateParameters();  // Update freq, Q, gain smoothers via ParameterManager
    //     band.processBuffer(buffer);  // Apply filtering + gain
    // }
}

void VaclisDynamicEQAudioProcessor::processOutputGain(juce::AudioBuffer<float>& buffer)
{
    outputGain.processBuffer(buffer);  // One line - much cleaner!
}

bool VaclisDynamicEQAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* VaclisDynamicEQAudioProcessor::createEditor()
{
    return new VaclisDynamicEQAudioProcessorEditor (*this);
}

void VaclisDynamicEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void VaclisDynamicEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VaclisDynamicEQAudioProcessor();
}