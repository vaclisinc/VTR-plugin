# Step 1 Implementation Plan - Project Setup & Audio Pass-Through

## Overview
Set up JUCE project structure and implement basic audio pass-through functionality for the vaclis-DynamicEQ plugin.

## Subtasks (30-60 minutes each)

### Subtask 1: Create JUCE Project Structure (30 min)
- Create CMakeLists.txt for JUCE project
- Set up source directory structure
- Configure VST3 and AU plugin formats
- Set basic plugin metadata (name, version, etc.)

### Subtask 2: Implement Plugin Processor (45 min)
- Create PluginProcessor class
- Implement prepareToPlay() for sample rate handling
- Implement processBlock() for audio pass-through
- Handle mono/stereo channel configurations
- Add parameter initialization (empty for now)

### Subtask 3: Implement Basic Plugin Editor (30 min)
- Create PluginEditor class
- Set up minimal GUI (just a placeholder)
- Connect editor to processor
- Set plugin window size

### Subtask 4: Build Configuration & Testing (45 min)
- Configure build for macOS
- Build VST3 and AU versions
- Test plugin loads in REAPER
- Verify audio pass-through functionality
- Test with different sample rates

## Deliverables
- Working JUCE project that builds successfully
- VST3 and AU plugin files
- Plugin loads in REAPER without errors
- Audio passes through cleanly (no processing yet)
- Basic GUI window opens

## Potential Challenges
- JUCE installation/configuration issues
- Build system setup on macOS
- Plugin format registration
- REAPER plugin path configuration

## Success Criteria
- Plugin builds without errors
- Loads in REAPER plugin browser
- Audio input equals audio output (perfect pass-through)
- GUI opens and displays properly
- Works with both mono and stereo tracks

## Files to Create
- `Source/PluginProcessor.h`
- `Source/PluginProcessor.cpp`
- `Source/PluginEditor.h`
- `Source/PluginEditor.cpp`
- `CMakeLists.txt`