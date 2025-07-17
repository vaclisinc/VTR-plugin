# Integration Example for External Feature Extractor

## 1. Update VTRNetwork to use FeatureExtractorFactory

```cpp
// In VTRNetwork.h, add:
#include "FeatureExtractorFactory.h"

class VTRNetwork
{
private:
    std::unique_ptr<VTR::FeatureExtractor> featureExtractor_;
    
public:
    VTRNetwork()
    {
        // Use factory to create extractor (AUTO selects best available)
        featureExtractor_ = VTR::FeatureExtractorFactory::create(
            VTR::FeatureExtractorFactory::ExtractorType::AUTO,
            true // fallback to JUCE if needed
        );
    }
};
```

## 2. Update CMakeLists.txt

Add the new source files:
```cmake
set(PLUGIN_SOURCES
    # ... existing sources ...
    Source/VTR/ExternalFeatureExtractor.cpp
    Source/VTR/FeatureExtractorFactory.cpp
)
```

## 3. Plugin Deployment Structure

### macOS (.vst3/.component)
```
YourPlugin.vst3/
└── Contents/
    ├── MacOS/
    │   └── YourPlugin
    └── Resources/
        └── vtr-feature-extractor  # Place executable here
```

### Windows (.vst3)
```
YourPlugin.vst3/
└── Contents/
    └── x86_64-win/
        ├── YourPlugin.vst3
        └── vtr-feature-extractor.exe  # Place next to DLL
```

### Linux (.vst3)
```
YourPlugin.vst3/
└── Contents/
    └── x86_64-linux/
        ├── YourPlugin.so
        └── vtr-feature-extractor  # Place next to SO
```

## 4. Build Script Integration

Add to your build process:
```bash
# After building plugin
if [ -f "standalone_extractor/dist/vtr-feature-extractor" ]; then
    # Copy to plugin resources
    cp standalone_extractor/dist/vtr-feature-extractor \
       Builds/MacOSX/build/Debug/YourPlugin.vst3/Contents/Resources/
fi
```

## 5. Testing the Integration

```cpp
// Simple test in your plugin
void testFeatureExtraction()
{
    auto extractor = VTR::FeatureExtractorFactory::create();
    
    // Generate test audio
    std::vector<float> testAudio(44100);
    for (int i = 0; i < 44100; ++i)
    {
        testAudio[i] = 0.5f * std::sin(2.0f * M_PI * 440.0f * i / 44100.0f);
    }
    
    // Extract features
    auto features = extractor->extractFeatures(
        testAudio.data(), 
        testAudio.size(), 
        44100
    );
    
    DBG("Extracted " << features.size() << " features");
}
```

## 6. Cross-Platform Building

For each platform, build the extractor:

```bash
# macOS ARM64
pyinstaller --target-arch arm64 --onefile feature_extractor.py

# macOS x86_64
pyinstaller --target-arch x86_64 --onefile feature_extractor.py

# Windows
pyinstaller --onefile feature_extractor.py

# Linux
pyinstaller --onefile feature_extractor.py
```

## 7. Environment Variable Override

Users can override the extractor path:
```bash
export VTR_EXTRACTOR_PATH=/custom/path/to/vtr-feature-extractor
```

This is useful for development or custom installations.