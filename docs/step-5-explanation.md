# Step 5: Multi-Band EQ System - Code Explanation

## Overview
Step 5 transforms the single-band Dynamic EQ into a professional 4-band system with individual enable/disable and solo functionality. This implementation creates a scalable architecture ready for future expansion to 8+ bands.

## Architecture Changes

### 1. Constants and Scalability (`Source/DSP/EQBand.h`)

```cpp
namespace DynamicEQ {
    static constexpr int MAX_BANDS = 8;      // Future expansion ready
    static constexpr int CURRENT_BANDS = 4;  // Current implementation
}
```

**Purpose**: Eliminates magic numbers and enables easy expansion. To scale to 8 bands, simply change `CURRENT_BANDS = 8`.

### 2. Parameter System Extension (`Source/PluginProcessor.cpp`)

#### Parameter Layout Creation
- **Total Parameters**: Expanded from 8 to 26 parameters
- **Parameter Organization**: Systematic parameter ID naming for multi-band support

```cpp
// Multi-band EQ parameters with different default frequencies for each band
const float defaultFreqs[DynamicEQ::CURRENT_BANDS] = {100.0f, 500.0f, 2000.0f, 8000.0f};
const juce::String bandNames[DynamicEQ::CURRENT_BANDS] = {"LOW", "LOW-MID", "HIGH-MID", "HIGH"};

for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
{
    // EQ parameters for each band
    addFrequencyParameter(layout, "eq_freq_band" + juce::String(band), 
                         "EQ Frequency " + bandNames[band], defaultFreqs[band]);
    addGainParameter(layout, "eq_gain_band" + juce::String(band), 
                    "EQ Gain " + bandNames[band], 0.0f);
    addQParameter(layout, "eq_q_band" + juce::String(band), 
                 "EQ Q " + bandNames[band], 1.0f);
    addFilterTypeParameter(layout, "eq_type_band" + juce::String(band), 
                          "EQ Type " + bandNames[band], 0.0f);
    
    // Boolean parameters (not smoothed)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "eq_enable_band" + juce::String(band),
        "Enable " + bandNames[band], true));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "eq_solo_band" + juce::String(band),
        "Solo " + bandNames[band], false));
}
```

**Key Design Decision**: Boolean parameters (enable/solo) are NOT added to ParameterManager because they cannot be smoothed. They're accessed directly via AudioProcessorValueTreeState.

#### Parameter Manager Setup
```cpp
// Add smoothable parameters to ParameterManager in correct order
for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
    parameterManager.addParameter("eq_freq_band" + juce::String(band), parameters);
for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band) 
    parameterManager.addParameter("eq_gain_band" + juce::String(band), parameters);
for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
    parameterManager.addParameter("eq_q_band" + juce::String(band), parameters);
for (int band = 0; band < DynamicEQ::CURRENT_BANDS; ++band)
    parameterManager.addParameter("eq_type_band" + juce::String(band), parameters);
```

**Purpose**: Organized parameter grouping enables efficient caching and reduces real-time string lookups.

### 3. Multi-Band Audio Processing (`Source/DSP/EQBand.cpp`)

#### MultiBandEQ Class Architecture
```cpp
class MultiBandEQ
{
    std::vector<std::unique_ptr<EQBand>> bands;
    ParameterManager* parameterManager = nullptr;
    juce::AudioProcessorValueTreeState* valueTreeState = nullptr;  // For boolean parameters
};
```

#### Band Enable/Solo Logic
```cpp
void MultiBandEQ::processBuffer(juce::AudioBuffer<float>& buffer)
{
    // Update all band parameters first
    for (auto& band : bands)
        if (band) band->updateParameters();
    
    // Check if any band is soloed
    bool anyBandSoloed = false;
    for (int i = 0; i < static_cast<int>(bands.size()); ++i)
    {
        if (bands[i] && isBandSoloed(i))
        {
            anyBandSoloed = true;
            break;
        }
    }
    
    // Process based on solo/enable state
    if (anyBandSoloed)
    {
        // Only process soloed bands
        for (int i = 0; i < static_cast<int>(bands.size()); ++i)
            if (bands[i] && isBandSoloed(i))
                bands[i]->processBuffer(buffer);
    }
    else
    {
        // Process all enabled bands
        for (int i = 0; i < static_cast<int>(bands.size()); ++i)
            if (bands[i] && isBandEnabled(i))
                bands[i]->processBuffer(buffer);
    }
}
```

**Critical Implementation**: Solo mode overrides enable/disable states. When any band is soloed, only soloed bands process audio.

#### Boolean Parameter Access
```cpp
bool MultiBandEQ::isBandEnabled(int bandIndex) const
{
    if (!valueTreeState || bandIndex < 0 || bandIndex >= static_cast<int>(bands.size()))
        return true;  // Default enabled when no state available
    
    juce::String paramID = "eq_enable_band" + juce::String(bandIndex);
    if (auto* param = dynamic_cast<juce::AudioParameterBool*>(valueTreeState->getParameter(paramID)))
        return param->get();
    return true;
}
```

**Design Pattern**: Direct boolean parameter access via AudioProcessorValueTreeState, not through ParameterManager.

### 4. Multi-Band UI Architecture (`Source/PluginEditor.cpp`)

#### Component-Based Design
```cpp
class BandControlComponent : public juce::Component
{
    int bandIndex;
    juce::String bandName;
    VaclisDynamicEQAudioProcessor& audioProcessor;
    
    // Controls
    juce::TextButton enableButton, soloButton;
    juce::Label bandLabel, frequencyRangeLabel;
    juce::Slider freqSlider, gainSlider, qSlider;
    std::vector<FilterTypeButton> filterTypeButtons;
};
```

