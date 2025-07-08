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
    
    // Add smoothable parameters to ParameterManager in correct order
    // First add all freq parameters, then all gain, then all q, then all type
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("eq_freq_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band) 
        parameterManager.addParameter("eq_gain_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("eq_q_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("eq_type_band" + juce::String(band), parameters);
    // Note: eq_enable and eq_solo are boolean parameters handled directly by ButtonAttachment
    
    // Add dynamics parameters to ParameterManager
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_threshold_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_ratio_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_attack_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_release_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_knee_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_detection_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_mode_band" + juce::String(band), parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
        parameterManager.addParameter("dyn_bypass_band" + juce::String(band), parameters);
    
    // Setup modular DSP components
    inputGain.setup("input_gain", &parameterManager);
    outputGain.setup("output_gain", &parameterManager);
    
    // Setup multi-band EQ system
    multiBandEQ.setNumBands(DynamicEQ::CURRENT_BANDS);
    multiBandEQ.setParameterManager(&parameterManager);
    multiBandEQ.setValueTreeState(&parameters);
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
    {
        if (auto* eqBand = multiBandEQ.getBand(band))
        {
            eqBand->setup("eq_freq_band" + juce::String(band), 
                         "eq_gain_band" + juce::String(band), 
                         "eq_q_band" + juce::String(band), 
                         "eq_type_band" + juce::String(band), 
                         &parameterManager);
            eqBand->setupDynamics("dyn_threshold_band" + juce::String(band),
                                 "dyn_ratio_band" + juce::String(band),
                                 "dyn_attack_band" + juce::String(band),
                                 "dyn_release_band" + juce::String(band),
                                 "dyn_knee_band" + juce::String(band),
                                 "dyn_detection_band" + juce::String(band),
                                 "dyn_mode_band" + juce::String(band),
                                 "dyn_bypass_band" + juce::String(band));
            eqBand->setBandIndex(band);
        }
    }
    
    // Parameter system initialized successfully
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

void VaclisDynamicEQAudioProcessor::addFilterTypeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                          const juce::String& parameterID,
                                                          const juce::String& parameterName,
                                                          float defaultValue)
{
    juce::StringArray filterTypeNames = {"Bell", "High Shelf", "Low Shelf", "High Pass", "Low Pass"};
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameterID,
        parameterName,
        filterTypeNames,
        static_cast<int>(defaultValue)
    ));
}

void VaclisDynamicEQAudioProcessor::addThresholdParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                         const juce::String& parameterID,
                                                         const juce::String& parameterName,
                                                         float defaultValue)
{
    auto thresholdRange = juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f);
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        thresholdRange,
        defaultValue,
        "dB",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
}

void VaclisDynamicEQAudioProcessor::addRatioParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                     const juce::String& parameterID,
                                                     const juce::String& parameterName,
                                                     float defaultValue)
{
    auto ratioRange = juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f);
    ratioRange.setSkewForCentre(4.0f);  // Logarithmic scaling centered on 4:1
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        ratioRange,
        defaultValue,
        ":1",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (value >= 20.0f)
                return juce::String("∞:1");
            else
                return juce::String(value, 1) + ":1";
        },
        [](const juce::String& text) { 
            if (text.contains("∞"))
                return 100.0f; // Will be converted to infinity in processor
            else
                return text.getFloatValue();
        }
    ));
}

void VaclisDynamicEQAudioProcessor::addAttackParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                      const juce::String& parameterID,
                                                      const juce::String& parameterName,
                                                      float defaultValue)
{
    auto attackRange = juce::NormalisableRange<float>(0.1f, 300.0f, 0.1f);
    attackRange.setSkewForCentre(10.0f);  // Logarithmic scaling
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        attackRange,
        defaultValue,
        "ms",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " ms"; },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
}

void VaclisDynamicEQAudioProcessor::addReleaseParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                       const juce::String& parameterID,
                                                       const juce::String& parameterName,
                                                       float defaultValue)
{
    auto releaseRange = juce::NormalisableRange<float>(1.0f, 3000.0f, 1.0f);
    releaseRange.setSkewForCentre(100.0f);  // Logarithmic scaling
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        releaseRange,
        defaultValue,
        "ms",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 0) + " ms"; },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
}

void VaclisDynamicEQAudioProcessor::addKneeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                    const juce::String& parameterID,
                                                    const juce::String& parameterName,
                                                    float defaultValue)
{
    auto kneeRange = juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f);
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        kneeRange,
        defaultValue,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1); },
        [](const juce::String& text) { return text.getFloatValue(); }
    ));
}

void VaclisDynamicEQAudioProcessor::addDetectionTypeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                             const juce::String& parameterID,
                                                             const juce::String& parameterName,
                                                             float defaultValue)
{
    juce::StringArray detectionTypeNames = {"Peak", "RMS", "Blend"};
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameterID,
        parameterName,
        detectionTypeNames,
        static_cast<int>(defaultValue)
    ));
}

