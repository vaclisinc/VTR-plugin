# Cross-Platform Build Guide

## Platform-Specific Executables

PyInstaller creates platform-specific executables. You need to build separately for each platform:

### macOS
- **Intel (x86_64)**: Build on Intel Mac or use Rosetta
- **Apple Silicon (arm64)**: Build on M1/M2/M3 Mac
- Output: `vtr-feature-extractor` (Unix executable)

### Windows
- **x64**: Build on 64-bit Windows
- **x86**: Build on 32-bit Windows (if needed)
- Output: `vtr-feature-extractor.exe`

### Linux
- **x86_64**: Build on 64-bit Linux
- **ARM64**: Build on ARM64 Linux (if needed)
- Output: `vtr-feature-extractor` (Unix executable)

## Build Strategy Options

### Option 1: GitHub Actions (Recommended)
Create `.github/workflows/build-extractors.yml`:
```yaml
name: Build Feature Extractors

on:
  push:
    branches: [ main ]
  workflow_dispatch:

jobs:
  build-macos:
    strategy:
      matrix:
        include:
          - os: macos-12
            arch: x86_64
          - os: macos-14
            arch: arm64
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-python@v4
        with:
          python-version: '3.9'
      - run: |
          cd standalone_extractor
          ./build_minimal.sh
      - uses: actions/upload-artifact@v3
        with:
          name: vtr-feature-extractor-macos-${{ matrix.arch }}
          path: standalone_extractor/dist/vtr-feature-extractor

  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-python@v4
        with:
          python-version: '3.9'
      - run: |
          cd standalone_extractor
          python build_windows.py
      - uses: actions/upload-artifact@v3
        with:
          name: vtr-feature-extractor-windows-x64
          path: standalone_extractor/dist/vtr-feature-extractor.exe
```

### Option 2: Universal Binary for macOS
Build both architectures and combine:
```bash
# Build for Intel
arch -x86_64 python3 -m pyinstaller feature_extractor.spec
mv dist/vtr-feature-extractor dist/vtr-feature-extractor-x86_64

# Build for ARM64
arch -arm64 python3 -m pyinstaller feature_extractor.spec
mv dist/vtr-feature-extractor dist/vtr-feature-extractor-arm64

# Create universal binary
lipo -create -output dist/vtr-feature-extractor \
    dist/vtr-feature-extractor-x86_64 \
    dist/vtr-feature-extractor-arm64
```

### Option 3: Docker for Linux Builds
```dockerfile
FROM python:3.9-slim
WORKDIR /build
COPY . .
RUN pip install -r requirements_minimal.txt
RUN pyinstaller feature_extractor.spec
```

## C++ Plugin Integration

Update `ExternalFeatureExtractor::getExecutablePath()` to handle platform differences:

```cpp
std::string getExecutablePath() {
    std::string execName = "vtr-feature-extractor";
#ifdef _WIN32
    execName += ".exe";
#endif

    // Check architecture on macOS
#ifdef __APPLE__
    #ifdef __aarch64__
        execName += "-arm64";
    #else
        execName += "-x86_64";
    #endif
#endif

    // ... rest of path checking logic
}
```

## Distribution Package Structure
```
VTR-smartEQ/
├── Resources/
│   ├── extractors/
│   │   ├── macos/
│   │   │   ├── vtr-feature-extractor-x86_64
│   │   │   └── vtr-feature-extractor-arm64
│   │   ├── windows/
│   │   │   └── vtr-feature-extractor.exe
│   │   └── linux/
│   │       └── vtr-feature-extractor
```

## Alternative: Pure C++ Solution

If cross-platform building is too complex, consider:
1. Porting the feature extraction to pure C++ using a library like Essentia
2. Using WebAssembly to run Python in the browser/plugin
3. Creating a web service for feature extraction