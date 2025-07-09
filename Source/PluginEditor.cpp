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
    frequencyRangeLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
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
    freqLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(freqLabel);
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    gainLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(gainLabel);
    
    qLabel.setText("Q", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centred);
    qLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    qLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(qLabel);
    
    // Setup filter type buttons
    setupFilterTypeButtons();
    
    // Setup dynamics controls
    setupDynamicsControls();
    
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

void BandControlComponent::setupDynamicsControls()
{
    // Dynamics toggle button
    dynamicsToggleButton.setButtonText("DYNAMICS");
    dynamicsToggleButton.setToggleable(true);
    dynamicsToggleButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::cyan);
    dynamicsToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    dynamicsToggleButton.onClick = [this]() { toggleDynamicsSection(); };
    addAndMakeVisible(dynamicsToggleButton);
    
    // Dynamics sliders (rotary style)
    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& labelText) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 14); // Wider text box, taller text
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::cyan);
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
        
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setFont(juce::Font(juce::FontOptions(10.0f))); // Consistent font size, using modern constructor
        addAndMakeVisible(label);
        
        // Initially hidden
        slider.setVisible(false);
        label.setVisible(false);
    };
    
    setupRotarySlider(thresholdSlider, thresholdLabel, "Thresh");
    setupRotarySlider(ratioSlider, ratioLabel, "Ratio");
    setupRotarySlider(attackSlider, attackLabel, "Attack");
    setupRotarySlider(releaseSlider, releaseLabel, "Release");
    setupRotarySlider(kneeSlider, kneeLabel, "Knee");
    
    // Detection type combo box
    detectionTypeCombo.addItem("Peak", 1);
    detectionTypeCombo.addItem("RMS", 2);
    detectionTypeCombo.addItem("Blend", 3);
    detectionTypeCombo.setSelectedId(1);
    detectionTypeCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    detectionTypeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colours::darkgrey);
    detectionTypeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(detectionTypeCombo);
    
    detectionLabel.setText("Detection", juce::dontSendNotification);
    detectionLabel.setJustificationType(juce::Justification::centred);
    detectionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    detectionLabel.setFont(juce::Font(juce::FontOptions(10.0f))); // Consistent font size
    addAndMakeVisible(detectionLabel);
    
    // Mode combo box
    modeCombo.addItem("Compressive", 1);
    modeCombo.addItem("Expansive", 2);
    modeCombo.addItem("De-esser", 3);
    modeCombo.addItem("Gate", 4);
    modeCombo.setSelectedId(1);
    modeCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    modeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colours::darkgrey);
    modeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(modeCombo);
    
    modeLabel.setText("Mode", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centred);
    modeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    modeLabel.setFont(juce::Font(juce::FontOptions(10.0f))); // Consistent font size
    addAndMakeVisible(modeLabel);
    
    // Bypass button removed - DYN button handles enable/disable
    
    // Initially hide combo boxes
    detectionTypeCombo.setVisible(false);
    detectionLabel.setVisible(false);
    modeCombo.setVisible(false);
    modeLabel.setVisible(false);
    
    // Create parameter attachments
    juce::String bandSuffix = juce::String(bandIndex);
    
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "dyn_threshold_band" + bandSuffix, thresholdSlider);
    
    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "dyn_ratio_band" + bandSuffix, ratioSlider);
    
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "dyn_attack_band" + bandSuffix, attackSlider);
    
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "dyn_release_band" + bandSuffix, releaseSlider);
    
    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "dyn_knee_band" + bandSuffix, kneeSlider);
    
    detectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "dyn_detection_band" + bandSuffix, detectionTypeCombo);
    
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "dyn_mode_band" + bandSuffix, modeCombo);
    
    // Bypass attachment removed - DYN button controls bypass parameter directly
}

