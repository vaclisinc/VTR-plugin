#include "PluginProcessor.h"
#include "PluginEditor.h"

VaclisDynamicEQAudioProcessor::VaclisDynamicEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false)
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
    
    // Initialize VTR system
    vtrThreadPool = std::make_unique<juce::ThreadPool>(1); // Single thread for VTR processing
    
    // Load VTR model
    juce::File pluginDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
    juce::File modelFile = pluginDir.getChildFile("vtr-model/exported_model/model_weights.json");
    juce::File scalerFile = pluginDir.getChildFile("vtr-model/exported_model/scaler_params.json");
    
    // Try multiple locations for the model files
    if (!modelFile.existsAsFile())
    {
        // Try relative to plugin bundle
        juce::File bundleDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
        modelFile = bundleDir.getParentDirectory().getParentDirectory().getParentDirectory()
                            .getChildFile("vtr-model/exported_model/model_weights.json");
        scalerFile = bundleDir.getParentDirectory().getParentDirectory().getParentDirectory()
                             .getChildFile("vtr-model/exported_model/scaler_params.json");
    }
    
    // Try in user documents
    if (!modelFile.existsAsFile())
    {
        juce::File documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        modelFile = documentsDir.getChildFile("VTR-plugin/vtr-model/exported_model/model_weights.json");
        scalerFile = documentsDir.getChildFile("VTR-plugin/vtr-model/exported_model/scaler_params.json");
    }
    
    // Try absolute path as fallback
    if (!modelFile.existsAsFile())
    {
        modelFile = juce::File("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/model_weights.json");
        scalerFile = juce::File("/Users/vaclis./Documents/project/VTR-plugin/vtr-model/exported_model/scaler_params.json");
    }
    
    if (modelFile.existsAsFile() && scalerFile.existsAsFile())
    {
        if (loadVTRModel(modelFile.getFullPathName(), scalerFile.getFullPathName()))
        {
            juce::Logger::writeToLog("VTR model loaded successfully from: " + modelFile.getFullPathName());
        }
        else
        {
            juce::Logger::writeToLog("Failed to load VTR model");
        }
    }
    else
    {
        juce::Logger::writeToLog("VTR model files not found. Looked in: " + modelFile.getFullPathName());
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
    // VTR target frequencies: 80Hz, 240Hz, 2.5kHz, 4kHz, 10kHz
    const float defaultFreqs[DynamicEQ::CURRENT_BANDS] = {80.0f, 240.0f, 2500.0f, 4000.0f, 10000.0f};
    const juce::String bandNames[DynamicEQ::CURRENT_BANDS] = {"SUB", "LOW", "MID", "HIGH-MID", "HIGH"};
    
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
    
    // Sidechain parameter
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "sidechain_enable",
        "Sidechain Enable",
        false  // Default off
    ));

    return layout;
}

const juce::String VaclisDynamicEQAudioProcessor::getName() const
{
#ifdef JucePlugin_Name
    return JucePlugin_Name;
#else
    return "VTR-smartEQ";
#endif
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
    spectrumAnalyzer.prepare(sampleRate, samplesPerBlock);
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
    // Support stereo main bus
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
        
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // If sidechain bus exists, it should be stereo or disabled
    // Check if we have more than one input bus
    if (getBusCount(true) > 1)
    {
        auto sidechainLayout = layouts.getChannelSet(true, 1);
        if (sidechainLayout != juce::AudioChannelSet::stereo() && 
            sidechainLayout != juce::AudioChannelSet::disabled())
            return false;
    }

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

    // Capture input for spectrum analysis and level metering
    juce::AudioBuffer<float> inputBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        inputBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
    
    // Calculate input level (RMS)
    float inputRMS = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        inputRMS += buffer.getRMSLevel(channel, 0, buffer.getNumSamples());
    }
    inputRMS /= buffer.getNumChannels();
    inputLevel.store(inputRMS);

    // Check for sidechain input
    const juce::AudioBuffer<float>* sidechainBuffer = nullptr;
    bool sidechainEnabled = false;
    
    if (auto* sidechainParam = parameters.getRawParameterValue("sidechain_enable"))
    {
        sidechainEnabled = *sidechainParam > 0.5f;
    }
    
    // Get sidechain input if enabled and available
    if (sidechainEnabled && getBusCount(true) > 1)
    {
        if (auto* bus = getBus(true, 1))
        {
            if (bus->isEnabled())
            {
                auto sidechainBus = getBusBuffer(buffer, true, 1); // Sidechain is input bus 1
                if (sidechainBus.getNumChannels() > 0 && sidechainBus.getNumSamples() > 0)
                {
                    sidechainBuffer = &sidechainBus;
                }
            }
        }
    }
    
    // Modular processing chain - clean and scalable
    updateParameterSmoothers();
    processInputGain(buffer);
    processEQWithSidechain(buffer, sidechainBuffer);
    processOutputGain(buffer);
    
    // Calculate output level (RMS)
    float outputRMS = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        outputRMS += buffer.getRMSLevel(channel, 0, buffer.getNumSamples());
    }
    outputRMS /= buffer.getNumChannels();
    outputLevel.store(outputRMS);
    
    // Spectrum analysis with input and output
    spectrumAnalyzer.processBlock(inputBuffer, buffer);
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

