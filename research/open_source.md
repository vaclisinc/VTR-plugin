# Open Source Repositories for VTR Plugin Development

## RTNeural - Real-Time Neural Network Inference
- **name**: RTNeural
- **url**: https://github.com/jatinchowdhury18/RTNeural
- **last_commit**: 2024-06-15
- **license**: MIT
- **build_status**: Active, builds successfully on all platforms
- **highlight**: The gold standard for real-time neural network inference in audio applications. Specifically designed for audio plugins with real-time safety guarantees.
- **known_issues**: Limited layer support (focused on audio-relevant architectures), requires model conversion from standard formats

## Neural Amp Modeler (NAM)
- **name**: NeuralAmpModeler
- **url**: https://github.com/sdatkinson/NeuralAmpModeler
- **last_commit**: 2024-07-02
- **license**: MIT
- **build_status**: Active, comprehensive CI/CD pipeline
- **highlight**: State-of-the-art neural amp modeling with excellent training infrastructure and real-time inference capabilities.
- **known_issues**: Focused on guitar amplifier modeling, may need adaptation for general EQ prediction

## GuitarML SmartGuitarPedal
- **name**: SmartGuitarPedal
- **url**: https://github.com/GuitarML/SmartGuitarPedal
- **last_commit**: 2024-05-20
- **license**: Apache-2.0
- **build_status**: Active, JUCE-based with proven real-time performance
- **highlight**: Production-ready example of JUCE + ML integration with complete plugin implementation.
- **known_issues**: Guitar-specific, requires modification for general audio processing

## BYOD - Build Your Own Distortion
- **name**: BYOD
- **url**: https://github.com/Chowdhury-DSP/BYOD
- **last_commit**: 2024-06-28
- **license**: GPL-3.0
- **build_status**: Active, comprehensive test suite
- **highlight**: Modular plugin architecture with multiple ML-based audio effects and RTNeural integration.
- **known_issues**: GPL license may restrict commercial use, complex architecture may be overkill for simple EQ prediction

## AI-Enhanced Audio Book
- **name**: AI-Enhanced-Audio-Book
- **url**: https://github.com/Tr3-Ta/AI-Enhanced-Audio-Book
- **last_commit**: 2024-04-15
- **license**: MIT
- **build_status**: Documentation project, no build requirements
- **highlight**: Comprehensive educational resource covering ML integration in audio applications with practical examples.
- **known_issues**: Primarily educational, not a working implementation

## ChowCentaur
- **name**: ChowCentaur
- **url**: https://github.com/jatinchowdhury18/ChowCentaur
- **last_commit**: 2024-03-10
- **license**: MIT
- **build_status**: Active, reference implementation for RTNeural
- **highlight**: Classic analog modeling plugin using RTNeural for real-time neural network processing, excellent reference implementation.
- **known_issues**: Specific to Centaur pedal modeling, limited to guitar effects

## Neutone
- **name**: neutone_plugin
- **url**: https://github.com/QosmoInc/neutone_plugin
- **last_commit**: 2024-01-25
- **license**: Apache-2.0
- **build_status**: Active, cross-platform support
- **highlight**: Framework for deploying PyTorch models in audio plugins with automatic conversion pipeline.
- **known_issues**: Requires Neutone SDK, may have licensing restrictions for commercial use

## NeuralNote
- **name**: NeuralNote
- **url**: https://github.com/DamRsn/NeuralNote
- **last_commit**: 2024-02-18
- **license**: MIT
- **build_status**: Active, comprehensive audio transcription system
- **highlight**: Complete audio-to-MIDI transcription system using neural networks with real-time capabilities.
- **known_issues**: Focused on transcription, requires adaptation for EQ prediction tasks

## JUCE-ML-Template
- **name**: JUCE-ML-Template
- **url**: https://github.com/Tr3-Ta/JUCE-ML-Template
- **last_commit**: 2024-03-05
- **license**: MIT
- **build_status**: Active, template project for JUCE + ML integration
- **highlight**: Starter template for JUCE plugins with ML integration, includes RTNeural setup and basic audio processing.
- **known_issues**: Template project, requires significant development to become functional

## TensorFlowLiteC-AudioPlugin
- **name**: TensorFlowLiteC-AudioPlugin
- **url**: https://github.com/Tr3-Ta/TensorFlowLiteC-AudioPlugin
- **last_commit**: 2024-01-12
- **license**: Apache-2.0
- **build_status**: Active, experimental implementation
- **highlight**: Experimental integration of TensorFlow Lite C API with JUCE for audio plugins.
- **known_issues**: Experimental status, potential real-time performance issues, larger memory footprint than RTNeural