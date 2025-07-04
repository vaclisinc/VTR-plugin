#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// BandControlComponent Implementation

BandControlComponent::BandControlComponent(int bandIdx, const juce::String& name, 
                                         VaclisDynamicEQAudioProcessor& processor)
    : bandIndex(bandIdx), bandName(name), audioProcessor(processor)
{
    setupComponents();
}

BandControlComponent::~BandControlComponent()
{
}

void BandControlComponent::setupComponents()
{
    // Band label and frequency range
    bandLabel.setText(bandName, juce::dontSendNotification);
    bandLabel.setJustificationType(juce::Justification::centred);
    bandLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(bandLabel);
    
    // Set frequency range labels based on band
    const juce::String frequencyRanges[4] = {"20Hz-200Hz", "200Hz-800Hz", "800Hz-3kHz", "3kHz-20kHz"};
    frequencyRangeLabel.setText(frequencyRanges[bandIndex], juce::dontSendNotification);
    frequencyRangeLabel.setJustificationType(juce::Justification::centred);
    frequencyRangeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    frequencyRangeLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(frequencyRangeLabel);
    
    // Enable/Disable button
    enableButton.setButtonText("EN");
    enableButton.setToggleable(true);
    enableButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    enableButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    
    // Manual button handling for enable/disable
    enableButton.onClick = [this]() {
        bool newState = !enableButton.getToggleState();
        enableButton.setToggleState(newState, juce::dontSendNotification);
        
        // Update parameter directly
        juce::String paramID = "eq_enable_band" + juce::String(bandIndex);
        if (auto* param = dynamic_cast<juce::AudioParameterBool*>(
            audioProcessor.getValueTreeState().getParameter(paramID)))
        {
            *param = newState;
        }
    };
    
    addAndMakeVisible(enableButton);
    
    // Solo button
    soloButton.setButtonText("SOLO");
    soloButton.setToggleable(true);
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    
    // Manual button handling for solo
    soloButton.onClick = [this]() {
        bool newState = !soloButton.getToggleState();
        soloButton.setToggleState(newState, juce::dontSendNotification);
        
        // Update parameter directly
        juce::String paramID = "eq_solo_band" + juce::String(bandIndex);
        if (auto* param = dynamic_cast<juce::AudioParameterBool*>(
            audioProcessor.getValueTreeState().getParameter(paramID)))
        {
            *param = newState;
        }
    };
    
    addAndMakeVisible(soloButton);
    
    // EQ sliders
    freqSlider.setSliderStyle(juce::Slider::LinearVertical);
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    freqSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(freqSlider);
    
    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    gainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(gainSlider);
    
    qSlider.setSliderStyle(juce::Slider::LinearVertical);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    qSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(qSlider);
    
    // Labels
    freqLabel.setText("Freq", juce::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::centred);
    freqLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    freqLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(freqLabel);
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    gainLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(gainLabel);
    
    qLabel.setText("Q", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centred);
    qLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    qLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(qLabel);
    
    // Setup filter type buttons
    setupFilterTypeButtons();
    
    // Create parameter attachments
    juce::String bandSuffix = juce::String(bandIndex);
    
    freqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "eq_freq_band" + bandSuffix, freqSlider);
    
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "eq_gain_band" + bandSuffix, gainSlider);
    
    qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "eq_q_band" + bandSuffix, qSlider);
    
    // Initialize button states from parameters
    juce::String enableParamID = "eq_enable_band" + bandSuffix;
    juce::String soloParamID = "eq_solo_band" + bandSuffix;
    
    auto* enableParam = audioProcessor.getValueTreeState().getParameter(enableParamID);
    auto* soloParam = audioProcessor.getValueTreeState().getParameter(soloParamID);
    
    if (enableParam && soloParam)
    {
        // Set initial button states from parameter values
        float enableValue = enableParam->getValue();
        float soloValue = soloParam->getValue();
        
        enableButton.setToggleState(enableValue > 0.5f, juce::dontSendNotification);
        soloButton.setToggleState(soloValue > 0.5f, juce::dontSendNotification);
    }
    
    // Ensure proper initial layout
    juce::MessageManager::callAsync([this]()
    {
        if (getWidth() > 0 && getHeight() > 0)
        {
            resized();
            repaint();
        }
    });
}

