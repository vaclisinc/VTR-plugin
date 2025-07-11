#!/usr/bin/env python3
"""
Test script to compare JUCE C++ feature extraction with Python librosa implementation
"""

import numpy as np
import librosa
import sys
import os
import subprocess
import tempfile

def extract_features_python(audio_file):
    """Extract features using Python/librosa (same as extract_features.py)"""
    y, sr = librosa.load(audio_file, sr=44100)
    
    features = {}
    
    # Extract features in the same order as the Python script
    features["spectral_centroid"] = np.mean(librosa.feature.spectral_centroid(y=y, sr=sr))
    features["spectral_bandwidth"] = np.mean(librosa.feature.spectral_bandwidth(y=y, sr=sr))
    features["spectral_rolloff"] = np.mean(librosa.feature.spectral_rolloff(y=y, sr=sr))
    
    # Extract MFCCs
    mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=13)
    for i in range(mfccs.shape[0]):
        features[f"mfcc_{i+1}"] = np.mean(mfccs[i])
    
    features["rms_energy"] = np.mean(librosa.feature.rms(y=y))
    
    return features

def create_juce_feature_test():
    """Create a simple JUCE application to test feature extraction"""
    
    # Create a simple test program that uses the SpectrumAnalyzer
    test_cpp = """
#include <iostream>
#include <vector>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "../Source/SpectrumAnalyzer.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <audio_file>" << std::endl;
        return 1;
    }
    
    juce::initialiseJuce_GUI();
    
    // Load audio file
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    juce::File audioFile(argv[1]);
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));
    
    if (!reader)
    {
        std::cerr << "Failed to load audio file: " << argv[1] << std::endl;
        return 1;
    }
    
    // Read audio data
    juce::AudioBuffer<float> buffer(reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
    
    // Convert to mono
    std::vector<float> audioData;
    if (buffer.getNumChannels() > 1)
    {
        // Mix to mono
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float sample = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                sample += buffer.getSample(ch, i);
            }
            audioData.push_back(sample / buffer.getNumChannels());
        }
    }
    else
    {
        // Already mono
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            audioData.push_back(buffer.getSample(0, i));
        }
    }
    
    // Extract features
    SpectrumAnalyzer analyzer;
    std::vector<float> features = analyzer.extractFeatures(audioData, reader->sampleRate);
    
    // Output features in the same order as Python
    // Python order: spectral_centroid, spectral_bandwidth, spectral_rolloff, mfcc_1-13, rms_energy
    // C++ order: rms_energy, spectral_centroid, mfcc_1-13, spectral_bandwidth, spectral_rolloff
    
    // Map C++ indices to Python order
    std::cout << "spectral_centroid: " << features[1] << std::endl;
    std::cout << "spectral_bandwidth: " << features[15] << std::endl;
    std::cout << "spectral_rolloff: " << features[16] << std::endl;
    
    for (int i = 0; i < 13; ++i)
    {
        std::cout << "mfcc_" << (i+1) << ": " << features[2 + i] << std::endl;
    }
    
    std::cout << "rms_energy: " << features[0] << std::endl;
    
    juce::shutdownJuce_GUI();
    return 0;
}
"""
    
    # Write the test program
    with open('/tmp/juce_feature_test.cpp', 'w') as f:
        f.write(test_cpp)
    
    return '/tmp/juce_feature_test.cpp'

def compare_features(audio_file):
    """Compare features between Python and C++ implementations"""
    
    print(f"Testing feature extraction with: {audio_file}")
    print("=" * 60)
    
    # Extract features using Python
    print("Extracting features with Python/librosa...")
    python_features = extract_features_python(audio_file)
    
    print("\nPython features:")
    feature_names = [
        'spectral_centroid', 'spectral_bandwidth', 'spectral_rolloff',
        'mfcc_1', 'mfcc_2', 'mfcc_3', 'mfcc_4', 'mfcc_5', 'mfcc_6', 'mfcc_7',
        'mfcc_8', 'mfcc_9', 'mfcc_10', 'mfcc_11', 'mfcc_12', 'mfcc_13',
        'rms_energy'
    ]
    
    python_values = []
    for name in feature_names:
        value = python_features[name]
        python_values.append(value)
        print(f"  {name}: {value:.6f}")
    
    # Note: We can't easily compile and run the C++ test without setting up the full JUCE environment
    # Instead, let's provide instructions for manual testing
    
    print("\n" + "=" * 60)
    print("C++ Feature Extraction Test Instructions:")
    print("=" * 60)
    
    print("""
To test the C++ implementation:

1. Build the VTR plugin project
2. Add a test target to extract features from the audio file
3. Compare the output with the Python values above

Key differences to check:
- Feature ordering: C++ uses different order than Python
- Numerical precision: Should match within reasonable tolerance
- MFCC implementation: Check if mel filterbank and DCT match librosa

Expected C++ feature order:
[0] rms_energy
[1] spectral_centroid  
[2-14] mfcc_1 through mfcc_13
[15] spectral_bandwidth
[16] spectral_rolloff

The C++ implementation in SpectrumAnalyzer.cpp should produce similar values
to the Python implementation when processing the same audio file.
""")
    
    return python_values

def test_with_sample_audio():
    """Test with the specified audio file"""
    audio_file = "/Users/vaclis./Documents/project/vtr/audio_samples/reaper/processed/01_eq_loshelf_-4_bell1_-4_bell2_-4_bell3_-8_hishelf_4.wav"
    
    if not os.path.exists(audio_file):
        print(f"Audio file not found: {audio_file}")
        return
    
    # Extract features
    python_values = compare_features(audio_file)
    
    print(f"\nPython feature vector (17 values):")
    print("[", end="")
    for i, value in enumerate(python_values):
        print(f"{value:.6f}", end="")
        if i < len(python_values) - 1:
            print(", ", end="")
    print("]")
    
    # Also create a simple numerical comparison script
    print("\n" + "=" * 60)
    print("Quick C++ vs Python Feature Comparison")
    print("=" * 60)
    
    print("""
To manually test the C++ implementation:

1. Extract features from the audio file using the JUCE SpectrumAnalyzer
2. Compare with these Python values:
""")
    
    # Print in C++ order for easy comparison
    cpp_order = [
        ('rms_energy', python_values[16]),
        ('spectral_centroid', python_values[0]),
    ]
    
    # Add MFCCs
    for i in range(13):
        cpp_order.append((f'mfcc_{i+1}', python_values[3 + i]))
    
    cpp_order.extend([
        ('spectral_bandwidth', python_values[1]),
        ('spectral_rolloff', python_values[2])
    ])
    
    print("\nC++ expected order and values:")
    for i, (name, value) in enumerate(cpp_order):
        print(f"  features[{i:2d}] = {value:10.6f}  // {name}")

if __name__ == "__main__":
    test_with_sample_audio()