#!/usr/bin/env python3
"""
Analyze librosa's exact parameters and implementation details
"""

import numpy as np
import librosa
import librosa.feature
import librosa.util
import scipy.signal

def analyze_librosa_defaults():
    """Analyze librosa's default parameters"""
    print("=== LIBROSA DEFAULT PARAMETERS ===")
    
    # Load test audio
    audio_file = "/Users/vaclis./Documents/project/vtr/audio_samples/reaper/processed/01_eq_loshelf_-4_bell1_-4_bell2_-4_bell3_-8_hishelf_4.wav"
    y, sr = librosa.load(audio_file, sr=44100)
    
    print(f"Audio length: {len(y)} samples")
    print(f"Sample rate: {sr} Hz")
    print(f"Duration: {len(y)/sr:.2f} seconds")
    
    # 1. FFT Parameters
    print("\n=== FFT PARAMETERS ===")
    # Default n_fft for most librosa functions
    default_n_fft = 2048
    default_hop_length = 512  # n_fft // 4
    default_win_length = None  # same as n_fft
    default_window = 'hann'
    
    print(f"Default n_fft: {default_n_fft}")
    print(f"Default hop_length: {default_hop_length}")
    print(f"Default win_length: {default_win_length if default_win_length else default_n_fft}")
    print(f"Default window: {default_window}")
    
    # 2. Spectral Features
    print("\n=== SPECTRAL FEATURES ===")
    
    # Get actual STFT for analysis
    stft = librosa.stft(y, n_fft=default_n_fft, hop_length=default_hop_length, window=default_window)
    magnitude = np.abs(stft)
    power = magnitude ** 2
    
    print(f"STFT shape: {stft.shape}")
    print(f"Power spectrum shape: {power.shape}")
    print(f"Frequency bins: {power.shape[0]}")
    print(f"Time frames: {power.shape[1]}")
    
    # Frequency mapping
    freqs = librosa.fft_frequencies(sr=sr, n_fft=default_n_fft)
    print(f"Frequency range: {freqs[0]:.2f} Hz to {freqs[-1]:.2f} Hz")
    print(f"Frequency resolution: {freqs[1] - freqs[0]:.2f} Hz")
    
    # 3. Spectral Centroid
    print("\n=== SPECTRAL CENTROID ===")
    centroid = librosa.feature.spectral_centroid(y=y, sr=sr)
    print(f"Spectral centroid shape: {centroid.shape}")
    print(f"Spectral centroid mean: {np.mean(centroid):.6f} Hz")
    
    # Manual calculation to understand the algorithm
    freq_bins = librosa.fft_frequencies(sr=sr, n_fft=default_n_fft)
    # Get magnitude spectrum
    stft_mag = np.abs(librosa.stft(y, n_fft=default_n_fft, hop_length=default_hop_length))
    
    # Calculate centroid manually
    centroid_manual = []
    for frame in range(stft_mag.shape[1]):
        magnitude = stft_mag[:, frame]
        if np.sum(magnitude) > 0:
            centroid_frame = np.sum(freq_bins * magnitude) / np.sum(magnitude)
            centroid_manual.append(centroid_frame)
        else:
            centroid_manual.append(0)
    
    print(f"Manual centroid mean: {np.mean(centroid_manual):.6f} Hz")
    
    # 4. Spectral Bandwidth
    print("\n=== SPECTRAL BANDWIDTH ===")
    bandwidth = librosa.feature.spectral_bandwidth(y=y, sr=sr)
    print(f"Spectral bandwidth shape: {bandwidth.shape}")
    print(f"Spectral bandwidth mean: {np.mean(bandwidth):.6f} Hz")
    
    # 5. Spectral Rolloff
    print("\n=== SPECTRAL ROLLOFF ===")
    rolloff = librosa.feature.spectral_rolloff(y=y, sr=sr, roll_percent=0.85)
    print(f"Spectral rolloff shape: {rolloff.shape}")
    print(f"Spectral rolloff mean: {np.mean(rolloff):.6f} Hz")
    print(f"Rolloff percentage: 85%")
    
    # 6. RMS Energy
    print("\n=== RMS ENERGY ===")
    rms = librosa.feature.rms(y=y)
    print(f"RMS shape: {rms.shape}")
    print(f"RMS mean: {np.mean(rms):.6f}")
    
    # 7. MFCC Parameters
    print("\n=== MFCC PARAMETERS ===")
    
    # Default MFCC parameters
    default_n_mfcc = 13
    default_n_mels = 128
    default_fmin = 0.0
    default_fmax = None  # sr/2
    default_htk = False
    default_norm = 'ortho'
    default_dtype = np.float32
    
    print(f"Default n_mfcc: {default_n_mfcc}")
    print(f"Default n_mels: {default_n_mels}")
    print(f"Default fmin: {default_fmin} Hz")
    print(f"Default fmax: {default_fmax if default_fmax else sr/2} Hz")
    print(f"Default HTK: {default_htk}")
    print(f"Default DCT norm: {default_norm}")
    print(f"Default dtype: {default_dtype}")
    
    # Extract MFCCs
    mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=default_n_mfcc, n_mels=default_n_mels)
    print(f"MFCC shape: {mfccs.shape}")
    print(f"MFCC means: {[np.mean(mfccs[i]) for i in range(mfccs.shape[0])]}")
    
    # 8. Mel Filterbank Analysis
    print("\n=== MEL FILTERBANK ===")
    
    # Get mel filterbank
    mel_filters = librosa.filters.mel(sr=sr, n_fft=default_n_fft, n_mels=default_n_mels)
    print(f"Mel filterbank shape: {mel_filters.shape}")
    print(f"Mel filterbank covers {mel_filters.shape[1]} frequency bins")
    
    # Mel scale conversion
    mel_freqs = librosa.mel_frequencies(n_mels=default_n_mels + 2)  # +2 for edges
    hz_freqs = librosa.mel_to_hz(mel_freqs)
    print(f"Mel frequency range: {mel_freqs[0]:.2f} to {mel_freqs[-1]:.2f} mel")
    print(f"Hz frequency range: {hz_freqs[0]:.2f} to {hz_freqs[-1]:.2f} Hz")
    
    # 9. DCT Analysis
    print("\n=== DCT ANALYSIS ===")
    
    # Get mel spectrogram
    mel_spec = librosa.feature.melspectrogram(y=y, sr=sr, n_mels=default_n_mels)
    log_mel_spec = librosa.power_to_db(mel_spec)
    
    # Apply DCT manually
    from scipy.fftpack import dct
    mfcc_manual = dct(log_mel_spec, type=2, axis=0, norm='ortho')[:default_n_mfcc]
    
    print(f"Manual MFCC shape: {mfcc_manual.shape}")
    print(f"Manual MFCC means: {[np.mean(mfcc_manual[i]) for i in range(mfcc_manual.shape[0])]}")
    
    # Compare with librosa's MFCC
    print("\nComparison with librosa MFCC:")
    for i in range(default_n_mfcc):
        lib_mean = np.mean(mfccs[i])
        man_mean = np.mean(mfcc_manual[i])
        diff = abs(lib_mean - man_mean)
        print(f"  MFCC {i+1}: librosa={lib_mean:.6f}, manual={man_mean:.6f}, diff={diff:.6f}")
    
    # 10. Window Function Analysis
    print("\n=== WINDOW FUNCTION ===")
    
    # Get Hann window
    hann_window = scipy.signal.windows.hann(default_n_fft)
    print(f"Hann window length: {len(hann_window)}")
    print(f"Hann window sum: {np.sum(hann_window):.6f}")
    print(f"Hann window peak: {np.max(hann_window):.6f}")
    
    return {
        'n_fft': default_n_fft,
        'hop_length': default_hop_length,
        'window': default_window,
        'n_mfcc': default_n_mfcc,
        'n_mels': default_n_mels,
        'fmin': default_fmin,
        'fmax': sr/2,
        'dct_norm': default_norm,
        'sr': sr,
        'expected_results': {
            'spectral_centroid': np.mean(centroid),
            'spectral_bandwidth': np.mean(bandwidth),
            'spectral_rolloff': np.mean(rolloff),
            'rms_energy': np.mean(rms),
            'mfcc_means': [np.mean(mfccs[i]) for i in range(mfccs.shape[0])]
        }
    }

