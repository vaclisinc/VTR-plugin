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
    
    // Create slider attachments to connect sliders to parameters
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "input_gain", inputGainSlider);
    
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "output_gain", outputGainSlider);
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
    g.drawFittedText ("Dynamic EQ - Step 2.3.0", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);
    
    g.setFont (12.0f);
    g.setColour (juce::Colours::lightgrey);
    g.drawFittedText ("Scalable Architecture - 50+ Parameters Ready", 
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
    const int spacing = 30;
    
    // Calculate total width needed for both sliders
    int totalWidth = 2 * sliderWidth + spacing;
    
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
    
    // Position Output Gain controls  
    auto outputArea = slidersArea.removeFromLeft(sliderWidth);
    outputGainLabel.setBounds(outputArea.removeFromTop(labelHeight));
    outputGainSlider.setBounds(outputArea.removeFromTop(sliderHeight));
}