void VaclisDynamicEQAudioProcessor::processEQWithSidechain(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>* sidechainBuffer)
{
    // Multi-band EQ processing with sidechain support
    multiBandEQ.processBuffer(buffer, sidechainBuffer);
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
    DBG("Creating editor - attempting to instantiate VaclisDynamicEQAudioProcessorEditor");
    try {
        auto* editor = new VaclisDynamicEQAudioProcessorEditor(*this);
        DBG("VaclisDynamicEQAudioProcessorEditor created successfully");
        return editor;
    } catch (const std::exception& e) {
        DBG("Exception creating editor: " << e.what());
        return nullptr;
    } catch (...) {
        DBG("Unknown exception creating editor");
        return nullptr;
    }
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

// VTR Implementation
bool VaclisDynamicEQAudioProcessor::loadVTRModel(const juce::String& modelPath, const juce::String& scalerPath)
{
    return vtrNetwork.loadModel(modelPath.toStdString(), scalerPath.toStdString());
}

void VaclisDynamicEQAudioProcessor::processReferenceAudioFile(const juce::File& audioFile)
{
    if (vtrProcessing.load())
    {
        juce::Logger::writeToLog("VTR processing already in progress");
        return;
    }
    
    // Create a job to process the audio file in the background
    class VTRProcessingJob : public juce::ThreadPoolJob
    {
    public:
        VTRProcessingJob(VaclisDynamicEQAudioProcessor* processor, juce::File file)
            : ThreadPoolJob("VTR Processing"), processor_(processor), audioFile_(file)
        {
        }
        
        JobStatus runJob() override
        {
            processor_->vtrProcessing.store(true);
            
            try
            {
                // Load the audio file
                juce::AudioFormatManager formatManager;
                formatManager.registerBasicFormats();
                
                std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile_));
                
                if (reader == nullptr)
                {
                    juce::Logger::writeToLog("Failed to load audio file: " + audioFile_.getFullPathName());
                    processor_->vtrProcessing.store(false);
                    return jobHasFinished;
                }
                
                // Read the audio data
                juce::AudioBuffer<float> audioBuffer(1, (int)reader->lengthInSamples);
                reader->read(&audioBuffer, 0, (int)reader->lengthInSamples, 0, true, false);
                
                // Resample to 44.1kHz if needed
                double sampleRate = reader->sampleRate;
                if (sampleRate != 44100.0)
                {
                    // Simple resampling for now - could be improved with better interpolation
                    int newLength = (int)((audioBuffer.getNumSamples() * 44100.0) / sampleRate);
                    juce::AudioBuffer<float> resampledBuffer(1, newLength);
                    
                    for (int i = 0; i < newLength; ++i)
                    {
                        float sourceIndex = (float)i * sampleRate / 44100.0f;
                        int index = (int)sourceIndex;
                        if (index < audioBuffer.getNumSamples())
                        {
                            resampledBuffer.setSample(0, i, audioBuffer.getSample(0, index));
                        }
                    }
                    
                    audioBuffer = std::move(resampledBuffer);
                    sampleRate = 44100.0;
                }
                
                // Convert to vector for feature extraction
                std::vector<float> audioData;
                audioData.resize(audioBuffer.getNumSamples());
                for (int i = 0; i < audioBuffer.getNumSamples(); ++i)
                {
                    audioData[i] = audioBuffer.getSample(0, i);
                }
                
                juce::Logger::writeToLog("VTR: Audio data size: " + juce::String(audioData.size()) + " samples (" + 
                                       juce::String(audioData.size() / sampleRate) + " seconds)");
                
                // Extract features using SpectrumAnalyzer
                juce::Logger::writeToLog("VTR: Starting feature extraction...");
                
                // Log current backend
                auto backend = processor_->spectrumAnalyzer.getFeatureExtractionBackend();
                juce::String backendName = "Unknown";
                switch (backend)
                {
                    case SpectrumAnalyzer::FeatureExtractionBackend::JUCE_BASED:
                        backendName = "JUCE";
                        break;
                    case SpectrumAnalyzer::FeatureExtractionBackend::PYTHON_LIBROSA:
                        backendName = "Python Librosa";
                        break;
                    case SpectrumAnalyzer::FeatureExtractionBackend::LIBXTRACT_BASED:
                        backendName = "LibXtract";
                        break;
                    default:
                        break;
                }
                juce::Logger::writeToLog("VTR: Using backend: " + backendName);
                
                auto features = processor_->spectrumAnalyzer.extractFeatures(audioData, sampleRate);
                juce::Logger::writeToLog("VTR: Feature extraction complete, got " + juce::String(features.size()) + " features");
                
                // Log first few features for debugging
                if (features.size() >= 5)
                {
                    juce::Logger::writeToLog("VTR: First 5 features: " + 
                        juce::String(features[0]) + ", " +
                        juce::String(features[1]) + ", " +
                        juce::String(features[2]) + ", " +
                        juce::String(features[3]) + ", " +
                        juce::String(features[4]));
                }
                
                // Run VTR prediction
                auto predictions = processor_->vtrNetwork.predict(features);
                
                // Log predictions
                if (predictions.size() >= 5)
                {
                    juce::Logger::writeToLog("VTR Predictions: " +
                        juce::String(predictions[0]) + " dB, " +
                        juce::String(predictions[1]) + " dB, " +
                        juce::String(predictions[2]) + " dB, " +
                        juce::String(predictions[3]) + " dB, " +
                        juce::String(predictions[4]) + " dB");
                }
                
                // Apply predictions to EQ parameters
                processor_->applyVTRPredictions(predictions);
                
                juce::Logger::writeToLog("VTR processing completed successfully");
            }
            catch (const std::exception& e)
            {
                juce::Logger::writeToLog("VTR processing failed: " + juce::String(e.what()));
            }
            
            processor_->vtrProcessing.store(false);
            return jobHasFinished;
        }
        
    private:
        VaclisDynamicEQAudioProcessor* processor_;
        juce::File audioFile_;
    };
    
    // Submit the job to the thread pool
    vtrThreadPool->addJob(new VTRProcessingJob(this, audioFile), true);
}

