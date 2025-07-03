#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include <juce_dsp/juce_dsp.h>

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
    
    juce::AudioProcessorValueTreeState parameters;
    
    // Scalable parameter management system
    struct ParameterManager
    {
        std::vector<juce::SmoothedValue<float>> smoothedValues;
        std::vector<std::atomic<float>*> parameterPointers;
        std::vector<juce::String> parameterIDs;
        
        void addParameter(const juce::String& parameterID, juce::AudioProcessorValueTreeState& apvts);
        void prepare(double sampleRate, double smoothingTimeMs = 30.0);
        void updateAllTargets();
        juce::SmoothedValue<float>* getSmoothedValue(const juce::String& parameterID);
        float getCurrentValue(const juce::String& parameterID);
    };
    
    ParameterManager parameterManager;
    
    // Reusable gain processor class - now uses ParameterManager
    struct GainProcessor
    {
        juce::String parameterID;
        ParameterManager* manager = nullptr;
        
        void setup(const juce::String& paramID, ParameterManager* paramManager);
        void processBuffer(juce::AudioBuffer<float>& buffer);
    };
    
    GainProcessor inputGain;
    GainProcessor outputGain;
    
    // EQ Band using chowdsp State Variable Filter
    struct EQBand
    {
        juce::dsp::IIR::Filter<float> filter;
        juce::String freqParamID;
        juce::String gainParamID;
        juce::String qParamID;
        ParameterManager* manager = nullptr;
        double currentSampleRate = 44100.0;
        
        void setup(const juce::String& freqID, const juce::String& gainID, const juce::String& qID, ParameterManager* paramManager);
        void prepare(double sampleRate, int samplesPerBlock);
        void processBuffer(juce::AudioBuffer<float>& buffer);
        void updateParameters();
    };
    
    EQBand eqBand;
    
    // Processing helper methods
    void updateParameterSmoothers();
    void processInputGain(juce::AudioBuffer<float>& buffer);
    void processEQ(juce::AudioBuffer<float>& buffer);
    void processOutputGain(juce::AudioBuffer<float>& buffer);
    
    // Safety monitoring
    bool checkForDangerousGainLevels() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaclisDynamicEQAudioProcessor)
};