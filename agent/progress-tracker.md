# Standalone Python Extractor Implementation Plan

## Overview
Convert Python feature extraction into standalone executable that communicates with C++ plugin via pipes.

## Architecture
```
C++ Plugin <--JSON/stdin/stdout--> Python Executable
```

## Implementation Steps

### 1. Create Standalone Python Script
- Extract Python code from PythonFeatureExtractor.cpp
- Add JSON-based communication protocol
- Support daemon mode for persistent operation
- Base64 encode audio data for transfer

### 2. Build with PyInstaller
```bash
pyinstaller --onefile --name vtr-feature-extractor feature_extractor.py
```
- Creates single executable (~40-50MB)
- Includes Python runtime + librosa
- Works without Python installation

### 3. C++ Integration
- Create ExternalFeatureExtractor class
- Process management (start/stop/monitor)
- Communication via stdin/stdout pipes
- Timeout handling for reliability

### 4. Benefits
- **No Python Required**: Users don't need Python installed
- **100% Accuracy**: Uses exact librosa implementation
- **Clean Separation**: Plugin and extractor are independent
- **Easy Updates**: Can update extractor separately

## Communication Protocol
```json
Request:  {"audio_data": "base64...", "sr": 44100}
Response: {"features": [0.1, 0.2, ...], "status": "success"}
```

## Deployment
- Bundle executable with plugin
- Place in Resources folder
- Auto-discover at runtime