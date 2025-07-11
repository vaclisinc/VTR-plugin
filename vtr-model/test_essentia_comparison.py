#!/usr/bin/env python3
"""
Test Essentia vs librosa feature extraction
"""

import numpy as np
import librosa
import sys
import os

# Try to import essentia
try:
    import essentia.standard as es
    ESSENTIA_AVAILABLE = True
    print("✅ Essentia is available")
except ImportError:
    ESSENTIA_AVAILABLE = False
    print("❌ Essentia not available. Install with: pip install essentia")

def extract_features_librosa(audio_file):
    """Extract features using librosa"""
    y, sr = librosa.load(audio_file, sr=44100)
    
    features = {}
    features["spectral_centroid"] = np.mean(librosa.feature.spectral_centroid(y=y, sr=sr))
    features["spectral_bandwidth"] = np.mean(librosa.feature.spectral_bandwidth(y=y, sr=sr))
    features["spectral_rolloff"] = np.mean(librosa.feature.spectral_rolloff(y=y, sr=sr))
    
    # Extract MFCCs
    mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=13)
    for i in range(mfccs.shape[0]):
        features[f"mfcc_{i+1}"] = np.mean(mfccs[i])
    
    features["rms_energy"] = np.mean(librosa.feature.rms(y=y))
    
    return features

def extract_features_essentia(audio_file):
    """Extract features using Essentia"""
    if not ESSENTIA_AVAILABLE:
        return None
    
    # Load audio
    loader = es.MonoLoader(filename=audio_file, sampleRate=44100)
    audio = loader()
    
    # Frame-based processing
    frameSize = 2048
    hopSize = 512
    
    # Initialize algorithms
    windowing = es.Windowing(type='hann')
    spectrum = es.Spectrum()
    mfcc = es.MFCC(numberCoefficients=13)
    spectralCentroid = es.SpectralCentroid()
    spectralBandwidth = es.SpectralBandwidth()
    spectralRolloff = es.SpectralRolloff()
    rms = es.RMS()
    
    # Process frames
    centroids = []
    bandwidths = []
    rolloffs = []
    rms_values = []
    all_mfccs = []
    
    for frame in es.FrameGenerator(audio, frameSize=frameSize, hopSize=hopSize):
        # Apply window and compute spectrum
        windowed_frame = windowing(frame)
        spec = spectrum(windowed_frame)
        
        # Extract features
        mfcc_bands, mfcc_coeffs = mfcc(spec)
        centroid = spectralCentroid(spec)
        bandwidth = spectralBandwidth(spec)
        rolloff = spectralRolloff(spec)
        rms_val = rms(frame)
        
        # Store values
        centroids.append(centroid)
        bandwidths.append(bandwidth)
        rolloffs.append(rolloff)
        rms_values.append(rms_val)
        all_mfccs.append(mfcc_coeffs)
    
    # Average across frames
    features = {}
    features["spectral_centroid"] = np.mean(centroids)
    features["spectral_bandwidth"] = np.mean(bandwidths)
    features["spectral_rolloff"] = np.mean(rolloffs)
    features["rms_energy"] = np.mean(rms_values)
    
    # Average MFCCs
    all_mfccs = np.array(all_mfccs)
    for i in range(13):
        features[f"mfcc_{i+1}"] = np.mean(all_mfccs[:, i])
    
    return features

def compare_libraries(audio_file):
    """Compare librosa vs Essentia"""
    print(f"=== COMPARING LIBROSA VS ESSENTIA ===")
    print(f"Audio file: {os.path.basename(audio_file)}")
    
    # Extract with librosa
    librosa_features = extract_features_librosa(audio_file)
    print("\n✅ Librosa features extracted")
    
    # Extract with Essentia
    essentia_features = extract_features_essentia(audio_file)
    
    if essentia_features is None:
        print("❌ Essentia features not available")
        return
    
    print("✅ Essentia features extracted")
    
    # Compare features
    print(f"\n{'Feature':<20} {'Librosa':<15} {'Essentia':<15} {'Diff %':<10}")
    print("-" * 65)
    
    feature_names = [
        'rms_energy', 'spectral_centroid', 'spectral_bandwidth', 'spectral_rolloff',
        'mfcc_1', 'mfcc_2', 'mfcc_3', 'mfcc_4', 'mfcc_5', 'mfcc_6', 'mfcc_7',
        'mfcc_8', 'mfcc_9', 'mfcc_10', 'mfcc_11', 'mfcc_12', 'mfcc_13'
    ]
    
    for name in feature_names:
        lib_val = librosa_features[name]
        ess_val = essentia_features[name]
        diff_pct = abs(lib_val - ess_val) / abs(lib_val) * 100 if lib_val != 0 else 0
        
        print(f"{name:<20} {lib_val:<15.6f} {ess_val:<15.6f} {diff_pct:<10.2f}")
    
    # Show which is closer to our C++ implementation
    print(f"\n=== RECOMMENDATION ===")
    print("If Essentia values are closer to our C++ implementation,")
    print("we should consider using Essentia instead of trying to match librosa exactly.")
    
    # Output C++ compatible values
    print(f"\n=== ESSENTIA C++ EXPECTED VALUES ===")
    essentia_order = [
        essentia_features['rms_energy'],
        essentia_features['spectral_centroid'],
        essentia_features['mfcc_1'], essentia_features['mfcc_2'], essentia_features['mfcc_3'],
        essentia_features['mfcc_4'], essentia_features['mfcc_5'], essentia_features['mfcc_6'],
        essentia_features['mfcc_7'], essentia_features['mfcc_8'], essentia_features['mfcc_9'],
        essentia_features['mfcc_10'], essentia_features['mfcc_11'], essentia_features['mfcc_12'],
        essentia_features['mfcc_13'],
        essentia_features['spectral_bandwidth'],
        essentia_features['spectral_rolloff']
    ]
    
    print("// Essentia expected values:")
    for i, val in enumerate(essentia_order):
        feature_names_ordered = [
            'rms_energy', 'spectral_centroid',
            'mfcc_1', 'mfcc_2', 'mfcc_3', 'mfcc_4', 'mfcc_5', 'mfcc_6', 'mfcc_7',
            'mfcc_8', 'mfcc_9', 'mfcc_10', 'mfcc_11', 'mfcc_12', 'mfcc_13',
            'spectral_bandwidth', 'spectral_rolloff'
        ]
        print(f"    {val:12.6f}f,  // {feature_names_ordered[i]}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python test_essentia_comparison.py <audio_file>")
        print("First install Essentia: pip install essentia")
        sys.exit(1)
    
    audio_file = sys.argv[1]
    
    if not os.path.exists(audio_file):
        print(f"Audio file not found: {audio_file}")
        sys.exit(1)
    
    compare_libraries(audio_file)