#### Layout: EN | BAND-NAME | SOLO
```cpp
// EN | BAND-NAME | SOLO layout  
auto topArea = bounds.removeFromTop(25);
int buttonWidth = topArea.getWidth() / 3;  // Divide into three equal parts

enableButton.setBounds(topArea.removeFromLeft(buttonWidth));
bandLabel.setBounds(topArea.removeFromLeft(buttonWidth));
soloButton.setBounds(topArea.removeFromLeft(buttonWidth));
```

**UI Innovation**: Horizontal layout with band name centered between enable/solo buttons for optimal space usage.

#### Manual Button Handling
```cpp
enableButton.onClick = [this]() {
    bool newState = !enableButton.getToggleState();
    enableButton.setToggleState(newState, juce::dontSendNotification);
    
    juce::String paramID = "eq_enable_band" + juce::String(bandIndex);
    if (auto* param = dynamic_cast<juce::AudioParameterBool*>(
        audioProcessor.getValueTreeState().getParameter(paramID)))
    {
        *param = newState;
    }
};
```

**Technical Solution**: Manual button handling was necessary because ButtonAttachment had issues with boolean parameters. Direct parameter updates ensure reliable functionality.

#### Scalable Component Array
```cpp
std::array<std::unique_ptr<BandControlComponent>, DynamicEQ::CURRENT_BANDS> bandComponents;
```

**Expandability**: Simply changing `CURRENT_BANDS` automatically scales the UI.

### 5. EQBand Individual Processing (`Source/DSP/EQBand.cpp`)

#### Professional Filter Implementation
```cpp
class StereoBellFilter
{
    chowdsp::SVFBell<float> filterL, filterR;
public:
    void processStereo(float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i) {
            left[i] = filterL.processSample(0, left[i]);
            if (right != left) right[i] = filterR.processSample(0, right[i]);
        }
    }
};
```

**Quality Focus**: chowdsp_utils provides professional-grade filters for studio-quality sound.

#### Parameter Caching for Performance
```cpp
struct ParameterIndices
{
    int frequency = -1;
    int gain = -1;
    int q = -1;
    int type = -1;
    bool isValid() const { return frequency >= 0 && gain >= 0 && q >= 0 && type >= 0; }
} paramIndices;

void EQBand::cacheParameterIndices()
{
    for (size_t i = 0; i < manager->parameterIDs.size(); ++i)
    {
        if (manager->parameterIDs[i] == freqParamID) paramIndices.frequency = static_cast<int>(i);
        if (manager->parameterIDs[i] == gainParamID) paramIndices.gain = static_cast<int>(i);
        if (manager->parameterIDs[i] == qParamID) paramIndices.q = static_cast<int>(i);
        if (manager->parameterIDs[i] == typeParamID) paramIndices.type = static_cast<int>(i);
    }
}
```

**Performance Optimization**: Parameter indices are cached during setup to eliminate string lookups in real-time audio processing.

## Frequency Band Design

### Band Allocation
- **LOW**: 20Hz - 200Hz (Default: 100Hz)
- **LOW-MID**: 200Hz - 800Hz (Default: 500Hz)  
- **HIGH-MID**: 800Hz - 3kHz (Default: 2kHz)
- **HIGH**: 3kHz - 20kHz (Default: 8kHz)

### Filter Types Per Band
- **Bell**: Parametric EQ with gain and Q control
- **High Shelf**: High-frequency shelving filter
- **Low Shelf**: Low-frequency shelving filter  
- **High Pass**: Low-frequency attenuation
- **Low Pass**: High-frequency attenuation

## Key Design Patterns

### 1. Separation of Concerns
- **ParameterManager**: Handles smoothable parameters (freq, gain, Q, type)
- **AudioProcessorValueTreeState**: Handles boolean parameters (enable, solo)
- **MultiBandEQ**: Orchestrates audio processing logic
- **BandControlComponent**: Manages individual band UI

### 2. RAII and Smart Pointers
```cpp
std::vector<std::unique_ptr<EQBand>> bands;
std::array<std::unique_ptr<BandControlComponent>, DynamicEQ::CURRENT_BANDS> bandComponents;
```

### 3. Template-Ready Architecture
The codebase is designed for easy expansion:
- Change `CURRENT_BANDS` constant
- All loops automatically scale
- Parameter creation scales automatically
- UI components scale automatically

## Testing and Validation

### Audio Processing Tests
1. **Enable/Disable**: Verify bands stop processing when disabled
2. **Solo Functionality**: Confirm only soloed bands process audio
3. **Parameter Updates**: Ensure smooth parameter changes
4. **Filter Quality**: Validate professional audio quality

### UI Tests  
1. **Button Responsiveness**: Enable/Solo buttons update correctly
2. **Parameter Sync**: UI changes reflect in audio processing
3. **Layout Scaling**: Components resize properly
4. **Visual Feedback**: Button states clearly indicate status

## Performance Considerations

### Real-Time Safety
- Parameter caching eliminates string lookups
- Boolean parameter access is direct (no smoothing overhead)
- JUCE's AudioProcessorValueTreeState handles thread safety

### Memory Efficiency
- Smart pointers prevent memory leaks
- Component-based design minimizes redundancy
- Cached indices reduce computational overhead

### Scalability
- Constant-time parameter access
- Linear scaling with band count
- Modular architecture supports unlimited bands

## Future Expansion Path

### To 8 Bands
1. Change `CURRENT_BANDS = 8`
2. Add 4 more default frequencies
3. Add 4 more band names
4. Adjust UI sizing if needed

### Advanced Features Ready
- Crossover filtering between bands
- Dynamic band allocation
- Frequency analysis visualization
- Automation and MIDI control

This implementation provides a solid foundation for professional multi-band audio processing while maintaining clean, maintainable, and expandable code architecture.