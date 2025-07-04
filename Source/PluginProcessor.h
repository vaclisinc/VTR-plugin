#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include "Parameters/ParameterManager.h"
#include "DSP/EQBand.h"
#include "DSP/GainProcessor.h"

class VaclisDynamicEQAudioProcessor  : public juce::AudioProcessor
{
public:
    VaclisDynamicEQAudioProcessor();
    ~VaclisDynamicEQAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Scalable parameter creation helpers
    static void addGainParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                const juce::String& parameterID,
                                const juce::String& parameterName,
                                float defaultValue = 0.0f);
    
    static void addFrequencyParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                     const juce::String& parameterID,
                                     const juce::String& parameterName,
                                     float defaultValue = 1000.0f);
    
    static void addQParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                             const juce::String& parameterID,
                             const juce::String& parameterName,
                             float defaultValue = 1.0f);
    
    static void addFilterTypeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                      const juce::String& parameterID,
                                      const juce::String& parameterName,
                                      float defaultValue = 0.0f);
    
    juce::AudioProcessorValueTreeState parameters;
    
    // Modular DSP components
    DynamicEQ::ParameterManager parameterManager;
    DynamicEQ::GainProcessor inputGain;
    DynamicEQ::GainProcessor outputGain;
    DynamicEQ::EQBand eqBand;
    
    // Processing helper methods
    void updateParameterSmoothers();
    void processInputGain(juce::AudioBuffer<float>& buffer);
    void processEQ(juce::AudioBuffer<float>& buffer);
    void processOutputGain(juce::AudioBuffer<float>& buffer);
    
    // Safety monitoring
    bool checkForDangerousGainLevels() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaclisDynamicEQAudioProcessor)
};