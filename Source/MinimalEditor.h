#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "SpectrumDisplay.h"
#include "FrequencyResponseDisplay.h"
#include "DSP/EQBand.h"

// Forward declaration
class BandControlComponent;

class MinimalEditor : public juce::AudioProcessorEditor
{
public:
    MinimalEditor(VaclisDynamicEQAudioProcessor& p);
    ~MinimalEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VaclisDynamicEQAudioProcessor& audioProcessor;
    
    // Only basic components
    juce::Label testLabel;
    juce::Slider testSlider;
    
    // Step 1: Add basic gain controls
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;
    
    // Parameter attachments for gain controls
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    
    // Step 2: Add spectrum display
    std::unique_ptr<SpectrumDisplay> spectrumDisplay;
    
    // Step 3: Add frequency response display
    std::unique_ptr<FrequencyResponseDisplay> frequencyResponseDisplay;
    
    // Step 4: Add one band component (most likely crash point)
    std::unique_ptr<BandControlComponent> bandComponent;
    
    // Step 5: Test second band component (simpler than array)
    std::unique_ptr<BandControlComponent> secondBandComponent;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MinimalEditor)
};