def create_cpp_reference_values():
    """Create reference values for C++ implementation"""
    print("\n" + "="*60)
    print("C++ IMPLEMENTATION REFERENCE VALUES")
    print("="*60)
    
    params = analyze_librosa_defaults()
    
    print("\n// C++ Constants to match librosa")
    print(f"static constexpr int FFT_SIZE = {params['n_fft']};")
    print(f"static constexpr int HOP_LENGTH = {params['hop_length']};")
    print(f"static constexpr int NUM_MFCC_COEFFS = {params['n_mfcc']};")
    print(f"static constexpr int NUM_MEL_FILTERS = {params['n_mels']};")
    print(f"static constexpr double FMIN = {params['fmin']:.1f};")
    print(f"static constexpr double FMAX = {params['fmax']:.1f};")
    
    print("\n// Expected results for test audio file:")
    results = params['expected_results']
    print(f"// Spectral centroid: {results['spectral_centroid']:.6f} Hz")
    print(f"// Spectral bandwidth: {results['spectral_bandwidth']:.6f} Hz")
    print(f"// Spectral rolloff: {results['spectral_rolloff']:.6f} Hz")
    print(f"// RMS energy: {results['rms_energy']:.6f}")
    print("// MFCC coefficients:")
    for i, mfcc_mean in enumerate(results['mfcc_means']):
        print(f"//   MFCC {i+1}: {mfcc_mean:.6f}")

if __name__ == "__main__":
    analyze_librosa_defaults()
    create_cpp_reference_values()