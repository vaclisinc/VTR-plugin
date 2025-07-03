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
}

VaclisDynamicEQAudioProcessorEditor::~VaclisDynamicEQAudioProcessorEditor()
{
}

void VaclisDynamicEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draw the plugin name and version
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("vaclis-DynamicEQ", getLocalBounds(), juce::Justification::centredTop, 1);
    
    g.setFont (14.0f);
    g.drawFittedText ("v1.0.1 - Audio Pass-Through with resizable corner", 
                      getLocalBounds().reduced(0, 40), 
                      juce::Justification::centredTop, 1);
    
    // Draw status message
    g.setFont (12.0f);
    g.setColour (juce::Colours::lightgrey);
    g.drawFittedText ("Plugin loaded successfully. Audio will pass through unchanged.", 
                      getLocalBounds(), 
                      juce::Justification::centred, 1);
}

void VaclisDynamicEQAudioProcessorEditor::resized()
{
    // Layout components here when we add them in future steps
    // The built-in resizable corner is handled automatically by JUCE
}