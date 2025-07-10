#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "DSP/EQBand.h"
#include "SpectrumDisplay.h"
#include "FrequencyResponseDisplay.h"
#include "LevelMeter.h"

// Forward declaration
class VaclisDynamicEQAudioProcessor;

//==============================================================================
/**
 * Individual band control component for 4-band EQ
 */
class BandControlComponent : public juce::Component
{
public:
    BandControlComponent(int bandIndex, const juce::String& bandName, 
                        VaclisDynamicEQAudioProcessor& processor);
    ~BandControlComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Setup and management
    void setupComponents();
    void updateFilterTypeButtonStates(int filterType);
    
private:
    int bandIndex;
    juce::String bandName;
    VaclisDynamicEQAudioProcessor& audioProcessor;
    
    // Band controls
    juce::TextButton enableButton;
    juce::TextButton soloButton;
    juce::Label bandLabel;
    juce::Label frequencyRangeLabel;
    
    // EQ controls
    juce::Slider freqSlider;
    juce::Label freqLabel;
    juce::Slider gainSlider;
    juce::Label gainLabel;
    juce::Slider qSlider;
    juce::Label qLabel;
    
    // Filter type buttons
    struct FilterTypeButton
    {
        std::unique_ptr<juce::TextButton> button;
        juce::String text;
        juce::Colour selectedColour;
        int filterTypeIndex;
    };
    std::vector<FilterTypeButton> filterTypeButtons;
    
    // Dynamics controls
    juce::TextButton dynamicsToggleButton;
    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider, kneeSlider;
    juce::ComboBox detectionTypeCombo, modeCombo;
    juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel, kneeLabel;
    juce::Label detectionLabel, modeLabel;
    bool dynamicsExpanded = false;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;
    
    // Dynamics parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> detectionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    
    // Helper methods
    void setupFilterTypeButtons();
    void createFilterTypeButton(int index, const juce::String& text, juce::Colour colour);
    void filterTypeButtonClicked(int filterType);
    void setupDynamicsControls();
    void toggleDynamicsSection();
    
public:
    int getRequiredHeight() const;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandControlComponent)
};

class VaclisDynamicEQAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                            public juce::AudioProcessorValueTreeState::Listener,
                                            private juce::Timer
{
public:
    VaclisDynamicEQAudioProcessorEditor (VaclisDynamicEQAudioProcessor&);
    ~VaclisDynamicEQAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    // AudioProcessorValueTreeState::Listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    // Timer callback for level meter updates
    void timerCallback() override;

private:
    VaclisDynamicEQAudioProcessor& audioProcessor;
    
    std::unique_ptr<juce::ComponentBoundsConstrainer> constrainer;
    
    // Global controls
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    
    // Multi-band control components  
    std::array<std::unique_ptr<BandControlComponent>, DynamicEQ::CURRENT_BANDS> bandComponents;
    
    // Frequency response display (new dedicated area)
    std::unique_ptr<FrequencyResponseDisplay> frequencyResponseDisplay;
    juce::TextButton spectrumModeButton;
    
    // Old spectrum display (keep for compatibility, but not used in new design)
    std::unique_ptr<SpectrumDisplay> spectrumDisplay;
    
    // Level meters
    std::unique_ptr<LevelMeter> inputLevelMeter;
    std::unique_ptr<LevelMeter> outputLevelMeter;
    
    // Sidechain control
    juce::TextButton sidechainButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sidechainAttachment;
    
    // VTR components
    juce::TextButton loadReferenceButton;
    juce::TextButton applyVTRButton;
    juce::Label vtrStatusLabel;
    double vtrProgress = 0.0;
    std::unique_ptr<juce::ProgressBar> vtrProgressBar;
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    // VTR callbacks
    void loadReferenceAudio();
    void applyVTRSettings();
    void updateVTRStatus();
    
    // Layout and management
    void setupBandComponents();
    void updateWindowSize();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VaclisDynamicEQAudioProcessorEditor)
};