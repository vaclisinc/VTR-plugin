# VTR Plugin Integration Checklist

## Current State Assessment ✅

**Existing Plugin Architecture (Already Implemented):**
- ✅ Sophisticated 4-band Dynamic EQ with ChowDSP filters 
- ✅ FFT-based spectral analysis (SpectrumAnalyzer with 2048-point FFT)
- ✅ Parameter management with smoothing and thread-safety
- ✅ Professional JUCE plugin framework (AU/VST3)
- ✅ Real-time audio processing pipeline
- ✅ GUI with frequency response visualization
- ✅ Scalable architecture (MAX_BANDS = 8, CURRENT_BANDS = 4)

**VTR Model Analysis (Based on vtr_model.ipynb):**
- ✅ Feature extraction: 17-dimensional vector (spectral_centroid, spectral_bandwidth, spectral_rolloff, 13 MFCC coefficients, RMS energy)
- ✅ Neural network architecture: 3-layer MLP (17→64→64→5) with ReLU activation
- ✅ Data preprocessing: StandardScaler for feature normalization
- ✅ Model performance: MSE = 0.0216 (significantly better than linear regression: 0.5354)
- ✅ Target frequencies: 80Hz, 240Hz, 2.5kHz, 4kHz, 10kHz (5 bands)

## VTR Integration Work Required

### Phase 1: EQ Extension for VTR

- [x] **VTR1**: Expand EQ from 4 to 5 bands
  - objective: Add 5th band to existing MultiBandEQ for VTR integration
  - current_state: EQBand.h shows CURRENT_BANDS = 4, MAX_BANDS = 8 (ready for expansion)
  - implementation: Change CURRENT_BANDS to 5, add parameters for 5th band
  - done_when: Plugin builds with 5 bands, parameters accessible

- [x] **VTR2**: Configure VTR target frequencies
  - objective: Set band frequencies to VTR targets: 80Hz, 240Hz, 2.5kHz, 4kHz, 10kHz
  - current_state: Existing bands have configurable frequencies
  - implementation: Set default frequencies to VTR targets, use Bell filter type
  - done_when: EQ bands target exact VTR frequencies

### Phase 2: Complete Feature Extraction System (17-dimensional vector)

- [x] **VTR3**: Implement spectral feature extraction
  - objective: Extract spectral_centroid, spectral_bandwidth, spectral_rolloff from FFT
  - current_state: SpectrumAnalyzer has 2048-point FFT with Hann window
  - implementation: Add spectral feature methods to SpectrumAnalyzer class
  - **note**: Process reference audio files at 44.1kHz (resample if needed)
  - reference: librosa.feature.spectral_centroid, spectral_bandwidth, spectral_rolloff
  - done_when: 3 spectral features match librosa within 1% tolerance

- [x] **VTR4**: Implement mel-scale filterbank for MFCC preprocessing
  - objective: Create mel filterbank matching librosa implementation
  - implementation: mel_frequencies, mel_to_hz, hz_to_mel functions + filterbank matrix
  - reference: librosa.filters.mel() - typically 128 mel bins for 44.1kHz
  - done_when: Mel filterbank energies match librosa within 1% tolerance

- [x] **VTR5**: Implement DCT for MFCC calculation
  - objective: Extract 13 MFCC coefficients using DCT-II
  - implementation: Apply DCT to log mel-filterbank energies
  - reference: librosa.feature.mfcc(n_mfcc=13)
  - done_when: 13 MFCC coefficients match librosa within 1% tolerance

- [x] **VTR6**: Implement RMS energy calculation
  - objective: Calculate RMS energy from audio frames
  - implementation: sqrt(mean(signal^2)) over analysis windows
  - reference: librosa.feature.rms()
  - done_when: RMS energy matches librosa within 1% tolerance

- [x] **VTR7**: Integrate complete 17-dimensional feature vector
  - objective: Combine all features into single extraction pipeline
  - implementation: [spectral_centroid, spectral_bandwidth, spectral_rolloff, mfcc_1...mfcc_13, rms_energy]
  - done_when: Complete feature vector matches Python reference implementation

### Phase 3: Neural Network Implementation

- [ ] **VTR8**: Implement StandardScaler for feature normalization
  - objective: Normalize features using mean and standard deviation
  - implementation: scaled_feature = (feature - mean) / std
  - reference: sklearn.preprocessing.StandardScaler from training data
  - done_when: Feature normalization matches Python preprocessing

- [ ] **VTR9**: Implement 3-layer MLP neural network
  - objective: Create feed-forward network (17→64→64→5) with ReLU activation
  - implementation: 
    - Layer 1: Linear(17, 64) + ReLU
    - Layer 2: Linear(64, 64) + ReLU  
    - Layer 3: Linear(64, 5)
  - done_when: Network structure matches PyTorch model architecture

