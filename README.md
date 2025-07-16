# VTR-smartEQ

A smart dynamic EQ plugin with AI-powered Vocal Tone Recognition (VTR) for automatic EQ settings based on reference audio analysis.

![VTR-smartEQ preview](https://raw.githubusercontent.com/vaclisinc/VTR-plugin/b6108b48426ac23cf7d8ee835bbb6c0641afdd4e/fiinish.gif)
## Features

### 1.  5-Band Dynamic EQ
- **Frequency Bands**: SUB (20-150Hz), LOW (150-400Hz), MID (400-3kHz), HIGH-MID (3-6kHz), HIGH (6-20kHz)
- **Filter Types**: Bell, High Shelf, Low Shelf, High Pass, Low Pass
- **Dynamic Processing**: Per-band compression/expansion with sidechain support
- **Solo/Mute**: Exclusive solo mode with EN state restoration

### 2. AI-Powered VTR (Vaclis Tone Replication)
- **Automatic EQ Matching**: Analyzes reference audio and applies optimal EQ settings
- **Neural Network**: Custom-trained model for vocal tone analysis
- **One-Click Operation**: Load reference → Apply VTR → Done

### 3. Visual Feedback
- **Real-time Spectrum Analyzer**: Dual spectrum display (input/output)
- **Frequency Response Display**: Interactive EQ curve with draggable points
- **Level Meters**: Input/output VU meters
- **Filter Type Indicators**: Visual feedback for active filter types

### 🔌 Plugin Formats
- VST3
- AU (Audio Unit)

## System Requirements

- **OS**: macOS 10.13+ (Intel/Apple Silicon)
- **DAW**: Any VST3 or AU compatible host
- **Python**: 3.8+ (for VTR feature extraction)
- **Dependencies**: NumPy, SciPy, Librosa (automatically handled)

## Installation

1. Download the latest release from the [Releases](https://github.com/yourusername/VTR-plugin/releases) page
2. Copy the plugin to your system's plugin folder:
   - **VST3**: `~/Library/Audio/Plug-Ins/VST3/`
   - **AU**: `~/Library/Audio/Plug-Ins/Components/`
3. Restart your DAW and scan for new plugins

## Usage

### Basic EQ Operation
1. Load the plugin on your track
2. Enable bands using the EN buttons
3. Adjust frequency, gain, and Q for each band
4. Select filter types (Bell, Shelf, Pass) as needed

### Using VTR (Vocal Tone Recognition)
1. Click "Load Reference & Apply VTR"
2. Select a reference audio file (WAV, MP3, FLAC, AIFF, M4A)
3. Wait for analysis (typically 2-5 seconds)
4. VTR automatically applies optimal EQ settings

### Dynamic Processing
1. Click the dynamics toggle button (↓) on any band
2. Adjust threshold, ratio, attack, release, and knee
3. Choose detection type (Peak/RMS/Blend)
4. Select mode (Compressive/Expansive/De-esser/Gate)

### Solo Mode
- Click SOLO to isolate a band
- Other bands are automatically muted
- Original EN states are restored when solo is disabled

## Technical Details

### VTR Model Architecture
- **Feature Extraction**: 193-dimensional feature vector including:
  - Spectral features (centroid, rolloff, flux, etc.)
  - Tonal features (chroma, tonnetz)
  - Rhythm features (tempo, onset strength)
  - Statistical features (mean, variance, skewness, kurtosis)
- **Neural Network**: 3-layer MLP (193→128→64→5)
- **Output**: 5 gain predictions for each frequency band

### Audio Processing
- **Sample Rates**: 44.1kHz, 48kHz, 96kHz
- **Buffer Sizes**: 64-2048 samples
- **Latency**: 0 samples (no lookahead)
- **Processing**: 64-bit internal precision

### Dependencies
- JUCE Framework 7.0.5+
- chowdsp_utils (DSP utilities)
- Python 3.8+ with:
  - NumPy
  - SciPy
  - Librosa
  - scikit-learn

## Building from Source

### Prerequisites
- CMake 3.22+
- Xcode 12+ (macOS)
- Python 3.8+ with required packages

### Build Steps
```bash
# Clone the repository
git clone --recursive https://github.com/vaclisinc/VTR-plugin.git
cd VTR-plugin

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . --config Release

# Plugins will be automatically installed to system folders
```

### Testing
```bash
# Run validation tests
pluginval --strictness 5 --validate-in-process ~/Library/Audio/Plug-Ins/VST3/VTR-smartEQ.vst3
```

## Troubleshooting

### VTR Not Working
- Ensure Python 3.8+ is installed
- Check Python packages: `pip install numpy scipy librosa scikit-learn`
- Verify model files exist in `vtr-model/exported_model/`

### Plugin Not Showing in DAW
- Rescan plugins in your DAW
- Check plugin format compatibility
- Verify installation path is correct

### Audio Glitches
- Increase buffer size in DAW
- Disable other CPU-intensive plugins
- Check sample rate compatibility

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests with pluginval
5. Submit a pull request

## License

This project is licensed under the GPL-3.0 License - see the [LICENSE](LICENSE) file for details.