void BandControlComponent::setupFilterTypeButtons()
{
    const std::array<std::pair<juce::String, juce::Colour>, 5> filterTypes = {{
        {"Bell", juce::Colours::orange},
        {"H.Shelf", juce::Colours::cyan},
        {"L.Shelf", juce::Colours::magenta}, 
        {"HPF", juce::Colours::red},
        {"LPF", juce::Colours::blue}
    }};
    
    for (int i = 0; i < 5; ++i)
    {
        createFilterTypeButton(i, filterTypes[i].first, filterTypes[i].second);
    }
}

void BandControlComponent::createFilterTypeButton(int index, const juce::String& text, juce::Colour colour)
{
    FilterTypeButton filterButton;
    filterButton.button = std::make_unique<juce::TextButton>();
    filterButton.text = text;
    filterButton.selectedColour = colour;
    filterButton.filterTypeIndex = index;
    
    filterButton.button->setButtonText(text);
    filterButton.button->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    filterButton.button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    filterButton.button->onClick = [this, index]() { filterTypeButtonClicked(index); };
    
    addAndMakeVisible(*filterButton.button);
    filterTypeButtons.push_back(std::move(filterButton));
}

void BandControlComponent::filterTypeButtonClicked(int filterType)
{
    updateFilterTypeButtonStates(filterType);
    
    juce::String paramID = "eq_type_band" + juce::String(bandIndex);
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(
        audioProcessor.getValueTreeState().getParameter(paramID)))
    {
        *choiceParam = filterType;
    }
}

void BandControlComponent::updateFilterTypeButtonStates(int filterType)
{
    // Reset all buttons
    for (size_t i = 0; i < filterTypeButtons.size(); ++i)
    {
        if (filterTypeButtons[i].button)
        {
            filterTypeButtons[i].button->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            filterTypeButtons[i].button->repaint();
        }
    }
    
    // Highlight selected button
    if (filterType >= 0 && filterType < static_cast<int>(filterTypeButtons.size()))
    {
        if (filterTypeButtons[filterType].button)
        {
            filterTypeButtons[filterType].button->setColour(
                juce::TextButton::buttonColourId, 
                filterTypeButtons[filterType].selectedColour
            );
            filterTypeButtons[filterType].button->repaint();
        }
    }
}

void BandControlComponent::paint(juce::Graphics& g)
{
    // Band background with different colors
    const juce::Colour bandColours[DynamicEQ::CURRENT_BANDS] = {
        juce::Colour(0xff1a2a3a),  // LOW - Blue tint
        juce::Colour(0xff1a3a2a),  // LOW-MID - Green tint  
        juce::Colour(0xff3a2a1a),  // HIGH-MID - Orange tint
        juce::Colour(0xff3a1a1a)   // HIGH - Red tint
    };
    
    g.setColour(bandColours[bandIndex]);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);
    
    // Border
    g.setColour(juce::Colours::darkgrey);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 8.0f, 1.0f);
}

void BandControlComponent::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    
    // EN | BAND-NAME | SOLO layout  
    auto topArea = bounds.removeFromTop(25);
    int buttonWidth = topArea.getWidth() / 3;  // Divide into three equal parts
    
    enableButton.setBounds(topArea.removeFromLeft(buttonWidth));
    bandLabel.setBounds(topArea.removeFromLeft(buttonWidth));
    soloButton.setBounds(topArea.removeFromLeft(buttonWidth));
    
    // Frequency range
    frequencyRangeLabel.setBounds(bounds.removeFromTop(15));
    
    bounds.removeFromTop(5); // spacing
    
    // Filter type buttons
    auto filterButtonArea = bounds.removeFromTop(30);
    int filterButtonWidth = filterButtonArea.getWidth() / 5;
    for (auto& filterButton : filterTypeButtons)
    {
        if (filterButton.button)
        {
            filterButton.button->setBounds(filterButtonArea.removeFromLeft(filterButtonWidth));
        }
    }
    
    // Add more spacing between filter buttons and EQ controls
    bounds.removeFromTop(20); // increased spacing for better visual separation
    
    // EQ controls - CENTER them properly
    auto labelArea = bounds.removeFromTop(15);
    auto sliderArea = bounds.removeFromTop(130);
    
    // Calculate centered positions for 3 controls
    int totalWidth = labelArea.getWidth();
    int controlWidth = 60;
    int totalControlsWidth = controlWidth * 3;
    int startX = (totalWidth - totalControlsWidth) / 2;
    
    // Position labels centered
    freqLabel.setBounds(startX, labelArea.getY(), controlWidth, labelArea.getHeight());
    gainLabel.setBounds(startX + controlWidth, labelArea.getY(), controlWidth, labelArea.getHeight());
    qLabel.setBounds(startX + controlWidth * 2, labelArea.getY(), controlWidth, labelArea.getHeight());
    
    // Position sliders centered (same X positions as labels)
    freqSlider.setBounds(startX, sliderArea.getY(), controlWidth, sliderArea.getHeight());
    gainSlider.setBounds(startX + controlWidth, sliderArea.getY(), controlWidth, sliderArea.getHeight());
    qSlider.setBounds(startX + controlWidth * 2, sliderArea.getY(), controlWidth, sliderArea.getHeight());
}

