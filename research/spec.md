# VTR Plugin Technical Specification

## 1. Plugin Purpose & UX Vision

The VTR (Vaclis Tone Replication) plugin enables musicians to automatically match audio tones using AI-powered EQ parameter prediction. Users process audio through a simple feed-forward neural network that analyzes spectral features and outputs precise 5-band EQ settings for the target frequencies: 80Hz, 240Hz, 2.5kHz, 4kHz, and 10kHz. The plugin transforms the current Python-based CSV workflow into a real-time audio effect, making automatic tone matching accessible within any DAW environment while maintaining the simplicity and effectiveness of the underlying NN-ReLU-NN-ReLU-NN architecture.

## 2. Core Functional Requirements

- Extract audio features equivalent to librosa's spectral analysis (MFCCs, spectral centroid, RMS energy)
- Perform real-time neural network inference using simple feed-forward architecture (3-layer network with ReLU activation)
- Apply 5-band parametric EQ with precise frequency targeting: 80, 240, 2500, 4000, 10000 Hz
- Process audio in background thread to avoid real-time audio thread memory allocation
- Smooth EQ parameter transitions to prevent audible artifacts during parameter updates
- Support AU and VST3 plugin formats with universal binary deployment for macOS
- Load and swap neural network model weights from JSON format for easy model updates
- Maintain <10ms total processing latency including feature extraction and inference

## 3. Technical Architecture Overview

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│  Audio Input    │──→ │  Feature         │──→ │  Background     │
│  (Real-time)    │    │  Extraction      │    │  Processing     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                                        │
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│  EQ Filters     │←── │  Parameter       │←── │  NN Inference   │
│  (5-band)       │    │  Smoothing       │    │  (C++ Direct)   │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

**Dependencies Required:**
- JUCE framework (dual-licensed: GPL/Commercial)
- No external ML libraries (RTNeural not needed for simple architecture)
- JSON parsing for model weights (JUCE built-in)

**Recommended DSP Algorithms:**
- JUCE DSP IIR filters for 5-band parametric EQ implementation
- JUCE FFT for spectral analysis replacing librosa functionality
- Custom feature extraction using JUCE windowing functions
- Direct C++ matrix operations for 3-layer neural network inference

## 4. Repo & Paper Leverage Matrix

```yaml
- source: repo
  name: GuitarML SmartGuitarPedal
  url: https://github.com/GuitarML/SmartGuitarPedal
  role_in_project: Primary reference for JUCE+ML integration patterns and real-time processing architecture
  reason_for_rank: Production-ready implementation with proven real-time performance, Apache-2.0 license safe for commercial use

- source: repo
  name: Neural Amp Modeler
  url: https://github.com/sdatkinson/NeuralAmpModeler
  role_in_project: Training infrastructure and model export pipeline reference
  reason_for_rank: State-of-the-art neural audio modeling with comprehensive CI/CD, MIT license

- source: repo
  name: JUCE-ML-Template
  url: https://github.com/Tr3-Ta/JUCE-ML-Template
  role_in_project: Starting template for plugin project structure and CMake configuration
  reason_for_rank: Specifically designed for JUCE+ML integration, minimal setup required, MIT license

- source: repo
  name: ChowCentaur
  url: https://github.com/jatinchowdhury18/ChowCentaur
  role_in_project: Reference implementation for analog modeling plugin architecture
  reason_for_rank: Excellent example of neural network integration with JUCE, MIT license

- source: paper
  name: Automatic Equalization for Individual Instrument Tracks Using Convolutional Neural Networks
  url: https://arxiv.org/abs/2407.16691
  role_in_project: Core methodology for automatic EQ parameter prediction from audio features
  reason_for_rank: Direct relevance to VTR goals, 24% improvement in parametric equalizer matching

- source: paper
  name: Real-Time Guitar Amplifier Emulation with Deep Learning
  url: https://www.mdpi.com/2076-3417/10/3/766
  role_in_project: Performance optimization strategies for real-time neural audio processing
  reason_for_rank: Comprehensive analysis of computational requirements and real-time constraints

- source: paper
  name: Spectral and Rhythm Features for Audio Classification
  url: https://arxiv.org/abs/2410.06927
  role_in_project: Feature extraction methodology for audio classification using spectral analysis
  reason_for_rank: Validates MFCCs and mel-spectrograms as optimal features for audio ML tasks

- source: repo
  name: BYOD
  url: https://github.com/Chowdhury-DSP/BYOD
  role_in_project: Modular plugin architecture reference for expandable design
  reason_for_rank: Multiple ML-based audio effects with comprehensive test coverage
  license_risk: high
```

## 5. Build Feasibility & Risk Assessment

| Area | Complexity (1–5) | Risk (1–5) | Mitigation |
|------|------------------|------------|------------|
| Feature Extraction Port | 4 | 3 | Use JUCE DSP modules, reference librosa documentation |
| Simple NN Implementation | 2 | 1 | Direct C++ matrix operations, no external dependencies |
| Real-time Audio Processing | 3 | 2 | Background thread processing, atomic parameter updates |
| 5-band EQ Implementation | 2 | 1 | JUCE IIR filters, established parametric EQ algorithms |
| Cross-platform Deployment | 3 | 2 | Universal binary builds, JUCE framework handles platform differences |
| Model Loading System | 2 | 1 | JSON format, JUCE built-in parsing capabilities |

**Known Issues from Research:**
- Feature extraction timing constraints may require optimization for real-time performance
- Parameter smoothing required to prevent audio artifacts during EQ updates
- Model weight loading strategy needs validation across different plugin hosts

## 6. Validation Strategy

**Smoke Build Gate (First Red Test):**
1. Configure project for host CPU architecture
2. Compile using Ninja build system
3. Deploy to macOS plugin paths (AU/VST3)
4. Execute `pluginval --strictness high --validate-in-process`
5. Render 2-second pink noise with <0.1 dB RMS difference verification
6. Capture UI screenshot with minimum 200x100 pixel validation

**Continuous Validation Tests:**
- **Offline Audio Render:** Process known WAV files through 5-band EQ, verify no NaNs/INFs, validate expected frequency response
- **Real-time CPU Benchmark:** Measure processBlock execution time, fail if >90% buffer period
- **UI Screenshot Sanity:** Automated GUI capture and dimension validation
- **Custom ML Tests:** Validate feature extraction accuracy against Python librosa reference, verify neural network inference matches expected outputs

**LLM-driven Test Harness:**
Automated test generation for various audio input scenarios, EQ parameter validation, and regression testing for model updates using the established CI pipeline with GitHub Actions artifact collection.

## 7. Open Questions

- **Librosa Feature Extraction Porting:** Which specific librosa functions require C++ implementation, and should JUCE DSP modules be used exclusively or combined with custom spectral analysis code?
- **Real-time Processing Architecture:** Should feature extraction occur in real-time audio thread with pre-allocated buffers, or exclusively in background thread with audio buffering?
- **Model Weight Loading Strategy:** Should neural network weights be embedded in plugin binary or loaded from external JSON files, considering plugin host compatibility?
- **Parameter Smoothing Implementation:** What smoothing algorithm provides optimal balance between responsiveness and artifact prevention for EQ parameter updates?
- **GUI Feedback Design:** Should users receive visual feedback about extracted features, predicted EQ curves, or processing status during real-time operation?