# Feature Extraction Comparison Results

## Test Audio File
`/Users/vaclis./Documents/project/vtr/audio_samples/reaper/processed/01_eq_loshelf_-4_bell1_-4_bell2_-4_bell3_-8_hishelf_4.wav`

## Results Summary

The JUCE C++ implementation produces significantly different results compared to the Python/librosa implementation:

### Key Differences:

| Feature | C++ Value | Python Value | Difference | Relative Error |
|---------|-----------|--------------|------------|----------------|
| RMS Energy | 0.008260 | 0.006516 | +0.001744 | 26.76% |
| Spectral Centroid | 155.72 Hz | 1383.14 Hz | -1227.42 Hz | **88.74%** |
| MFCC 1 | -54.24 | -660.78 | +606.54 | **91.79%** |
| MFCC 2 | 31.56 | 164.44 | -132.88 | **80.81%** |
| Spectral Bandwidth | 143.97 Hz | 2115.86 Hz | -1971.89 Hz | **93.20%** |
| Spectral Rolloff | 150.73 Hz | 2280.68 Hz | -2129.95 Hz | **93.39%** |

## Root Cause Analysis

### 1. **Spectral Features (Centroid, Bandwidth, Rolloff)**
The C++ implementation produces values that are **~10x smaller** than expected. This suggests:
- **Frequency calculation error**: The frequency mapping from FFT bins to Hz might be incorrect
- **Power spectrum computation**: The power spectrum calculation might be using different scaling

### 2. **MFCC Coefficients**
Large differences in MFCC values indicate:
- **Mel filterbank differences**: The mel filter implementation might not match librosa
- **DCT differences**: The DCT implementation might use different normalization
- **Log scaling**: Different approaches to log scaling of mel energies

### 3. **RMS Energy**
Smaller difference (26.76%) suggests this is the most accurate feature.

## Potential Issues in C++ Implementation

### 1. **Frequency Calculation** (SpectrumAnalyzer.cpp:258)
```cpp
double frequency = (i * sampleRate) / (2.0 * powerSpectrum.size());
```
**Issue**: This assumes `powerSpectrum.size()` is the full FFT size, but it might be FFT_SIZE/2.

**Fix**: Should be:
```cpp
double frequency = (i * sampleRate) / (2.0 * FFT_SIZE);
```

### 2. **Power Spectrum Computation** (SpectrumAnalyzer.cpp:461)
```cpp
powerSpectrum[i] = fftData[i] * fftData[i];
```
**Issue**: JUCE's `performFrequencyOnlyForwardTransform` returns magnitude, not complex data.

**Fix**: The power spectrum calculation might need adjustment based on JUCE's FFT output format.

### 3. **Mel Filter Implementation**
The mel filterbank implementation might not match librosa's default parameters:
- Number of mel filters (26 vs different)
- Mel scale formula variations
- Filter normalization

### 4. **DCT Normalization**
```cpp
const float sqrt2OverN = std::sqrt(2.0f / N);
```
**Issue**: librosa might use different DCT normalization.

## Recommended Fixes

### 1. **Immediate Fix for Testing**
Create a simpler test that uses known reference values or synthetic signals to validate each component separately.

### 2. **Match librosa Parameters**
- Verify FFT size (default 2048 vs current FFT_SIZE)
- Check mel filter count (default 128 vs current 26)
- Ensure hop length matches
- Verify window function (Hann vs others)

### 3. **Debug Step by Step**
1. Compare power spectrum between C++ and Python
2. Compare mel filterbank energies
3. Compare DCT coefficients
4. Test with simple sine wave inputs

### 4. **Use librosa's Exact Parameters**
Extract and use the exact parameters librosa uses:
```python
# Get librosa's default parameters
n_fft = 2048
hop_length = 512
n_mels = 128
n_mfcc = 13
```

## Current Status
❌ **C++ feature extraction does NOT match Python implementation**

The differences are too large (>50% error) for the VTR model to work correctly. The neural network was trained on Python-extracted features, so it expects the same feature scaling and distribution.

## Next Steps
1. **Fix the frequency calculation** in spectral features
2. **Verify the power spectrum computation** 
3. **Match librosa's exact MFCC parameters**
4. **Test with a simple sine wave** to validate each component
5. **Re-test with the same audio file** to verify fixes