#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include <atomic>
#include "Parameters/ParameterManager.h"
#include "DSP/EQBand.h"
#include "DSP/GainProcessor.h"
#include "SpectrumAnalyzer.h"

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
    SpectrumAnalyzer& getSpectrumAnalyzer() { return spectrumAnalyzer; }
    
    // Level metering
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }

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
    
    // Dynamics parameter creation helpers
    static void addThresholdParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                     const juce::String& parameterID,
                                     const juce::String& parameterName,
                                     float defaultValue = -20.0f);
    
    static void addRatioParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                 const juce::String& parameterID,
                                 const juce::String& parameterName,
                                 float defaultValue = 4.0f);
    
    static void addAttackParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                  const juce::String& parameterID,
                                  const juce::String& parameterName,
                                  float defaultValue = 1.0f);
    
    static void addReleaseParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                   const juce::String& parameterID,
                                   const juce::String& parameterName,
                                   float defaultValue = 100.0f);
    
    static void addKneeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                const juce::String& parameterID,
                                const juce::String& parameterName,
                                float defaultValue = 2.0f);
    
    static void addDetectionTypeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                         const juce::String& parameterID,
                                         const juce::String& parameterName,
                                         float defaultValue = 0.0f);
    
    static void addDynamicsModeParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                        const juce::String& parameterID,
                                        const juce::String& parameterName,
                                        float defaultValue = 0.0f);
    
    static void addDynamicsBypassParameter(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                          const juce::String& parameterID,
                                          const juce::String& parameterName,
                                          bool defaultValue = false);
    
    juce::AudioProcessorValueTreeState parameters;
    
    // Modular DSP components
    DynamicEQ::ParameterManager parameterManager;
    DynamicEQ::GainProcessor inputGain;
    DynamicEQ::GainProcessor outputGain;
    DynamicEQ::MultiBandEQ multiBandEQ;
    SpectrumAnalyzer spectrumAnalyzer;
    
    // Level metering (atomic for thread safety)
    std::atomic<float> inputLevel{0.0f};
    std::atomic<float> outputLevel{0.0f};
    
    // Processing helper methods
    void updateParameterSmoothers();
    void processInputGain(juce::AudioBuffer<float>& buffer);
    void processEQ(juce::AudioBuffer<float>& buffer);
    void processEQWithSidechain(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>* sidechainBuffer);
    void processOutputGain(juce::AudioBuffer<float>& buffer);
    
    // Safety monitoring
    bool checkForDangerousGainLevels() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaclisDynamicEQAudioProcessor)
};