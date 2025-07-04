#include "PluginProcessor.h"
#include "PluginEditor.h"

VaclisDynamicEQAudioProcessorEditor::VaclisDynamicEQAudioProcessorEditor (VaclisDynamicEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set the initial size of the plugin window
    setSize (400, 300);
    
    // Create constrainer for resize limits
    constrainer = std::make_unique<juce::ComponentBoundsConstrainer>();
    constrainer->setMinimumSize (300, 200);
    constrainer->setMaximumSize (800, 600);
    
    // Use the built-in resize corner from AudioProcessorEditor
    setResizable (true, constrainer.get());
    
    // Setup Input Gain Slider
    inputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    inputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(inputGainSlider);
    
    // Setup Input Gain Label
    inputGainLabel.setText("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centredTop);
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(inputGainLabel);
    
    // Setup Output Gain Slider
    outputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(outputGainSlider);
    
    // Setup Output Gain Label
    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centredTop);
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(outputGainLabel);
    
    // Setup EQ Frequency Slider
    eqFreqSlider.setSliderStyle(juce::Slider::LinearVertical);
    eqFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    eqFreqSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(eqFreqSlider);
    
    // Setup EQ Frequency Label
    eqFreqLabel.setText("EQ Freq", juce::dontSendNotification);
    eqFreqLabel.setJustificationType(juce::Justification::centredTop);
    eqFreqLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(eqFreqLabel);
    
    // Setup EQ Gain Slider
    eqGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    eqGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    eqGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(eqGainSlider);
    
    // Setup EQ Gain Label
    eqGainLabel.setText("EQ Gain", juce::dontSendNotification);
    eqGainLabel.setJustificationType(juce::Justification::centredTop);
    eqGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(eqGainLabel);
    
    // Setup EQ Q Slider
    eqQSlider.setSliderStyle(juce::Slider::LinearVertical);
    eqQSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    eqQSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(eqQSlider);
    
    // Setup EQ Q Label
    eqQLabel.setText("EQ Q", juce::dontSendNotification);
    eqQLabel.setJustificationType(juce::Justification::centredTop);
    eqQLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(eqQLabel);
    
    // Create slider attachments to connect sliders to parameters
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "input_gain", inputGainSlider);
    
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "output_gain", outputGainSlider);
    
    eqFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "eq_freq", eqFreqSlider);
    
    eqGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "eq_gain", eqGainSlider);
    
    eqQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "eq_q", eqQSlider);
    
    // Setup Filter Type Selection
    setupFilterTypeButtons();
    
    // Listen to parameter changes
    audioProcessor.getValueTreeState().addParameterListener("eq_type", this);
}

VaclisDynamicEQAudioProcessorEditor::~VaclisDynamicEQAudioProcessorEditor()
{
    audioProcessor.getValueTreeState().removeParameterListener("eq_type", this);
}

void VaclisDynamicEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background with gradient
    juce::ColourGradient gradient (juce::Colour(0xff2a2a2a), 0, 0,
                                   juce::Colour(0xff1a1a1a), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill (gradient);
    g.fillAll();

    // Draw the plugin name and version
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("Dynamic EQ - Step 4.8 Refactored Architecture", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);
    
    g.setFont (12.0f);
    g.setColour (juce::Colours::lightgrey);
    g.drawFittedText ("Clean, Maintainable, Expandable", 
                      getLocalBounds().removeFromTop(120), 
                      juce::Justification::centred, 1);
}

void VaclisDynamicEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve space for title
    bounds.removeFromTop(50);
    
    // Position Filter Type buttons at the top with more space
    auto filterTypeArea = bounds.removeFromTop(100).reduced(20);  // Much more space to prevent overlap
    filterTypeLabel.setBounds(filterTypeArea.removeFromTop(35)); // More space for label
    
    // Arrange filter type buttons horizontally - expandable layout
    const int buttonWidth = 65;
    const int buttonHeight = 35;  // Taller buttons
    const int buttonSpacing = 12;  // More spacing
    int numButtons = static_cast<int>(filterTypeButtons.size());
    int totalButtonWidth = numButtons * buttonWidth + (numButtons - 1) * buttonSpacing;
    
    auto buttonArea = filterTypeArea.withWidth(totalButtonWidth)
                                   .withHeight(buttonHeight)
                                   .withCentre(filterTypeArea.getCentre());
    
    // Position all buttons dynamically
    for (int i = 0; i < numButtons; ++i)
    {
        if (filterTypeButtons[i].button)
        {
            filterTypeButtons[i].button->setBounds(buttonArea.removeFromLeft(buttonWidth));
            if (i < numButtons - 1)  // Don't add spacing after last button
                buttonArea.removeFromLeft(buttonSpacing);
        }
    }
    
    // Create area for controls with padding
    auto controlsArea = bounds.reduced(20);
    
    // Calculate slider dimensions for vertical sliders
    const int sliderWidth = 60;
    const int sliderHeight = 150;
    const int labelHeight = 20;
    const int spacing = 15;
    
    // Calculate total width needed for all 5 sliders (2 gain + 3 EQ)
    int totalWidth = 5 * sliderWidth + 4 * spacing;
    
    // Center the controls horizontally
    auto slidersArea = controlsArea.withWidth(totalWidth)
                                  .withHeight(sliderHeight + labelHeight + 10)
                                  .withCentre(controlsArea.getCentre());
    
    // Position Input Gain controls
    auto inputArea = slidersArea.removeFromLeft(sliderWidth);
    inputGainLabel.setBounds(inputArea.removeFromTop(labelHeight));
    inputGainSlider.setBounds(inputArea.removeFromTop(sliderHeight));
    
    // Add spacing
    slidersArea.removeFromLeft(spacing);
    
    // Position EQ Frequency controls
    auto eqFreqArea = slidersArea.removeFromLeft(sliderWidth);
    eqFreqLabel.setBounds(eqFreqArea.removeFromTop(labelHeight));
    eqFreqSlider.setBounds(eqFreqArea.removeFromTop(sliderHeight));
    
    // Add spacing
    slidersArea.removeFromLeft(spacing);
    
    // Position EQ Gain controls
    auto eqGainArea = slidersArea.removeFromLeft(sliderWidth);
    eqGainLabel.setBounds(eqGainArea.removeFromTop(labelHeight));
    eqGainSlider.setBounds(eqGainArea.removeFromTop(sliderHeight));
    
    // Add spacing
    slidersArea.removeFromLeft(spacing);
    
    // Position EQ Q controls
    auto eqQArea = slidersArea.removeFromLeft(sliderWidth);
    eqQLabel.setBounds(eqQArea.removeFromTop(labelHeight));
    eqQSlider.setBounds(eqQArea.removeFromTop(sliderHeight));
    
    // Add spacing
    slidersArea.removeFromLeft(spacing);
    
    // Position Output Gain controls  
    auto outputArea = slidersArea.removeFromLeft(sliderWidth);
    outputGainLabel.setBounds(outputArea.removeFromTop(labelHeight));
    outputGainSlider.setBounds(outputArea.removeFromTop(sliderHeight));
}

void VaclisDynamicEQAudioProcessorEditor::setupFilterTypeButtons()
{
    // Setup Filter Type Label
    filterTypeLabel.setText("Filter Type", juce::dontSendNotification);
    filterTypeLabel.setJustificationType(juce::Justification::centred);
    filterTypeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(filterTypeLabel);
    
    // Clear existing buttons
    filterTypeButtons.clear();
    
    // Create all filter type buttons - expandable design
    createFilterTypeButton(0, "BELL", juce::Colours::blue);
    createFilterTypeButton(1, "H-SHF", juce::Colours::green);
    createFilterTypeButton(2, "L-SHF", juce::Colours::orange);
    createFilterTypeButton(3, "H-PASS", juce::Colours::red);
    createFilterTypeButton(4, "L-PASS", juce::Colours::violet);
    
    // Set initial button state based on current parameter value
    auto* param = audioProcessor.getValueTreeState().getRawParameterValue("eq_type");
    int currentFilterType = param ? static_cast<int>(*param) : 0;
    updateFilterTypeButtonStates(0, currentFilterType);  // Band 0 for single-band mode
}

