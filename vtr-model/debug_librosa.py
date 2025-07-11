#!/usr/bin/env python3
"""
Debug librosa's exact calculations to understand the differences
"""

import numpy as np
import librosa
import librosa.feature
import scipy.signal

def debug_librosa_computation():
    """Debug librosa's exact computation step by step"""
    audio_file = "/Users/vaclis./Documents/project/vtr/audio_samples/reaper/processed/01_eq_loshelf_-4_bell1_-4_bell2_-4_bell3_-8_hishelf_4.wav"
    
    # Load audio
    y, sr = librosa.load(audio_file, sr=44100)
    
    print(f"Audio length: {len(y)} samples")
    print(f"Audio RMS (full): {np.sqrt(np.mean(y**2)):.6f}")
    print(f"Audio range: {np.min(y):.6f} to {np.max(y):.6f}")
    
    # Parameters matching our C++ implementation
    n_fft = 2048
    hop_length = 512
    window = 'hann'
    
    # Get STFT
    print(f"\n=== STFT COMPUTATION ===")
    stft = librosa.stft(y, n_fft=n_fft, hop_length=hop_length, window=window)
    print(f"STFT shape: {stft.shape}")
    print(f"STFT is complex: {np.iscomplexobj(stft)}")
    
    # Get magnitude and power
    magnitude = np.abs(stft)
    power = magnitude ** 2
    
    print(f"Magnitude shape: {magnitude.shape}")
    print(f"Power shape: {power.shape}")
    
    # Check first frame
    print(f"\n=== FIRST FRAME ANALYSIS ===")
    first_frame_magnitude = magnitude[:, 0]
    first_frame_power = power[:, 0]
    
    print(f"First frame magnitude sum: {np.sum(first_frame_magnitude):.6f}")
    print(f"First frame power sum: {np.sum(first_frame_power):.6f}")
    print(f"First frame power (bins 0-9): {first_frame_power[:10]}")
    
    # Manual spectral centroid calculation
    freqs = librosa.fft_frequencies(sr=sr, n_fft=n_fft)
    print(f"Frequency bins: {len(freqs)}")
    print(f"Frequency range: {freqs[0]:.2f} to {freqs[-1]:.2f} Hz")
    print(f"Frequency resolution: {freqs[1] - freqs[0]:.2f} Hz")
    
    # Calculate centroid for first frame
    if np.sum(first_frame_power) > 0:
        centroid_frame0 = np.sum(freqs * first_frame_power) / np.sum(first_frame_power)
        print(f"Manual centroid (frame 0): {centroid_frame0:.2f} Hz")
    
    # Compare with librosa's centroid
    centroid_librosa = librosa.feature.spectral_centroid(y=y, sr=sr, n_fft=n_fft, hop_length=hop_length)
    print(f"Librosa centroid (frame 0): {centroid_librosa[0, 0]:.2f} Hz")
    print(f"Librosa centroid (average): {np.mean(centroid_librosa):.2f} Hz")
    
    # Test single frame extraction
    print(f"\n=== SINGLE FRAME EXTRACTION ===")
    
    # Extract first frame like our C++ code does
    frame = y[:n_fft].copy()
    print(f"Frame length: {len(frame)}")
    print(f"Frame RMS: {np.sqrt(np.mean(frame**2)):.6f}")
    print(f"Frame range: {np.min(frame):.6f} to {np.max(frame):.6f}")
    
    # Apply window
    window_func = scipy.signal.windows.hann(n_fft)
    frame_windowed = frame * window_func
    
    print(f"Window sum: {np.sum(window_func):.6f}")
    print(f"Windowed frame RMS: {np.sqrt(np.mean(frame_windowed**2)):.6f}")
    
    # Compute FFT
    fft_result = np.fft.fft(frame_windowed, n=n_fft)
    fft_magnitude = np.abs(fft_result[:n_fft//2 + 1])
    fft_power = fft_magnitude ** 2
    
    print(f"FFT magnitude shape: {fft_magnitude.shape}")
    print(f"FFT power sum: {np.sum(fft_power):.6f}")
    print(f"FFT power (bins 0-9): {fft_power[:10]}")
    
    # Compare with our C++ values
    print(f"\n=== COMPARISON WITH C++ ===")
    print("C++ power spectrum first 10 values:")
    cpp_power_values = [0.00277731, 0.00615045, 0.0994996, 5.3586, 41.5276, 74.031, 53.7673, 7.95694, 1.44876, 3.31652]
    for i, cpp_val in enumerate(cpp_power_values):
        py_val = fft_power[i]
        ratio = cpp_val / py_val if py_val > 0 else 0
        print(f"  Bin {i}: Python={py_val:.6f}, C++={cpp_val:.6f}, Ratio={ratio:.2f}")
    
    # Calculate centroid manually
    if np.sum(fft_power) > 0:
        manual_centroid = np.sum(freqs * fft_power) / np.sum(fft_power)
        print(f"Manual centroid (single frame): {manual_centroid:.2f} Hz")
    
    # Check if there's a scaling issue
    print(f"\n=== SCALING INVESTIGATION ===")
    print(f"C++ total energy: 212.429")
    print(f"Python total energy: {np.sum(fft_power):.6f}")
    print(f"Energy ratio (C++/Python): {212.429 / np.sum(fft_power):.2f}")
    
    # Check RMS calculation
    print(f"\n=== RMS INVESTIGATION ===")
    print(f"C++ RMS: 0.008260")
    print(f"Python RMS (full): {np.sqrt(np.mean(y**2)):.6f}")
    print(f"Python RMS (frame): {np.sqrt(np.mean(frame**2)):.6f}")
    
    # Librosa RMS
    rms_librosa = librosa.feature.rms(y=y)
    print(f"Librosa RMS (average): {np.mean(rms_librosa):.6f}")
    
    # Check if there's a different window or normalization
    print(f"\n=== WINDOW NORMALIZATION ===")
    print(f"Window energy: {np.sum(window_func**2):.6f}")
    print(f"Window norm (L1): {np.sum(window_func):.6f}")
    print(f"Window norm (L2): {np.sqrt(np.sum(window_func**2)):.6f}")

if __name__ == "__main__":
    debug_librosa_computation()