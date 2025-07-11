#include "MinimalEditor.h"
#include "PluginEditor.h"

MinimalEditor::MinimalEditor(VaclisDynamicEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    DBG("MinimalEditor constructor starting");
    
    // Set basic size
    setSize(600, 950);
    
    // Setup basic label
    testLabel.setText("Minimal Editor Test", juce::dontSendNotification);
    testLabel.setJustificationType(juce::Justification::centred);
    testLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(testLabel);
    
    // Setup basic slider
    testSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    testSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    testSlider.setRange(0.0, 1.0, 0.01);
    testSlider.setValue(0.5);
    addAndMakeVisible(testSlider);
    
    // Step 1: Add basic gain controls
    DBG("MinimalEditor adding gain controls");
    
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
    
    // Create global gain attachments
    DBG("MinimalEditor creating parameter attachments");
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "input_gain", inputGainSlider);
    
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "output_gain", outputGainSlider);
    
    // Step 2: Add spectrum display
    DBG("MinimalEditor adding spectrum display");
    spectrumDisplay = std::make_unique<SpectrumDisplay>(audioProcessor.getSpectrumAnalyzer());
    spectrumDisplay->setDisplayMode(SpectrumDisplay::DisplayMode::Both);
    spectrumDisplay->setAlpha(0.7f);
    addAndMakeVisible(*spectrumDisplay);
    DBG("MinimalEditor spectrum display added successfully");
    
    // Step 3: Add frequency response display
    DBG("MinimalEditor adding frequency response display");
    frequencyResponseDisplay = std::make_unique<FrequencyResponseDisplay>(audioProcessor.getSpectrumAnalyzer(), audioProcessor);
    frequencyResponseDisplay->setDisplayMode(FrequencyResponseDisplay::DisplayMode::Both);
    frequencyResponseDisplay->setSpectrumVisible(true);
    addAndMakeVisible(*frequencyResponseDisplay);
    DBG("MinimalEditor frequency response display added successfully");
    
    // Step 4: Add one band component (most likely crash point)
    DBG("MinimalEditor adding band component");
    bandComponent = std::make_unique<BandControlComponent>(0, "TEST", audioProcessor);
    addAndMakeVisible(*bandComponent);
    DBG("MinimalEditor band component added successfully");
    
    // Step 5: Test parameter existence before creating band components
    DBG("MinimalEditor testing parameter existence");
    auto& apvts = audioProcessor.getValueTreeState();
    
    // Test if all required parameters exist for each band
    for (int i = 0; i < 5; ++i)
    {
        juce::String freqID = "eq_freq_band" + juce::String(i);
        juce::String gainID = "eq_gain_band" + juce::String(i);
        juce::String qID = "eq_q_band" + juce::String(i);
        juce::String typeID = "eq_type_band" + juce::String(i);
        juce::String enableID = "eq_enable_band" + juce::String(i);
        juce::String soloID = "eq_solo_band" + juce::String(i);
        
        bool freqExists = apvts.getParameter(freqID) != nullptr;
        bool gainExists = apvts.getParameter(gainID) != nullptr;
        bool qExists = apvts.getParameter(qID) != nullptr;
        bool typeExists = apvts.getParameter(typeID) != nullptr;
        bool enableExists = apvts.getParameter(enableID) != nullptr;
        bool soloExists = apvts.getParameter(soloID) != nullptr;
        
        DBG("Band " + juce::String(i) + " params - freq:" + (freqExists ? "true" : "false") + " gain:" + (gainExists ? "true" : "false") 
            + " q:" + (qExists ? "true" : "false") + " type:" + (typeExists ? "true" : "false") 
            + " enable:" + (enableExists ? "true" : "false") + " solo:" + (soloExists ? "true" : "false"));
            
        if (!freqExists || !gainExists || !qExists || !typeExists || !enableExists || !soloExists)
        {
            DBG("ERROR: Missing parameters for band " + juce::String(i));
            return; // Don't create components if parameters are missing
        }
    }
    
    // Step 6: Create single additional band component to test setupBandComponents() equivalent
    DBG("MinimalEditor testing second band component creation");
    try {
        secondBandComponent = std::make_unique<BandControlComponent>(1, "LOW", audioProcessor);
        addAndMakeVisible(*secondBandComponent);
        DBG("MinimalEditor second band component created successfully");
    } catch (const std::exception& e) {
        DBG("Exception creating second band component: " + juce::String(e.what()));
        return;
    } catch (...) {
        DBG("Unknown exception creating second band component");
        return;
    }
    
    DBG("MinimalEditor constructor complete");
}

MinimalEditor::~MinimalEditor()
{
    DBG("MinimalEditor destructor");
}

void MinimalEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("Minimal Editor - Testing", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);
}

void MinimalEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    testLabel.setBounds(bounds.removeFromTop(80));
    testSlider.setBounds(bounds.removeFromTop(40));
    
    // Layout gain controls
    auto gainArea = bounds.removeFromTop(200);
    auto leftGainArea = gainArea.removeFromLeft(100);
    auto rightGainArea = gainArea.removeFromRight(100);
    
    // Input gain
    inputGainLabel.setBounds(leftGainArea.removeFromTop(20));
    inputGainSlider.setBounds(leftGainArea);
    
    // Output gain  
    outputGainLabel.setBounds(rightGainArea.removeFromTop(20));
    outputGainSlider.setBounds(rightGainArea);
    
    // Spectrum display
    auto spectrumArea = bounds.removeFromTop(120);
    if (spectrumDisplay)
    {
        spectrumDisplay->setBounds(spectrumArea);
    }
    
    // Frequency response display
    auto freqRespArea = bounds.removeFromTop(120);
    if (frequencyResponseDisplay)
    {
        frequencyResponseDisplay->setBounds(freqRespArea);
    }
    
    // Band component
    auto bandArea = bounds.removeFromTop(300);
    if (bandComponent)
    {
        bandComponent->setBounds(bandArea);
    }
    
    // Second band component test
    auto secondBandArea = bounds.removeFromTop(200);
    if (secondBandComponent)
    {
        secondBandComponent->setBounds(secondBandArea);
    }
}