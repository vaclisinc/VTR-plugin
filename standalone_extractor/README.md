# VTR Standalone Feature Extractor

This directory contains the standalone Python feature extractor that allows the VTR-smartEQ plugin to work without requiring Python installation on the end user's system.

## Overview

The feature extractor is a Python script that gets compiled into a standalone executable using PyInstaller. It communicates with the C++ plugin via JSON messages over stdin/stdout pipes.

## Building the Executable

1. Ensure you have Python 3.8+ installed
2. Run the build script:
   ```bash
   ./build.sh
   ```

This will:
- Create a virtual environment
- Install all dependencies (librosa, numpy, etc.)
- Build the standalone executable using PyInstaller
- Test the executable
- Output: `dist/vtr-feature-extractor` (~40-50MB)

## Testing

Test the standalone executable:
```bash
./dist/vtr-feature-extractor --test
```

## Integration with Plugin

The plugin will automatically find and use the executable from:
1. Plugin's Resources folder (for deployed plugins)
2. `standalone_extractor/dist/` (for development)
3. System PATH

## Communication Protocol

**Request Format:**
```json
{
    "audio_data": "base64_encoded_float32_array",
    "sr": 44100
}
```

**Response Format:**
```json
{
    "status": "success",
    "features": [0.1, 0.2, ...],  // 17 features
    "feature_names": ["spectral_centroid", ...]
}
```

## Features Extracted

1. Spectral Centroid
2. Spectral Bandwidth
3. Spectral Rolloff
4. MFCC 1-13 (13 features)
5. RMS Energy

Total: 17 features matching the VTR model requirements.

## Deployment

For plugin distribution:
1. Build the executable for each platform (macOS Intel, macOS ARM)
2. Sign the executable with your developer certificate
3. Include in plugin's Resources folder
4. The plugin will automatically find and use it