#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class VaclisDynamicEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VaclisDynamicEQAudioProcessorEditor (VaclisDynamicEQAudioProcessor&);
    ~VaclisDynamicEQAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    VaclisDynamicEQAudioProcessor& audioProcessor;
    
    std::unique_ptr<juce::ComponentBoundsConstrainer> constrainer;
    
    // GUI Components
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    
    // EQ Controls
    juce::Slider eqFreqSlider;
    juce::Label eqFreqLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqFreqAttachment;
    
    juce::Slider eqGainSlider;
    juce::Label eqGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqGainAttachment;
    
    juce::Slider eqQSlider;
    juce::Label eqQLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqQAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaclisDynamicEQAudioProcessorEditor)
};