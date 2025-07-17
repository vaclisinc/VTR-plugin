#!/usr/bin/env python3
"""
VTR Feature Extractor - Standalone executable for audio feature extraction
Communicates with C++ plugin via JSON over stdin/stdout
"""

import sys
import json
import base64
import numpy as np
import librosa
import argparse
import struct
import time

def extract_features_vector(audio_data, sr=44100):
    """Extract feature vector matching original VTR model"""
    try:
        y = np.array(audio_data, dtype=np.float32)
        
        # Safety check for audio data size
        if len(y) > 44100 * 60:  # More than 60 seconds at 44.1kHz
            return {"status": "error", "message": f"Audio data too large: {len(y)} samples"}
        
        if len(y) == 0:
            return {"status": "error", "message": "Empty audio data"}
        
        # Extract features using librosa (exactly matching extract_features.py)
        
        # Spectral features
        spectral_centroid = float(np.mean(librosa.feature.spectral_centroid(y=y, sr=sr)))
        spectral_bandwidth = float(np.mean(librosa.feature.spectral_bandwidth(y=y, sr=sr)))
        spectral_rolloff = float(np.mean(librosa.feature.spectral_rolloff(y=y, sr=sr)))
        
        # MFCC features
        mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=13)
        mfcc_means = [float(np.mean(mfccs[i])) for i in range(13)]
        
        # RMS energy
        rms_energy = float(np.mean(librosa.feature.rms(y=y)))
        
        # Return in the order expected by VTR model
        features = [spectral_centroid, spectral_bandwidth, spectral_rolloff] + mfcc_means + [rms_energy]
        
        return {
            "status": "success",
            "features": features,
            "feature_names": [
                "spectral_centroid", "spectral_bandwidth", "spectral_rolloff",
                "mfcc_1", "mfcc_2", "mfcc_3", "mfcc_4", "mfcc_5", "mfcc_6",
                "mfcc_7", "mfcc_8", "mfcc_9", "mfcc_10", "mfcc_11", "mfcc_12", "mfcc_13",
                "rms_energy"
            ]
        }
        
    except Exception as e:
        return {"status": "error", "message": str(e)}

def send_message(message):
    """Send JSON message with 4-byte length prefix"""
    msg_bytes = json.dumps(message).encode('utf-8')
    length = struct.pack('<I', len(msg_bytes))
    sys.stdout.buffer.write(length + msg_bytes)
    sys.stdout.buffer.flush()

def receive_message():
    """Receive JSON message with 4-byte length prefix"""
    # Read 4-byte length
    length_bytes = sys.stdin.buffer.read(4)
    if len(length_bytes) < 4:
        return None
    
    length = struct.unpack('<I', length_bytes)[0]
    
    # Read message
    msg_bytes = sys.stdin.buffer.read(length)
    if len(msg_bytes) < length:
        return None
    
    return json.loads(msg_bytes.decode('utf-8'))

def daemon_mode():
    """Run in daemon mode, processing requests until exit signal"""
    # Send ready signal
    send_message({"status": "ready", "version": "1.0"})
    
    while True:
        try:
            # Receive request
            request = receive_message()
            if request is None:
                break
            
            # Handle exit command
            if request.get("command") == "exit":
                send_message({"status": "exit"})
                break
            
            # Extract features
            if "audio_data" in request:
                # Decode base64 audio data
                audio_bytes = base64.b64decode(request["audio_data"])
                audio_data = np.frombuffer(audio_bytes, dtype=np.float32)
                
                sr = request.get("sr", 44100)
                result = extract_features_vector(audio_data, sr)
                send_message(result)
            else:
                send_message({"status": "error", "message": "No audio_data in request"})
                
        except Exception as e:
            send_message({"status": "error", "message": str(e)})

def test_mode():
    """Test mode with synthetic audio"""
    print("Running in test mode...")
    
    # Generate test audio (1 second sine wave)
    sr = 44100
    duration = 1.0
    frequency = 440.0  # A4 note
    
    t = np.linspace(0, duration, int(sr * duration))
    y = 0.5 * np.sin(2 * np.pi * frequency * t)
    
    # Extract features
    result = extract_features_vector(y, sr)
    
    if result["status"] == "success":
        print(f"\nExtracted {len(result['features'])} features:")
        for name, value in zip(result["feature_names"], result["features"]):
            print(f"  {name}: {value:.6f}")
    else:
        print(f"Error: {result['message']}")

def main():
    parser = argparse.ArgumentParser(description="VTR Feature Extractor")
    parser.add_argument("--daemon", action="store_true", help="Run in daemon mode")
    parser.add_argument("--test", action="store_true", help="Run test with synthetic audio")
    
    args = parser.parse_args()
    
    if args.test:
        test_mode()
    elif args.daemon or not sys.stdin.isatty():
        daemon_mode()
    else:
        print("VTR Feature Extractor")
        print("Usage:")
        print("  --daemon    Run in daemon mode (for plugin communication)")
        print("  --test      Test with synthetic audio")
        print("\nWhen run without arguments from a plugin, it automatically enters daemon mode.")

if __name__ == "__main__":
    main()