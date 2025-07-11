#!/usr/bin/env python3
"""
Extract features for any audio file to compare with C++ implementation
"""

import numpy as np
import librosa
import sys
import os

def extract_features_for_file(audio_file):
    """Extract features using Python/librosa for any audio file"""
    
    if not os.path.exists(audio_file):
        print(f"Audio file not found: {audio_file}")
        return None
    
    y, sr = librosa.load(audio_file, sr=44100)
    
    print(f"=== PYTHON FEATURE EXTRACTION ===")
    print(f"Audio file: {audio_file}")
    print(f"Audio length: {len(y)} samples")
    print(f"Sample rate: {sr} Hz")
    print(f"Duration: {len(y)/sr:.2f} seconds")
    
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
    
    print(f"\nPython features:")
    feature_names = [
        'spectral_centroid', 'spectral_bandwidth', 'spectral_rolloff',
        'mfcc_1', 'mfcc_2', 'mfcc_3', 'mfcc_4', 'mfcc_5', 'mfcc_6', 'mfcc_7',
        'mfcc_8', 'mfcc_9', 'mfcc_10', 'mfcc_11', 'mfcc_12', 'mfcc_13',
        'rms_energy'
    ]
    
    python_values = []
    for name in feature_names:
        value = features[name]
        python_values.append(value)
        print(f"  {name}: {value:.6f}")
    
    # Print in C++ order for easy comparison
    print(f"\n=== C++ EXPECTED ORDER AND VALUES ===")
    
    cpp_order = [
        ('rms_energy', features['rms_energy']),
        ('spectral_centroid', features['spectral_centroid']),
    ]
    
    # Add MFCCs
    for i in range(13):
        cpp_order.append((f'mfcc_{i+1}', features[f'mfcc_{i+1}']))
    
    cpp_order.extend([
        ('spectral_bandwidth', features['spectral_bandwidth']),
        ('spectral_rolloff', features['spectral_rolloff'])
    ])
    
    print("Expected values for C++ test:")
    for i, (name, value) in enumerate(cpp_order):
        print(f"    expected[{i:2d}] = {value:12.6f}f;  // {name}")
    
    return features

def create_updated_test_program(audio_file, features):
    """Create an updated test program with the correct expected values"""
    
    if features is None:
        return
    
    cpp_values = [
        features['rms_energy'],
        features['spectral_centroid'],
        features['mfcc_1'], features['mfcc_2'], features['mfcc_3'], 
        features['mfcc_4'], features['mfcc_5'], features['mfcc_6'], features['mfcc_7'],
        features['mfcc_8'], features['mfcc_9'], features['mfcc_10'], 
        features['mfcc_11'], features['mfcc_12'], features['mfcc_13'],
        features['spectral_bandwidth'],
        features['spectral_rolloff']
    ]
    
    print(f"\n=== UPDATED C++ TEST VALUES ===")
    print(f"// Expected values for: {os.path.basename(audio_file)}")
    print("float expectedValues[] = {")
    for i, value in enumerate(cpp_values):
        feature_names = [
            'rms_energy', 'spectral_centroid',
            'mfcc_1', 'mfcc_2', 'mfcc_3', 'mfcc_4', 'mfcc_5', 'mfcc_6', 'mfcc_7',
            'mfcc_8', 'mfcc_9', 'mfcc_10', 'mfcc_11', 'mfcc_12', 'mfcc_13',
            'spectral_bandwidth', 'spectral_rolloff'
        ]
        print(f"    {value:12.6f}f,  // {feature_names[i]}")
    print("};")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python test_any_audio.py <audio_file>")
        sys.exit(1)
    
    audio_file = sys.argv[1]
    features = extract_features_for_file(audio_file)
    
    if features:
        create_updated_test_program(audio_file, features)