void VaclisDynamicEQAudioProcessor::applyVTRPredictions(const std::vector<float>& predictions)
{
    if (predictions.size() != 5)
    {
        juce::Logger::writeToLog("VTR predictions size mismatch: expected 5, got " + juce::String(static_cast<int>(predictions.size())));
        return;
    }
    
    // Apply predictions to the 5 EQ bands
    // VTR targets: 80Hz, 240Hz, 2.5kHz, 4kHz, 10kHz
    const std::vector<float> targetFreqs = {80.0f, 240.0f, 2500.0f, 4000.0f, 10000.0f};
    
    for (int band = 0; band < 5; ++band)
    {
        // Set the gain for this band
        if (auto* gainParam = parameters.getParameter("eq_gain_band" + juce::String(band)))
        {
            // Clamp predictions to reasonable range (-20dB to +20dB)
            float clampedGain = juce::jlimit(-20.0f, 20.0f, predictions[band]);
            gainParam->setValueNotifyingHost(gainParam->convertTo0to1(clampedGain));
        }
        
        // Set the frequency to VTR target
        if (auto* freqParam = parameters.getParameter("eq_freq_band" + juce::String(band)))
        {
            freqParam->setValueNotifyingHost(freqParam->convertTo0to1(targetFreqs[band]));
        }
        
        // Set Q to a reasonable value for tone matching
        if (auto* qParam = parameters.getParameter("eq_q_band" + juce::String(band)))
        {
            qParam->setValueNotifyingHost(qParam->convertTo0to1(1.0f)); // Q = 1.0
        }
        
        // Set appropriate filter type based on frequency range
        if (auto* typeParam = parameters.getParameter("eq_type_band" + juce::String(band)))
        {
            int filterType = 0; // Default to Bell
            
            // Set filter types based on VTR frequency targets
            if (targetFreqs[band] <= 150.0f)          // 80Hz (SUB) - Low Shelf
            {
                filterType = 2; // LowShelf
            }
            else if (targetFreqs[band] >= 6000.0f)    // 10kHz (HIGH) - High Shelf  
            {
                filterType = 1; // HighShelf
            }
            else                                       // 240Hz, 2.5kHz, 4kHz (LOW, MID, HIGH-MID) - Bell
            {
                filterType = 0; // Bell
            }
            
            typeParam->setValueNotifyingHost(typeParam->convertTo0to1(static_cast<float>(filterType)));
        }
        
        // Enable the band to make the setting visible
        if (auto* enableParam = parameters.getParameter("eq_enable_band" + juce::String(band)))
        {
            enableParam->setValueNotifyingHost(1.0f); // Enable the band
        }
    }
    
    juce::Logger::writeToLog("VTR predictions applied to EQ parameters - all bands enabled");
    
    // Force parameter manager update to ensure all changes are reflected
    parameterManager.updateAllTargets();
    
    // Notify editor to refresh display (on main thread)
    juce::MessageManager::callAsync([this]() {
        if (auto* editor = getActiveEditor())
        {
            editor->repaint();
        }
    });
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VaclisDynamicEQAudioProcessor();
}