void BandControlComponent::toggleDynamicsSection()
{
    dynamicsExpanded = !dynamicsExpanded;
    
    // Update button state
    dynamicsToggleButton.setToggleState(dynamicsExpanded, juce::dontSendNotification);
    
    // Link dynamics section state to bypass parameter
    juce::String bypassParamID = "dyn_bypass_band" + juce::String(bandIndex);
    if (auto* bypassParam = dynamic_cast<juce::AudioParameterBool*>(
        audioProcessor.getValueTreeState().getParameter(bypassParamID)))
    {
        // When expanding dynamics section, enable dynamics (bypass = false)
        // When collapsing dynamics section, disable dynamics (bypass = true)
        *bypassParam = !dynamicsExpanded;
    }
    
    // Show/hide dynamics controls
    thresholdSlider.setVisible(dynamicsExpanded);
    thresholdLabel.setVisible(dynamicsExpanded);
    ratioSlider.setVisible(dynamicsExpanded);
    ratioLabel.setVisible(dynamicsExpanded);
    attackSlider.setVisible(dynamicsExpanded);
    attackLabel.setVisible(dynamicsExpanded);
    releaseSlider.setVisible(dynamicsExpanded);
    releaseLabel.setVisible(dynamicsExpanded);
    kneeSlider.setVisible(dynamicsExpanded);
    kneeLabel.setVisible(dynamicsExpanded);
    
    detectionTypeCombo.setVisible(dynamicsExpanded);
    detectionLabel.setVisible(dynamicsExpanded);
    modeCombo.setVisible(dynamicsExpanded);
    modeLabel.setVisible(dynamicsExpanded);
    
    // Trigger layout update
    resized();
    
    // Notify parent that our size requirements have changed
    if (auto* parent = getParentComponent())
    {
        parent->resized();
    }
}

int BandControlComponent::getRequiredHeight() const
{
    int baseHeight = 280; // Base height for EQ controls
    int dynamicsHeight = dynamicsExpanded ? 185 : 0; // 2-row layout with larger knobs (75+75+spacing)
    return baseHeight + dynamicsHeight;
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
    
    // Dynamics section
    bounds.removeFromTop(10); // spacing
    
    // Dynamics toggle button
    auto dynamicsToggleArea = bounds.removeFromTop(25);
    dynamicsToggleButton.setBounds(dynamicsToggleArea);
    
    // Dynamics controls (only layout if expanded) - 3-column, 2-row layout
    if (dynamicsExpanded)
    {
        bounds.removeFromTop(10); // spacing
        
        // Use same column positions as Freq/Gain/Q sliders above
        int columnWidth = controlWidth; // Same as EQ controls (60px)
        int knobSize = 65; // Larger rotary knobs
        int startX = (bounds.getWidth() - (columnWidth * 3)) / 2;
        
        // Column positions aligned with EQ controls
        int col1X = startX;
        int col2X = startX + columnWidth;
        int col3X = startX + columnWidth * 2;
        
        // Row 1: Threshold, Ratio, Attack
        auto row1LabelArea = bounds.removeFromTop(12);
        auto row1ControlArea = bounds.removeFromTop(75); // Taller for bigger knobs
        
        // Threshold (Col 1)
        thresholdLabel.setBounds(col1X, row1LabelArea.getY(), columnWidth, row1LabelArea.getHeight());
        thresholdSlider.setBounds(col1X - 2, row1ControlArea.getY(), knobSize, knobSize);
        
        // Ratio (Col 2)
        ratioLabel.setBounds(col2X, row1LabelArea.getY(), columnWidth, row1LabelArea.getHeight());
        ratioSlider.setBounds(col2X - 2, row1ControlArea.getY(), knobSize, knobSize);
        
        // Attack (Col 3)
        attackLabel.setBounds(col3X, row1LabelArea.getY(), columnWidth, row1LabelArea.getHeight());
        attackSlider.setBounds(col3X - 2, row1ControlArea.getY(), knobSize, knobSize);
        
        bounds.removeFromTop(10); // spacing between rows
        
        // Row 2: Release, Detection/Mode (vertical sub-component), Knee
        auto row2LabelArea = bounds.removeFromTop(12);
        auto row2ControlArea = bounds.removeFromTop(75);
        
        // Release (Col 1)
        releaseLabel.setBounds(col1X, row2LabelArea.getY(), columnWidth, row2LabelArea.getHeight());
        releaseSlider.setBounds(col1X - 2, row2ControlArea.getY(), knobSize, knobSize);
        
        // Detection/Mode vertical sub-component (Col 2) - tightly packed
        auto comboSubArea = juce::Rectangle<int>(col2X, row2LabelArea.getY(), columnWidth, row2LabelArea.getHeight() + row2ControlArea.getHeight());
        
        // Detection (upper part of sub-component)
        detectionLabel.setBounds(col2X, row2LabelArea.getY(), columnWidth, row2LabelArea.getHeight());
        detectionTypeCombo.setBounds(col2X + 2, row2ControlArea.getY() + 5, columnWidth - 4, 22);
        
        // Mode (lower part of sub-component, tightly below Detection)
        modeLabel.setBounds(col2X, row2ControlArea.getY() + 30, columnWidth, 12);
        modeCombo.setBounds(col2X + 2, row2ControlArea.getY() + 42, columnWidth - 4, 22);
        
        // Knee (Col 3)
        kneeLabel.setBounds(col3X, row2LabelArea.getY(), columnWidth, row2LabelArea.getHeight());
        kneeSlider.setBounds(col3X - 2, row2ControlArea.getY(), knobSize, knobSize);
    }
}