void VaclisDynamicEQAudioProcessor::addDynamicsModeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                            const juce::String& parameterID,
                                                            const juce::String& parameterName,
                                                            float defaultValue)
{
    juce::StringArray dynamicsModeNames = {"Compressive", "Expansive", "De-esser", "Gate"};
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameterID,
        parameterName,
        dynamicsModeNames,
        static_cast<int>(defaultValue)
    ));
}

void VaclisDynamicEQAudioProcessor::addDynamicsBypassParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                              const juce::String& parameterID,
                                                              const juce::String& parameterName,
                                                              bool defaultValue)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        parameterID,
        parameterName,
        defaultValue
    ));
}

juce::AudioProcessorValueTreeState::ParameterLayout VaclisDynamicEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Global parameters
    addGainParameter(layout, "input_gain", "Input Gain", 0.0f);
    addGainParameter(layout, "output_gain", "Output Gain", 0.0f);
    
    // Multi-band EQ parameters with different default frequencies for each band
    const float defaultFreqs[DynamicEQ::CURRENT_BANDS] = {100.0f, 500.0f, 2000.0f, 8000.0f};  // LOW, LOW-MID, HIGH-MID, HIGH
    const juce::String bandNames[DynamicEQ::CURRENT_BANDS] = {"LOW", "LOW-MID", "HIGH-MID", "HIGH"};
    
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
    {
        juce::String bandSuffix = " Band " + juce::String(band);
        
        // EQ parameters for each band
        addFrequencyParameter(layout, "eq_freq_band" + juce::String(band), 
                             "EQ Frequency " + bandNames[band], defaultFreqs[band]);
        addGainParameter(layout, "eq_gain_band" + juce::String(band), 
                        "EQ Gain " + bandNames[band], 0.0f);
        addQParameter(layout, "eq_q_band" + juce::String(band), 
                     "EQ Q " + bandNames[band], 1.0f);
        addFilterTypeParameter(layout, "eq_type_band" + juce::String(band), 
                              "EQ Type " + bandNames[band], 0.0f);  // Default to Bell
        
        // Enable/Disable parameter
        layout.add(std::make_unique<juce::AudioParameterBool>(
            "eq_enable_band" + juce::String(band),
            "Enable " + bandNames[band],
            true  // Default enabled
        ));
        
        // Solo parameter
        layout.add(std::make_unique<juce::AudioParameterBool>(
            "eq_solo_band" + juce::String(band),
            "Solo " + bandNames[band],
            false  // Default not soloed
        ));
        
        // Dynamics parameters for each band
        addThresholdParameter(layout, "dyn_threshold_band" + juce::String(band),
                             "Dynamics Threshold " + bandNames[band], -20.0f);
        addRatioParameter(layout, "dyn_ratio_band" + juce::String(band),
                         "Dynamics Ratio " + bandNames[band], 4.0f);
        addAttackParameter(layout, "dyn_attack_band" + juce::String(band),
                          "Dynamics Attack " + bandNames[band], 1.0f);
        addReleaseParameter(layout, "dyn_release_band" + juce::String(band),
                           "Dynamics Release " + bandNames[band], 100.0f);
        addKneeParameter(layout, "dyn_knee_band" + juce::String(band),
                        "Dynamics Knee " + bandNames[band], 2.0f);
        addDetectionTypeParameter(layout, "dyn_detection_band" + juce::String(band),
                                 "Dynamics Detection " + bandNames[band], 0.0f);
        addDynamicsModeParameter(layout, "dyn_mode_band" + juce::String(band),
                                "Dynamics Mode " + bandNames[band], 0.0f);
        addDynamicsBypassParameter(layout, "dyn_bypass_band" + juce::String(band),
                                  "Dynamics Bypass " + bandNames[band], true);
    }

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

void VaclisDynamicEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare scalable parameter system
    parameterManager.prepare(sampleRate, 30.0);  // 30ms smoothing
    
    // Prepare modular DSP components
    multiBandEQ.prepare(sampleRate, samplesPerBlock);
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

    // Modular processing chain - clean and scalable
    updateParameterSmoothers();
    processInputGain(buffer);
    processEQ(buffer);
    processOutputGain(buffer);
}

void VaclisDynamicEQAudioProcessor::updateParameterSmoothers()
{
    // Update all parameters with one call - scales automatically!
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
    inputGain.processBuffer(buffer);
}

void VaclisDynamicEQAudioProcessor::processEQ(juce::AudioBuffer<float>& buffer)
{
    // Multi-band EQ processing with enable/disable and solo support
    multiBandEQ.processBuffer(buffer);
}

void VaclisDynamicEQAudioProcessor::processOutputGain(juce::AudioBuffer<float>& buffer)
{
    outputGain.processBuffer(buffer);
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