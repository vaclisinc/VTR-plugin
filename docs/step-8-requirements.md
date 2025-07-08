# Step 8 Requirements - Enhanced Features & Visualization

## User Requirements (Provided 2025-07-08)

### Spectrum Analyzer Preferences:
- **FFT size**: 2048
- **Window type**: Hann window
- **Update rate**: 30Hz
- **Peak hold**: Yes, with 2-second decay

### Sidechain Support:
- **External sidechain input**: Yes
- **Per-band sidechain filtering**: No
- **Sidechain listen mode**: No
- **Note**: User mentioned some features might already be finished

### Additional Meters:
- **Input/output spectrum**: Both, user-selectable
- **Correlation meter**: No
- **Peak/RMS meters**: Yes, simple input/output meters

### Visual Integration:
- **Spectrum analyzer placement**: Overlay on frequency response
- **Color scheme**: Complementary to band colors
- **Display priority**: EQ curve primary, spectrum secondary

## Current Implementation Status (Analysis)

### Already Implemented:
- Multi-band dynamic EQ core (4 bands)
- Professional chowdsp filters and dynamics processing
- Complete GUI framework with collapsible sections
- chowdsp_visualizers components available (SpectrumPlotBase, EqualizerPlot, etc.)

### Not Yet Implemented:
- Spectrum analyzer (FFT analysis system)
- ff_meters integration
- Sidechain support (external input routing)
- Input/output level meters
- Real-time frequency response plotting

## Technical Implementation Notes

### Spectrum Analyzer:
- Use JUCE's FFT with 2048 samples
- Implement Hann windowing
- 30Hz update rate (timer-based)
- Peak hold with 2-second decay buffer

### Sidechain:
- Add external input routing capability
- Integrate with existing dynamics processing
- No additional filtering or listen mode needed

### Metering:
- Simple input/output level meters
- User-selectable spectrum display (input/output/both)
- No correlation metering required

### Visual Design:
- Overlay spectrum on existing frequency response
- Use complementary colors to band colors
- Maintain EQ curve visual priority
- Keep spectrum as secondary background element