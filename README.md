# VTR-smartEQ

An dynamic EQ plugin integrating AI-powered Tone Replication model ([VTR](https://github.com/vaclisinc/Vaclis_Tone_Replication/)) for automatic EQ settings based on reference audio analysis.

![VTR-smartEQ preview](https://raw.githubusercontent.com/vaclisinc/VTR-smartEQ/43ab351f3b64555691a31cb9854238d170dd4448/finish.gif)

## Features

- **5-Band Dynamic EQ**: SUB, LOW, MID, HIGH-MID, HIGH frequency bands with multiple filter types
- **AI-Powered VTR**: Automatic EQ matching based on reference audio analysis
- **Real-time Visualization**: Spectrum analyzer and frequency response display
- **Plugin Formats**: VST3, AU (Audio Unit)

## System Requirements

- **OS**: macOS 10.13+ (Intel/Apple Silicon)
- **DAW**: Any VST3 or AU compatible host
- **Python**: 3.8+ (for VTR feature extraction)

## Installation

1. Download the latest release from the [Releases](https://github.com/vaclisinc/VTR-smartEQ/releases) page
2. Express the zip file and copy the plugin to your system's plugin folder:
   - **VST3**: `~/Library/Audio/Plug-Ins/VST3/`
   - **AU**: `~/Library/Audio/Plug-Ins/Components/`
3. Restart your DAW and scan for new plugins

## Usage Instructions

### Using VTR (Vaclis Tone Replication)

1. **Click "Load Reference & Apply VTR" to upload audio file**
   - Select reference audio file (supports WAV, MP3, FLAC, AIFF, M4A)
   - **Recommended to keep audio length short (<8s)** for better results
   - This model uses feature extraction as input, longer audio files may cause information confusion

2. **Wait for processing and automatic application**
   - Wait a moment for the system to automatically analyze and apply to the plugin
   - You can make any additional EQ adjustments to meet your specific goals

### Basic EQ Operation

1. Load the plugin on your track
2. Enable bands using the EN buttons
3. Adjust frequency, gain, and Q for each band
4. Select filter types (Bell, Shelf, Pass) as needed

### Dynamic Processing

1. Click the dynamics toggle button (↓) on any band
2. Adjust threshold, ratio, attack, release parameters
3. Choose detection type and processing mode

## Building from Source code

### Prerequisites
- CMake 3.22+
- Xcode 12+ (macOS)
- Python 3.8+ with required packages

### Build Steps
```bash
# Clone the repository
git clone --recursive https://github.com/vaclisinc/VTR-smartEQ.git
cd VTR-smartEQ

# Create build directory
mkdir build && cd build

# Configure and build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
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

## License

This project is licensed under the GPL-3.0 License - see the [LICENSE](LICENSE) file for details.
