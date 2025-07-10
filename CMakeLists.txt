cmake_minimum_required(VERSION 3.22)

project(VTR-smartEQ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure for host CPU architecture only (not universal binary)
set(CMAKE_OSX_ARCHITECTURES ${CMAKE_HOST_SYSTEM_PROCESSOR})

# Find JUCE
include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        8.0.8
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Add chowdsp_utils
add_subdirectory(third-party/chowdsp_utils)

# Plugin configuration
juce_add_plugin(VTR-smartEQ
    COMPANY_NAME "Vaclis Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Vacl
    PLUGIN_CODE VsEQ
    FORMATS AU VST3
    PRODUCT_NAME "VTR-smartEQ"
    BUNDLE_ID "com.vaclis.VTR-smartEQ"
    DESCRIPTION "AI-Powered Smart EQ Plugin"
)

# Source files
target_sources(VTR-smartEQ
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters/ParameterManager.cpp
        Source/Parameters/ParameterManager.h
        Source/DSP/EQBand.cpp
        Source/DSP/EQBand.h
        Source/DSP/GainProcessor.cpp
        Source/DSP/GainProcessor.h
        Source/SpectrumAnalyzer.cpp
        Source/SpectrumAnalyzer.h
        Source/SpectrumDisplay.cpp
        Source/SpectrumDisplay.h
        Source/FrequencyResponseDisplay.cpp
        Source/FrequencyResponseDisplay.h
        Source/LevelMeter.cpp
        Source/LevelMeter.h
)

# Compile definitions
target_compile_definitions(VTR-smartEQ
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_ALSA=0
        JUCE_JACK=0
        JUCE_COREAUDIO=1
        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1
        JUCE_ASIO=1
)

# Link libraries
target_link_libraries(VTR-smartEQ
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        chowdsp_filters
        chowdsp_eq
        chowdsp_compressor
        chowdsp_visualizers
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Test executables
add_executable(test_vtr3_feature_extraction 
    tests/test_vtr3_feature_extraction.cpp 
    Source/SpectrumAnalyzer.cpp
)
target_compile_features(test_vtr3_feature_extraction PRIVATE cxx_std_17)
target_link_libraries(test_vtr3_feature_extraction PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)
target_include_directories(test_vtr3_feature_extraction PRIVATE Source)

# Pink noise processing test with full JUCE integration
add_executable(test_pink_noise_processing
    tests/test_pink_noise_processing.cpp
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/Parameters/ParameterManager.cpp
    Source/Parameters/ParameterManager.h
    Source/DSP/EQBand.cpp
    Source/DSP/EQBand.h
    Source/DSP/GainProcessor.cpp
    Source/DSP/GainProcessor.h
    Source/SpectrumAnalyzer.cpp
    Source/SpectrumAnalyzer.h
    Source/SpectrumDisplay.cpp
    Source/SpectrumDisplay.h
    Source/FrequencyResponseDisplay.cpp
    Source/FrequencyResponseDisplay.h
    Source/LevelMeter.cpp
    Source/LevelMeter.h
)

target_compile_features(test_pink_noise_processing PRIVATE cxx_std_17)

target_compile_definitions(test_pink_noise_processing
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_ALSA=0
        JUCE_JACK=0
        JUCE_COREAUDIO=1
        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1
        JUCE_ASIO=1
)

target_link_libraries(test_pink_noise_processing PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    chowdsp_filters
    chowdsp_eq
    chowdsp_compressor
    chowdsp_visualizers
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_include_directories(test_pink_noise_processing PRIVATE Source)

# Simple pink noise test without plugin dependencies
add_executable(test_simple_pink_noise tests/test_simple_pink_noise.cpp)
target_compile_features(test_simple_pink_noise PRIVATE cxx_std_17)
target_link_libraries(test_simple_pink_noise PRIVATE
    juce::juce_audio_basics
    juce::juce_core
    juce::juce_dsp
)

# JUCE DSP pink noise test with proper DSP processing
add_executable(test_juce_dsp_pink_noise tests/test_juce_dsp_pink_noise.cpp)
target_compile_features(test_juce_dsp_pink_noise PRIVATE cxx_std_17)
target_link_libraries(test_juce_dsp_pink_noise PRIVATE
    juce::juce_audio_basics
    juce::juce_core
    juce::juce_dsp
)

# VTR3 Feature storage test
add_executable(test_feature_storage 
    tests/test_feature_storage.cpp 
    Source/SpectrumAnalyzer.cpp
)
target_compile_features(test_feature_storage PRIVATE cxx_std_17)
target_link_libraries(test_feature_storage PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)
target_include_directories(test_feature_storage PRIVATE Source)