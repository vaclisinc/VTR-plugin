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
}

VaclisDynamicEQAudioProcessorEditor::~VaclisDynamicEQAudioProcessorEditor()
{
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
    g.drawFittedText ("Dynamic EQ - Step 3 Complete", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);
    
    g.setFont (12.0f);
    g.setColour (juce::Colours::lightgrey);
    g.drawFittedText ("Single Band EQ with Professional Filters", 
                      getLocalBounds().removeFromTop(120), 
                      juce::Justification::centred, 1);
}

void VaclisDynamicEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve space for title
    bounds.removeFromTop(50);
    
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