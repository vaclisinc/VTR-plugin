# Dynamic EQ Plugin

A professional multi-band dynamic EQ plugin built with JUCE and modular architecture.

## Quick Start

### Prerequisites
- CMake 3.22+
- C++17 compatible compiler
- macOS (tested) / Windows / Linux

### Build Instructions

1. **Clone and initialize submodules:**
```bash
git clone https://github.com/vaclisPG/vaclis-DynamicEQ.git
cd vac-first-plugin
git submodule update --init --recursive
```

2. **Configure build:**
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
```

3. **Build plugin:**
```bash
cmake --build build --config Debug
```

4. **Clean build (if needed):**
```bash
# Partial clean:
cmake --build build --target clean

# Complete clean:
rm -rf build
```

### Plugin Formats
- **AU**: `~/Library/Audio/Plug-Ins/Components/vaclis-DynamicEQ.component`
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/vaclis-DynamicEQ.vst3`

## Current Features (Step 3.1)

### Audio Processing
- Professional Bell/Peak EQ filter using JUCE DSP
- Input and output gain controls with smart limiting
- Real-time parameter smoothing (30ms)
- Scalable parameter management system

### GUI Controls
- **Input Gain**: Audio input level control
- **EQ Freq**: Filter frequency (20Hz - 20kHz, logarithmic)
- **EQ Gain**: Filter gain (±12dB, linear)
- **EQ Q**: Filter Q factor (0.1 - 10, logarithmic)
- **Output Gain**: Audio output level control

### Technical Specs
- **Sample Rates**: 44.1kHz - 192kHz
- **Channels**: Mono/Stereo
- **Latency**: Zero latency
- **CPU Usage**: Minimal (single biquad per channel)

## Development Status

- ✅ **Step 1**: JUCE project setup with audio pass-through
- ✅ **Step 2**: Scalable parameter system architecture  
- ✅ **Step 3**: Single band EQ with professional filters
- ✅ **Step 3.1**: Modular architecture refactor
- ⏳ **Step 4**: Multiple filter types (Bell, Shelf, Pass)
- ⏳ **Step 5**: Multi-band system (4+ bands)
- ⏳ **Step 6**: Basic GUI with frequency response display
- ⏳ **Step 7**: Add dynamics (per-band compression)
- ⏳ **Step 8**: Enhanced features & visualization
- ⏳ **Step 9**: Optimization
- ⏳ **Step 10**: Testing & polish

## Architecture

### Modular Design
```
Source/
├── PluginProcessor.h/cpp     # Main audio processor (310 lines)
├── PluginEditor.h/cpp        # GUI interface
├── Parameters/
│   └── ParameterManager.*    # Scalable parameter management
└── DSP/
    ├── EQBand.*             # Professional EQ filtering
    └── GainProcessor.*      # High-quality gain processing
```

### Key Benefits
- **Scalable**: Ready for 50+ parameters and multiple bands
- **Professional**: Industry-standard DSP quality
- **Maintainable**: Single responsibility per component
- **Expandable**: Clean architecture for future features

## Testing

Load the plugin in your DAW (tested with REAPER):
1. Create audio track with signal
2. Load "vaclis-DynamicEQ" plugin
3. Adjust EQ parameters to hear frequency response changes
4. Test gain controls for level management

## Dependencies

- **JUCE Framework 7.0.9**: Core audio plugin framework 
- **chowdsp_utils**: Professional DSP library (integrated as submodule)

## Contributing

This is a step-by-step educational project. Each step builds upon the previous with:
- Detailed documentation in `docs/`
- Progress tracking in `progress-tracker.md`
- Clean git history with descriptive commits

## License

[Add your license here]