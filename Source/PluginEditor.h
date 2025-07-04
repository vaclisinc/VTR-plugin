#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class VaclisDynamicEQAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                            public juce::AudioProcessorValueTreeState::Listener
{
public:
    VaclisDynamicEQAudioProcessorEditor (VaclisDynamicEQAudioProcessor&);
    ~VaclisDynamicEQAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    // AudioProcessorValueTreeState::Listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;

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
    
    // Filter Type Selection - Expandable for multi-band
    struct FilterTypeButton
    {
        std::unique_ptr<juce::TextButton> button;
        juce::String text;
        juce::Colour selectedColour;
        int filterTypeIndex;
    };
    
    std::vector<FilterTypeButton> filterTypeButtons;
    juce::Label filterTypeLabel;
    
    // Multi-band expandable methods
    void setupFilterTypeButtons();
    void filterTypeButtonClicked(int bandIndex, int filterType);  // Prepared for multi-band
    void updateFilterTypeButtonStates(int bandIndex, int filterType);  // Prepared for multi-band
    void createFilterTypeButton(int index, const juce::String& text, juce::Colour colour);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaclisDynamicEQAudioProcessorEditor)
};