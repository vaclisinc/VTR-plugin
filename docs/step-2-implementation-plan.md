# Step 2 Implementation Plan - Parameter System Foundation

## Overview
Implement AudioProcessorValueTreeState with Input and Output Gain parameters, including parameter smoothing for professional audio quality.

## Subtasks (30-60 min each)

### Subtask 1: Add APVTS Infrastructure (45 min)
- **Goal**: Add AudioProcessorValueTreeState to DynamicEQProcessor
- **Deliverables**:
  - APVTS member variable in header
  - Constructor initialization with parameter layout
  - getStateInformation/setStateInformation implementation
- **Files to modify**: `Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`
- **Testing**: Plugin loads without crashes, state saving works

### Subtask 2: Create Parameter Layout (30 min)
- **Goal**: Define Input and Output Gain parameters with proper ranges
- **Deliverables**:
  - createParameterLayout() function
  - Input Gain: -24dB to +12dB, default 0dB
  - Output Gain: -24dB to +12dB, default 0dB
  - String-based parameter IDs for future expansion
- **Files to modify**: `Source/PluginProcessor.cpp`
- **Testing**: Parameters appear in DAW automation list

### Subtask 3: Implement Parameter Smoothing (45 min)
- **Goal**: Add SmoothedValue objects for smooth parameter changes
- **Deliverables**:
  - SmoothedValue<float> for input and output gain
  - 20-50ms smoothing time (user specified)
  - Sample rate dependent smoothing setup
- **Files to modify**: `Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`
- **Testing**: Parameter changes sound smooth, no clicks/pops

### Subtask 4: Apply Gain Processing (30 min)
- **Goal**: Apply smoothed gain values in processBlock
- **Deliverables**:
  - Get current parameter values from APVTS
  - Apply input gain to incoming audio
  - Apply output gain to outgoing audio
  - Convert dB to linear gain properly
- **Files to modify**: `Source/PluginProcessor.cpp`
- **Testing**: Gain changes audibly affect signal level

### Subtask 5: Add Parameter Value Formatters (30 min)
- **Goal**: Create user-friendly parameter display
- **Deliverables**:
  - Custom stringFromValue functions for dB display
  - valueFromString functions for text input
  - Proper unit suffixes ("dB")
- **Files to modify**: `Source/PluginProcessor.cpp`
- **Testing**: Parameters display as "0.0 dB" in DAW

## Architecture Decisions

### Parameter Management
```cpp
class DynamicEQProcessor {
private:
    AudioProcessorValueTreeState parameters;
    
    // Smoothed parameter values
    SmoothedValue<float> inputGainSmoothed;
    SmoothedValue<float> outputGainSmoothed;
    
    // Parameter pointers for efficiency
    std::atomic<float>* inputGainParam;
    std::atomic<float>* outputGainParam;
};
```

### Parameter IDs
- Use descriptive, expandable naming: "input_gain", "output_gain"
- Future bands: "band1_freq", "band1_gain", "band1_q"
- Dynamics: "band1_threshold", "band1_ratio"

### Forward Compatibility
- Design parameter layout function for easy expansion
- Use consistent naming conventions
- Plan for parameter grouping/organization

## Potential Challenges
1. **Parameter smoothing timing**: Ensure 20-50ms feels natural
2. **Thread safety**: APVTS handles this, but verify atomic access
3. **State restoration**: Test saving/loading parameter states

## Success Criteria
- [ ] Plugin loads with parameters visible in DAW
- [ ] Input/Output gain controls work smoothly
- [ ] No audio artifacts during parameter changes
- [ ] Parameter automation records/plays back correctly
- [ ] State saving/loading preserves parameter values
- [ ] Parameters display with proper dB formatting

## Estimated Time: 3 hours total