- [ ] **VTR10**: Add model weight loading system
  - objective: Load trained weights and biases from files
  - implementation: Binary or JSON format for weight matrices and biases
  - current_state: Need to export weights from PyTorch model
  - done_when: Plugin loads model weights and initializes network correctly

- [ ] **VTR11**: Implement matrix operations for inference
  - objective: Pure C++ implementation without external ML libraries
  - implementation: Matrix multiplication, ReLU activation, bias addition
  - done_when: Network inference produces correct outputs for test vectors

### Phase 4: VTR Processing Pipeline

- [ ] **VTR12**: Create background processing thread for reference audio
  - objective: Process uploaded reference audio files without blocking UI
  - current_state: Plugin already has thread-safe spectrum analysis
  - implementation: Background thread for file processing, atomic parameter updates
  - **workflow**: File upload → Background processing → Feature extraction → NN inference → Update EQ parameters
  - done_when: Reference audio processing runs without blocking UI thread

- [ ] **VTR13**: Connect feature extraction to neural network
  - objective: Complete pipeline: Audio → Features → Normalization → NN → EQ parameters
  - implementation: Feature vector extraction → StandardScaler → MLP inference
  - done_when: Audio analysis produces 5 EQ parameter predictions

- [ ] **VTR14**: Integrate NN output with existing EQ parameter system
  - objective: Apply NN predictions to existing ParameterManager
  - current_state: ParameterManager handles smoothing and thread-safe updates
  - implementation: Convert NN output to parameter values, apply through existing system
  - done_when: NN predictions automatically adjust EQ band parameters

- [ ] **VTR15**: Add VTR GUI components
  - objective: Create user interface for VTR functionality
  - implementation:
    - "Load Reference Audio" button
    - File browser dialog for audio files
    - Progress indicator for processing
    - "Apply VTR Settings" button
    - Status display (file loaded, processing, completed)
  - done_when: Users can interact with VTR functionality through GUI

- [ ] **VTR16**: Add reference audio file loading and processing
  - objective: Allow users to upload reference audio files for tone matching
  - implementation: 
    - File browser/drag-drop interface for audio files
    - Audio file loading (WAV, MP3, etc.) using JUCE AudioFormatManager
    - Resample to 44.1kHz if needed for feature extraction
    - Process entire file through VTR pipeline
  - done_when: Plugin can load reference files, analyze them, and apply learned EQ settings

### Phase 5: Testing & Validation

- [ ] **VTR17**: Validate feature extraction accuracy
  - objective: Ensure C++ features match Python librosa reference
  - tests: Process identical audio through both implementations
  - acceptance: All 17 features match librosa within 1% tolerance

- [ ] **VTR18**: Validate neural network inference
  - objective: Ensure C++ inference matches PyTorch reference
  - tests: Process identical feature vectors through both implementations  
  - acceptance: C++ inference matches PyTorch within 0.1% tolerance

- [ ] **VTR19**: End-to-end integration testing
  - objective: Verify complete VTR workflow functions correctly
  - tests: Upload reference audio → feature extraction → normalization → NN → EQ application
  - acceptance: Plugin successfully matches reference tones through GUI workflow

## Model Export Requirements

**From Python to C++:**
- Export StandardScaler parameters (mean, std for 17 features)
- Export neural network weights and biases:
  - Layer 1: weight_matrix[64x17], bias[64]
  - Layer 2: weight_matrix[64x64], bias[64]
  - Layer 3: weight_matrix[5x64], bias[5]
- Export model metadata (input/output dimensions, activation functions)

## Technical Implementation Notes

**Feature Extraction Specifications:**
- Sample rate: 44.1kHz (matching training data)
- Window size: 2048 samples (existing FFT size)
- Hop length: 512 samples (typical for librosa)
- Mel filterbank: 128 bins (standard for MFCC)
- MFCC coefficients: 13 (matching training data)
- **Static analysis**: Resample uploaded reference audio to 44.1kHz if needed

**Neural Network Specifications:**
- Input: 17-dimensional feature vector
- Hidden layers: 64 neurons each with ReLU activation
- Output: 5 EQ parameters (gain values for each band)
- Precision: float32 for real-time performance

**Integration Strategy:**
- Extend existing SpectrumAnalyzer class for feature extraction
- Maintain existing parameter smoothing and thread-safety
- Add VTR functionality as optional processing layer
- **Primary use case**: Static analysis of uploaded reference audio files
- **Future enhancement**: Dynamic analysis of real-time audio (Phase 2)
- Preserve existing plugin functionality while adding VTR capabilities