VaclisDynamicEQAudioProcessorEditor::VaclisDynamicEQAudioProcessorEditor (VaclisDynamicEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set initial size for multi-band layout
    setSize(1250, 385);
    
    // Create constrainer for resize limits
    constrainer = std::make_unique<juce::ComponentBoundsConstrainer>();
    constrainer->setMinimumSize(800, 350);
    constrainer->setMaximumSize(1400, 600);
    
    // Use the built-in resize corner from AudioProcessorEditor
    setResizable(true, constrainer.get());
    
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
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "input_gain", inputGainSlider);
    
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "output_gain", outputGainSlider);
    
    // Setup 4-band components
    setupBandComponents();
    
    // Ensure proper initial layout
    resized();
    repaint();
    
    // Initialize all band components
    for (int i = 0; i < DynamicEQ::CURRENT_BANDS; ++i)
    {
        if (bandComponents[i])
        {
            bandComponents[i]->resized();
            bandComponents[i]->repaint();
        }
    }
}

VaclisDynamicEQAudioProcessorEditor::~VaclisDynamicEQAudioProcessorEditor()
{
}

void VaclisDynamicEQAudioProcessorEditor::setupBandComponents()
{
    const juce::String bandNames[DynamicEQ::CURRENT_BANDS] = {"LOW", "LOW-MID", "HIGH-MID", "HIGH"};
    
    for (int i = 0; i < DynamicEQ::CURRENT_BANDS; ++i)
    {
        bandComponents[i] = std::make_unique<BandControlComponent>(i, bandNames[i], audioProcessor);
        addAndMakeVisible(*bandComponents[i]);
    }
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
    g.drawFittedText ("Dynamic EQ - Step 5 Multi-Band System", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);
    
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
    bounds.removeFromTop(70);
    
    // Create main layout area
    auto mainArea = bounds.reduced(10);
    
    // Input Gain (left side)
    auto inputGainArea = mainArea.removeFromLeft(80);
    inputGainLabel.setBounds(inputGainArea.removeFromTop(20));
    inputGainSlider.setBounds(inputGainArea.removeFromTop(250));
    
    // Output Gain (right side) 
    auto outputGainArea = mainArea.removeFromRight(80);
    outputGainLabel.setBounds(outputGainArea.removeFromTop(20));
    outputGainSlider.setBounds(outputGainArea.removeFromTop(250));
    
    // Multi-band area (center)
    auto bandsArea = mainArea.reduced(10, 0);
    int bandWidth = bandsArea.getWidth() / DynamicEQ::CURRENT_BANDS;
    
    for (int i = 0; i < DynamicEQ::CURRENT_BANDS; ++i)
    {
        if (bandComponents[i])
        {
            auto bandArea = bandsArea.removeFromLeft(bandWidth);
            if (i < DynamicEQ::CURRENT_BANDS - 1) bandArea = bandArea.withTrimmedRight(5); // Add spacing between bands
            bandComponents[i]->setBounds(bandArea);
        }
    }
}

void VaclisDynamicEQAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Handle parameter changes for all bands
    for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
    {
        juce::String typeParamID = "eq_type_band" + juce::String(band);
        if (parameterID == typeParamID && bandComponents[band])
        {
            int filterType = static_cast<int>(newValue + 0.5f);
            juce::MessageManager::callAsync([this, band, filterType]()
            {
                if (bandComponents[band])
                    bandComponents[band]->updateFilterTypeButtonStates(filterType);
            });
            break;
        }
    }
}