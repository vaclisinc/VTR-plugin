# Step 1 Requirements - Project Setup & Audio Pass-Through

## User Requirements Gathered: 2025-07-03

### Platform & Development
- **Development Platform**: macOS
- **Target Platforms**: Cross-platform (macOS primary, Windows secondary)
- **Testing DAW**: REAPER
- **Project Name**: vaclis-DynamicEQ

### Plugin Format Support
- **VST3**: Yes
- **AU (Audio Units)**: Yes
- **Rationale**: Both formats for maximum compatibility, JUCE handles both easily

### Audio Specifications
- **Sample Rate Support**: All common rates (44.1k, 48k, 88.2k, 96k, 192k)
- **Channel Configuration**: Both mono and stereo
- **Rationale**: Standard professional audio requirements

### Implementation Notes
- User confirmed understanding that plugin will work cross-platform once built
- JUCE framework chosen for cross-platform compatibility
- Initial focus on macOS development and testing
- REAPER selected as primary testing environment

### Next Steps
1. Create JUCE project structure
2. Configure for VST3 + AU builds
3. Implement basic audio pass-through
4. Test loading and audio flow in REAPER