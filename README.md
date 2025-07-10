# vaclis-DynamicEQ Plugin 🎛️

A professional 4-band dynamic EQ plugin built with JUCE framework, featuring real-time spectrum analysis, advanced visualization, and professional-grade audio processing.

## 🚀 Features

### Core Audio Processing
- **4-Band Dynamic EQ System**: Professional multi-band dynamic equalization
- **Professional Filters**: Bell, High/Low Shelf, High/Low Pass filters using chowdsp_utils
- **Dynamics Processing**: Complete compressor engine with threshold, ratio, attack, release
- **Sidechain Support**: External sidechain input capability
- **High-Quality DSP**: Industry-standard audio processing

### Advanced Visualization
- **Real-time Spectrum Analyzer**: 2048-point FFT with 30Hz update rate
- **Frequency Response Display**: Live EQ curve visualization
- **Level Meters**: Professional input/output meters with peak hold
- **Band-specific Controls**: Color-coded interface for intuitive operation

### Professional GUI
- **Resizable Interface**: Optimized 730px height layout
- **Rotary Controls**: Professional 65px rotary sliders
- **Toggle Buttons**: Enable/Solo/Dynamics/Spectrum/Sidechain controls
- **Collapsible Interface**: Organized layout with logical grouping

## 📥 Installation

### Option 1: Download Pre-built Binaries (Recommended)
1. Go to [Releases](https://github.com/vaclisPG/vaclis-DynamicEQ/releases)
2. Download the latest release for your platform
3. Extract and copy to your plugin directories:
   - **macOS AU**: `~/Library/Audio/Plug-Ins/Components/`
   - **macOS VST3**: `~/Library/Audio/Plug-Ins/VST3/`
   - **Windows VST3**: `C:\Program Files\Common Files\VST3\`

### Option 2: Build from Source

#### Prerequisites
- CMake 3.22+
- C++17 compatible compiler
- macOS (tested) / Windows / Linux

#### Build Instructions
1. **Clone and initialize submodules:**
```bash
git clone https://github.com/vaclisPG/vaclis-DynamicEQ.git
cd vaclis-DynamicEQ
git submodule update --init --recursive
```

2. **Configure build:**
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

3. **Build plugin:**
```bash
cmake --build build --config Release
```

4. **Install plugin:**
```bash
cmake --install build
```

## 🎵 Usage Guide

### Basic Operation

1. **Load the Plugin**
   - Open your DAW (tested with REAPER, Logic Pro, Ableton Live)
   - Create an audio track with your source material
   - Load "vaclis-DynamicEQ" from your plugin list

2. **Understanding the Interface**
   - **Top Section**: Frequency response display shows your EQ curve
   - **Middle Section**: Real-time spectrum analyzer (toggle with SPEC button)
   - **Bottom Sections**: 4 bands of EQ and dynamics controls
   - **Side Panels**: Input/output level meters

### EQ Controls (Per Band)

- **FREQ**: Frequency center point (20Hz - 20kHz)
- **GAIN**: Boost/cut amount (±18dB)
- **Q**: Bandwidth/resonance (0.1 - 10)
- **Filter Type Buttons**: Bell, High/Low Shelf, High/Low Pass
- **Enable/Solo**: Band on/off and isolation

### Dynamics Controls (Per Band)

- **THRESH**: Compression threshold (-60dB to 0dB)
- **RATIO**: Compression ratio (1:1 to ∞:1)
- **ATTACK**: Response time (0.1ms - 100ms)
- **RELEASE**: Recovery time (1ms - 1000ms)
- **KNEE**: Soft/hard compression knee
- **DYN**: Enable/disable dynamics processing

### Advanced Features

- **Sidechain**: Click SC button to enable external sidechain input
- **Spectrum Analyzer**: Click SPEC button to show/hide real-time spectrum
- **Solo**: Use SOLO buttons to isolate individual bands
- **Bypass**: Use enable buttons to bypass individual bands

### Typical Workflow

1. **Set up your EQ curve** using the frequency response display
2. **Enable dynamics** on bands that need compression
3. **Adjust thresholds** to set when compression kicks in
4. **Fine-tune ratios** for desired compression amount
5. **Use the spectrum analyzer** to visualize your changes
6. **Enable sidechain** if you want frequency-specific ducking

## 🔧 Technical Specifications

- **Audio Processing**: 32-bit floating point, up to 192kHz sample rate
- **Latency**: Near-zero latency processing
- **Format Support**: VST3, AU (Audio Unit)
- **Platform**: macOS, Windows (cross-platform)
- **Dependencies**: JUCE 8.0.8, chowdsp_utils

## 📊 Development Status

- ✅ **Step 1**: JUCE project setup with audio pass-through
- ✅ **Step 2**: Scalable parameter system architecture  
- ✅ **Step 3**: Single band EQ with professional filters
- ✅ **Step 4**: Multiple filter types (Bell, Shelf, Pass)
- ✅ **Step 5**: Multi-band system (4 bands)
- ✅ **Step 6**: Professional GUI with frequency response display
- ✅ **Step 7**: Complete dynamics processing
- ✅ **Step 8**: Enhanced features & visualization
- ✅ **PROJECT COMPLETE**: Ready for production use

## 🏗️ Architecture

```
Source/
├── PluginProcessor.h/cpp         # Main audio processor
├── PluginEditor.h/cpp            # GUI interface
├── DSP/
│   ├── EQBand.*                 # Individual band processing
│   ├── MultiBandEQ.*            # Multi-band coordinator
│   └── ParameterManager.*       # Parameter management
├── GUI/
│   ├── BandControlComponent.*   # Band-specific controls
│   ├── FrequencyResponseDisplay.* # EQ curve display
│   ├── SpectrumAnalyzer.*       # Real-time spectrum analysis
│   └── LevelMeter.*             # Level metering
└── docs/                        # Complete documentation
```

## 🧪 Testing

The plugin has been tested with:
- **REAPER** (primary testing platform)
- **Logic Pro** (macOS)
- **Ableton Live** (macOS/Windows)
- **Pro Tools** (basic compatibility)

## 📝 Documentation

Complete technical documentation available in `docs/`:
- Step-by-step implementation guides
- Architecture documentation
- API references
- Development progress tracking

## 🤝 Contributing

This project showcases professional plugin development practices:
- Clean, modular architecture
- Comprehensive documentation
- Step-by-step development process
- Professional coding standards

## 📄 License

This project is licensed under the **Beer-Ware License** (Revision 42) 🍺

> As long as you retain this notice you can do whatever you want with this stuff. 
> If we meet some day, and you think this stuff is worth it, you can buy me a beer in return.

See LICENSE file for the full license text.

## 🙏 Acknowledgments

- **JUCE Framework**: Core audio plugin framework
- **chowdsp_utils**: Professional DSP library
- **Community**: Open source audio development community

---

**Made with ❤️ for the audio production community**

*For support, questions, or feature requests, please open an issue on GitHub.*