VaclisDynamicEQAudioProcessorEditor::VaclisDynamicEQAudioProcessorEditor (VaclisDynamicEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set initial size for multi-band layout
    setSize(1250, 730);
    
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
    
    // Setup frequency response display (dedicated area)
    frequencyResponseDisplay = std::make_unique<FrequencyResponseDisplay>(audioProcessor.getSpectrumAnalyzer(), audioProcessor);
    frequencyResponseDisplay->setDisplayMode(FrequencyResponseDisplay::DisplayMode::Both);
    frequencyResponseDisplay->setSpectrumVisible(true); // Start visible
    addAndMakeVisible(*frequencyResponseDisplay);
    
    // Keep old spectrum display for compatibility (not visible by default)
    spectrumDisplay = std::make_unique<SpectrumDisplay>(audioProcessor.getSpectrumAnalyzer());
    spectrumDisplay->setDisplayMode(SpectrumDisplay::DisplayMode::Both);
    spectrumDisplay->setAlpha(0.7f);
    spectrumDisplay->setVisible(false); // Not used in new design
    addAndMakeVisible(*spectrumDisplay);
    
    // Setup level meters
    inputLevelMeter = std::make_unique<LevelMeter>();
    inputLevelMeter->setOrientation(false); // Vertical
    inputLevelMeter->setRange(-60.0f, 0.0f);
    addAndMakeVisible(*inputLevelMeter);
    
    outputLevelMeter = std::make_unique<LevelMeter>();
    outputLevelMeter->setOrientation(false); // Vertical  
    outputLevelMeter->setRange(-60.0f, 0.0f);
    addAndMakeVisible(*outputLevelMeter);
    
    // Setup spectrum mode button
    spectrumModeButton.setButtonText("SPEC");
    spectrumModeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));      // Dark gray when off
    spectrumModeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF00AA00));    // Bright green when on
    spectrumModeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    spectrumModeButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    spectrumModeButton.setToggleable(true);
    spectrumModeButton.setClickingTogglesState(true);
    spectrumModeButton.setRadioGroupId(0); // Ensure proper toggle behavior
    spectrumModeButton.setToggleState(true, juce::dontSendNotification); // Default on
    spectrumModeButton.onClick = [this]() {
        if (frequencyResponseDisplay) {
            bool shouldShow = spectrumModeButton.getToggleState();
            frequencyResponseDisplay->setSpectrumVisible(shouldShow);
            frequencyResponseDisplay->repaint();
            repaint(); // Force GUI refresh
            DBG("Spectrum display set to: " << (shouldShow ? "visible" : "hidden"));
        }
    };
    addAndMakeVisible(spectrumModeButton);
    
    // Setup sidechain button
    sidechainButton.setButtonText("SC");
    sidechainButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0x40404040));
    sidechainButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0x80FF6600)); // Orange when active
    sidechainButton.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    sidechainButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    sidechainButton.setToggleable(true);
    sidechainButton.setClickingTogglesState(true); // 讓按鈕可以切換狀態
    addAndMakeVisible(sidechainButton);
    
    // Create sidechain attachment
    sidechainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "sidechain_enable", sidechainButton);
    
    // Start timer for level meter updates (30Hz)
    startTimer(33);
    
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
    g.drawFittedText ("Dynamic EQ - Step 9: Draggable EQ Points", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);
    
    g.setFont (12.0f);
    g.setColour (juce::Colours::lightgrey);
    g.drawFittedText ("Clean, Maintainable, Expandable", 
                      getLocalBounds().removeFromTop(120), 
                      juce::Justification::centred, 1);
}

void VaclisDynamicEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve space for title and add control buttons
    auto titleArea = bounds.removeFromTop(40); // Reduced height
    auto buttonRow = titleArea.removeFromRight(120).removeFromBottom(25).reduced(5);
    
    // Add frequency response display area
    auto frequencyResponseArea = bounds.removeFromTop(180); // Increased height by 50% (120 * 1.5 = 180)
    
    // Position frequency response display
    if (frequencyResponseDisplay)
    {
        frequencyResponseDisplay->setBounds(frequencyResponseArea.reduced(10));
    }
    
    // Position buttons side by side
    auto specButtonArea = buttonRow.removeFromLeft(50);
    auto scButtonArea = buttonRow.removeFromLeft(50);
    
    spectrumModeButton.setBounds(specButtonArea);
    sidechainButton.setBounds(scButtonArea);
    
    // Create main layout area
    auto mainArea = bounds.reduced(10);
    
    // Input Gain (left side)
    auto inputGainArea = mainArea.removeFromLeft(100); // Wider for meter
    inputGainLabel.setBounds(inputGainArea.removeFromTop(20));
    
    auto inputControlsArea = inputGainArea.removeFromTop(250);
    auto inputMeterArea = inputControlsArea.removeFromRight(15);
    inputGainSlider.setBounds(inputControlsArea);
    
    if (inputLevelMeter)
        inputLevelMeter->setBounds(inputMeterArea);
    
    // Output Gain (right side) 
    auto outputGainArea = mainArea.removeFromRight(100); // Wider for meter
    outputGainLabel.setBounds(outputGainArea.removeFromTop(20));
    
    auto outputControlsArea = outputGainArea.removeFromTop(250);
    auto outputMeterArea = outputControlsArea.removeFromLeft(15);
    outputGainSlider.setBounds(outputControlsArea);
    
    if (outputLevelMeter)
        outputLevelMeter->setBounds(outputMeterArea);
    
    // Multi-band area (center)
    auto bandsArea = mainArea.reduced(10, 0);
    auto originalBandsArea = bandsArea; // Save for spectrum display
    int bandWidth = bandsArea.getWidth() / DynamicEQ::CURRENT_BANDS;
    
    // Calculate required height based on band components
    int maxRequiredHeight = 280; // Base height
    for (int i = 0; i < DynamicEQ::CURRENT_BANDS; ++i)
    {
        if (bandComponents[i])
        {
            maxRequiredHeight = juce::jmax(maxRequiredHeight, bandComponents[i]->getRequiredHeight());
        }
    }
    
    // Ensure the editor window is tall enough
    int requiredEditorHeight = 70 + 20 + maxRequiredHeight + 40; // title + margins + band height + bottom margin
    if (getHeight() < requiredEditorHeight)
    {
        setSize(getWidth(), requiredEditorHeight);
        return; // resized() will be called again with the new size
    }
    
    for (int i = 0; i < DynamicEQ::CURRENT_BANDS; ++i)
    {
        if (bandComponents[i])
        {
            auto bandArea = bandsArea.removeFromLeft(bandWidth);
            if (i < DynamicEQ::CURRENT_BANDS - 1) bandArea = bandArea.withTrimmedRight(5); // Add spacing between bands
            bandComponents[i]->setBounds(bandArea);
        }
    }
    
    // Old spectrum display not used in new design (keep hidden)
    if (spectrumDisplay)
    {
        spectrumDisplay->setBounds(0, 0, 0, 0); // Hide by making it invisible
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

void VaclisDynamicEQAudioProcessorEditor::timerCallback()
{
    // Update level meters with current audio levels
    if (inputLevelMeter)
    {
        inputLevelMeter->updateLevel(audioProcessor.getInputLevel());
    }
    
    if (outputLevelMeter)
    {
        outputLevelMeter->updateLevel(audioProcessor.getOutputLevel());
    }
}