void VaclisDynamicEQAudioProcessorEditor::createFilterTypeButton(int index, const juce::String& text, juce::Colour colour)
{
    FilterTypeButton filterButton;
    filterButton.button = std::make_unique<juce::TextButton>();
    filterButton.text = text;
    filterButton.selectedColour = colour;
    filterButton.filterTypeIndex = index;
    
    // Configure button - no toggle states at all
    filterButton.button->setButtonText(text);
    filterButton.button->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    filterButton.button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    // Use lambda with proper capture for multi-band expansion
    filterButton.button->onClick = [this, index]() 
    { 
        filterTypeButtonClicked(0, index);  // Band 0 for single-band mode
    };
    
    addAndMakeVisible(*filterButton.button);
    filterTypeButtons.push_back(std::move(filterButton));
}

void VaclisDynamicEQAudioProcessorEditor::filterTypeButtonClicked(int bandIndex, int filterType)
{
    DBG("Button clicked: bandIndex=" << bandIndex << ", filterType=" << filterType);
    
    // FIRST: Update visual states immediately to avoid listener interference
    updateFilterTypeButtonStates(bandIndex, filterType);
    
    // THEN: Update the parameter value (for single-band mode, always band 0)
    if (bandIndex == 0)  // Single-band mode
    {
        // Use direct APVTS assignment for choice parameter
        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.getValueTreeState().getParameter("eq_type")))
        {
            int oldValue = choiceParam->getIndex();
            *choiceParam = filterType;  // Direct assignment of index
            int newValue = choiceParam->getIndex();
            DBG("Choice parameter change: " << oldValue << " -> " << newValue);
        }
    }
}

void VaclisDynamicEQAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "eq_type")
    {
        DBG("Parameter listener triggered: " << parameterID << " = " << newValue);
        // Update button states when parameter changes from automation, preset load, etc.
        int filterType = static_cast<int>(newValue + 0.5f); // Direct conversion - no normalization needed
        juce::MessageManager::callAsync([this, filterType]()
        {
            updateFilterTypeButtonStates(0, filterType);  // Band 0 for single-band mode
        });
    }
}

void VaclisDynamicEQAudioProcessorEditor::updateFilterTypeButtonStates(int bandIndex, int filterType)
{
    DBG("updateFilterTypeButtonStates: bandIndex=" << bandIndex << ", filterType=" << filterType);
    
    // Expandable for multi-band: For now, only handle band 0 (single-band mode)
    if (bandIndex != 0) return;  // Only handle single band for now
    
    // Debug: ensure filterType is valid
    if (filterType < 0 || filterType >= static_cast<int>(filterTypeButtons.size()))
    {
        DBG("Invalid filterType: " << filterType);
        return;
    }
    
    // Reset all buttons to transparent background with immediate effect
    for (int i = 0; i < static_cast<int>(filterTypeButtons.size()); ++i)
    {
        if (filterTypeButtons[i].button)
        {
            filterTypeButtons[i].button->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            filterTypeButtons[i].button->repaint();
            DBG("Reset button " << i << " to transparent");
        }
    }
    
    // Set the selected button's background color with immediate effect
    if (filterTypeButtons[filterType].button)
    {
        filterTypeButtons[filterType].button->setColour(
            juce::TextButton::buttonColourId, 
            filterTypeButtons[filterType].selectedColour
        );
        filterTypeButtons[filterType].button->repaint();
        DBG("Set button " << filterType << " to selected color");
    }
}