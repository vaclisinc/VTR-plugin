# VTR Plugin Research Summary

## Research Plan

The VTR plugin has been classified as **novel** based on its integration of AI/ML for real-time EQ parameter prediction from audio features. However, the actual model architecture is refreshingly simple: a basic feed-forward network (NN-ReLU-NN-ReLU-NN) that processes CSV input features and outputs CSV EQ gains for 5 frequency bands (80, 240, 2500, 4000, 10000 Hz). The current workflow involves uploading 8-second audio files, extracting features via librosa in Python, and processing CSV data. This simplicity significantly reduces implementation complexity compared to the advanced neural architectures researched.

## Recurring Patterns

### Technical Architecture Patterns
- **Python Training → Model Export → C++ Inference**: The dominant pattern involves training models in Python/PyTorch, exporting to intermediate formats (ONNX/JSON), and deploying via specialized C++ inference engines
- **RTNeural Dominance**: RTNeural has emerged as the de facto standard for real-time neural network inference in audio applications, offering real-time safety guarantees and optimized performance
- **JUCE Framework Integration**: All successful implementations leverage JUCE framework for cross-platform audio plugin development with consistent patterns for ML integration

### Performance Optimization Strategies
- **Small Model Architectures**: Successful real-time implementations use models with <1M parameters, focusing on efficiency over complexity
- **Real-Time Memory Management**: Avoiding memory allocation in audio threads through pre-allocated buffers and lockless data structures
- **Hybrid Processing**: Separating real-time audio processing from ML inference through background thread architectures

## Best Practices & Common Pitfalls

### Best Practices
1. **Use RTNeural for Real-Time Inference**: Provides optimal balance of performance and real-time safety
2. **Implement Feature Extraction Pipeline**: Focus on proven features like MFCCs and mel-scaled spectrograms
3. **Adopt Modern JUCE/CMake Workflow**: Enables proper dependency management and cross-platform builds
4. **Quantization and Model Optimization**: Apply INT8 quantization and model pruning for performance
5. **Thread-Safe Design**: Separate real-time audio processing from ML inference using lockless communication

### Common Pitfalls
1. **Memory Allocation in Audio Threads**: Causes audio dropouts and real-time performance issues
2. **Model Complexity**: Overly complex models fail real-time constraints (<10ms processing budget)
3. **License Conflicts**: GPL libraries restrict commercial use, requiring careful license selection
4. **Platform-Specific Dependencies**: Avoiding dependencies that don't support all target platforms
5. **Training Data Limitations**: Insufficient diversity in training data leads to poor generalization

## Top 5 Repos

1. **RTNeural**: The gold standard for real-time neural network inference in audio applications with proven performance and real-time safety guarantees
2. **Neural Amp Modeler (NAM)**: State-of-the-art neural amp modeling with excellent training infrastructure and comprehensive real-time inference capabilities
3. **GuitarML SmartGuitarPedal**: Production-ready example of JUCE + ML integration demonstrating complete plugin implementation with proven real-time performance
4. **BYOD**: Modular plugin architecture showcasing multiple ML-based audio effects with RTNeural integration and comprehensive test coverage
5. **ChowCentaur**: Reference implementation for RTNeural integration providing clear examples of analog modeling with neural networks

## Top 3 Papers

1. **"Real-Time Black-Box Modelling With Recurrent Neural Networks" (DAFx 2019)**: Demonstrates that RNNs can achieve WaveNet-level accuracy with significantly reduced computational requirements, includes practical JUCE implementation
2. **"Automatic Equalization for Individual Instrument Tracks Using Convolutional Neural Networks" (2024)**: Most directly relevant to VTR goals, showing 24% improvement in parametric equalizer matching with practical implementation strategies
3. **"Real-Time Guitar Amplifier Emulation with Deep Learning" (Applied Sciences 2020)**: Provides comprehensive analysis of computational requirements and optimization techniques for real-time neural audio processing

## Key Open Questions

### Simplified Implementation Strategy
Given the simple NN-ReLU-NN-ReLU-NN architecture and CSV-based workflow, the implementation questions become much more practical:

#### Technical Implementation
- **Feature Extraction Port**: How to efficiently port the librosa-based feature extraction from Python to C++ for real-time processing?
- **Model Deployment**: Should the simple feed-forward network be implemented directly in C++ or converted through RTNeural for consistency?
- **CSV Processing**: How to handle the current CSV input/output workflow in a real-time plugin environment?

#### Real-Time Integration
- **Audio Buffer Processing**: How to adapt the current 8-second audio file processing to work with real-time audio buffers?
- **Feature Extraction Timing**: Can librosa-equivalent feature extraction be performed within real-time audio constraints?
- **EQ Parameter Application**: How to smoothly apply the 5-band EQ parameters (80, 240, 2500, 4000, 10000 Hz) without audio artifacts?

#### Expandability Strategy
- **Model Versioning**: How to design the plugin architecture to easily swap in improved models as they're developed?
- **Feature Set Expansion**: How to add new audio features without breaking existing functionality?
- **Multi-Instrument Support**: Strategy for expanding beyond piano to other instruments while maintaining the simple architecture?

#### User Experience
- **Processing Latency**: Can the feature extraction + inference be completed fast enough for musical responsiveness?
- **Visual Feedback**: Should users see the extracted features or predicted EQ curve in real-time?
- **Audio Upload Interface**: How to integrate the current "upload audio file" workflow into a real-time plugin interface?