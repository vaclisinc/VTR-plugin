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
    
    // Dynamics bypass button
    dynamicsBypassButton.setButtonText("BYPASS");
    dynamicsBypassButton.setToggleable(true);
    dynamicsBypassButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    dynamicsBypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    addAndMakeVisible(dynamicsBypassButton);
    
    // Initially hide combo boxes and bypass button
    detectionTypeCombo.setVisible(false);
    detectionLabel.setVisible(false);
    modeCombo.setVisible(false);
    modeLabel.setVisible(false);
    dynamicsBypassButton.setVisible(false);
    
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
    
    dynamicsBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "dyn_bypass_band" + bandSuffix, dynamicsBypassButton);
    
    // Initialize bypass button state from parameter (should be true by default)
    juce::String bypassParamID = "dyn_bypass_band" + bandSuffix;
    auto* bypassParam = audioProcessor.getValueTreeState().getParameter(bypassParamID);
    if (bypassParam)
    {
        float bypassValue = bypassParam->getValue();
        dynamicsBypassButton.setToggleState(bypassValue > 0.5f, juce::dontSendNotification);
    }
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
    dynamicsBypassButton.setVisible(dynamicsExpanded);
    
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
    int dynamicsHeight = dynamicsExpanded ? 155 : 0; // Additional height for dynamics (increased for better text display)
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
    
    // Dynamics controls (only layout if expanded)
    if (dynamicsExpanded)
    {
        bounds.removeFromTop(10); // spacing
        
        // Row 1: Threshold, Ratio, Attack, Release (4 rotary sliders)
        auto dynamicsRow1LabelArea = bounds.removeFromTop(15); // Taller for better text display
        auto dynamicsRow1SliderArea = bounds.removeFromTop(70); // Taller for text boxes
        
        int dynamicsControlWidth = 50; // Wider for better text display
        int dynamicsSpacing = (bounds.getWidth() - (dynamicsControlWidth * 4)) / 5;
        
        // Position first row labels and sliders
        int currentX = dynamicsSpacing;
        
        thresholdLabel.setBounds(currentX, dynamicsRow1LabelArea.getY(), dynamicsControlWidth, dynamicsRow1LabelArea.getHeight());
        thresholdSlider.setBounds(currentX, dynamicsRow1SliderArea.getY(), dynamicsControlWidth, dynamicsRow1SliderArea.getHeight());
        currentX += dynamicsControlWidth + dynamicsSpacing;
        
        ratioLabel.setBounds(currentX, dynamicsRow1LabelArea.getY(), dynamicsControlWidth, dynamicsRow1LabelArea.getHeight());
        ratioSlider.setBounds(currentX, dynamicsRow1SliderArea.getY(), dynamicsControlWidth, dynamicsRow1SliderArea.getHeight());
        currentX += dynamicsControlWidth + dynamicsSpacing;
        
        attackLabel.setBounds(currentX, dynamicsRow1LabelArea.getY(), dynamicsControlWidth, dynamicsRow1LabelArea.getHeight());
        attackSlider.setBounds(currentX, dynamicsRow1SliderArea.getY(), dynamicsControlWidth, dynamicsRow1SliderArea.getHeight());
        currentX += dynamicsControlWidth + dynamicsSpacing;
        
        releaseLabel.setBounds(currentX, dynamicsRow1LabelArea.getY(), dynamicsControlWidth, dynamicsRow1LabelArea.getHeight());
        releaseSlider.setBounds(currentX, dynamicsRow1SliderArea.getY(), dynamicsControlWidth, dynamicsRow1SliderArea.getHeight());
        
        bounds.removeFromTop(5); // spacing
        
        // Row 2: Knee, Detection, Mode
        auto dynamicsRow2LabelArea = bounds.removeFromTop(15); // Taller for better text display
        auto dynamicsRow2ControlArea = bounds.removeFromTop(50); // Taller for better control display
        
        int row2ControlWidth = 65; // Wider for better text display
        int row2Spacing = (bounds.getWidth() - (row2ControlWidth * 3)) / 4;
        currentX = row2Spacing;
        
        kneeLabel.setBounds(currentX, dynamicsRow2LabelArea.getY(), row2ControlWidth, dynamicsRow2LabelArea.getHeight());
        kneeSlider.setBounds(currentX, dynamicsRow2ControlArea.getY(), row2ControlWidth, 45);
        currentX += row2ControlWidth + row2Spacing;
        
        detectionLabel.setBounds(currentX, dynamicsRow2LabelArea.getY(), row2ControlWidth, dynamicsRow2LabelArea.getHeight());
        detectionTypeCombo.setBounds(currentX, dynamicsRow2ControlArea.getY(), row2ControlWidth, 22); // Taller combo box
        currentX += row2ControlWidth + row2Spacing;
        
        modeLabel.setBounds(currentX, dynamicsRow2LabelArea.getY(), row2ControlWidth, dynamicsRow2LabelArea.getHeight());
        modeCombo.setBounds(currentX, dynamicsRow2ControlArea.getY(), row2ControlWidth, 22); // Taller combo box
        
        bounds.removeFromTop(5); // spacing
        
        // Row 3: Bypass button
        auto bypassArea = bounds.removeFromTop(25);
        int bypassWidth = 80;
        int bypassX = (bounds.getWidth() - bypassWidth) / 2;
        dynamicsBypassButton.setBounds(bypassX, bypassArea.getY(), bypassWidth, bypassArea.getHeight());
    }
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
    g.drawFittedText ("Dynamic EQ - Step 7 Complete (No Default Presets)", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);
    
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