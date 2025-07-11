cmake_minimum_required(VERSION 3.22)

project(VTR-smartEQ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add AddressSanitizer for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()

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

# Add Essentia (optional)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(ESSENTIA essentia)
    if(ESSENTIA_FOUND)
        message(STATUS "Found Essentia: ${ESSENTIA_VERSION}")
        set(HAVE_ESSENTIA TRUE)
    else()
        message(STATUS "Essentia not found. Building without Essentia support.")
        set(HAVE_ESSENTIA FALSE)
    endif()
else()
    message(STATUS "PkgConfig not found. Building without Essentia support.")
    set(HAVE_ESSENTIA FALSE)
endif()

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
        Source/VTR/VTRNetwork.cpp
        Source/VTR/VTRNetwork.h
)

# Add Essentia sources if available
if(HAVE_ESSENTIA)
    target_sources(VTR-smartEQ PRIVATE
        Source/VTR/EssentiaFeatureExtractor.cpp
        Source/VTR/EssentiaFeatureExtractor.h
    )
endif()

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

# Add Essentia compile definitions if available
if(HAVE_ESSENTIA)
    target_compile_definitions(VTR-smartEQ PUBLIC HAVE_ESSENTIA=1)
endif()

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

# Add Essentia libraries if available
if(HAVE_ESSENTIA)
    target_link_libraries(VTR-smartEQ PRIVATE ${ESSENTIA_LIBRARIES})
    target_include_directories(VTR-smartEQ PRIVATE ${ESSENTIA_INCLUDE_DIRS})
endif()

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
target_include_directories(test_vtr3_feature_extraction PRIVATE Source ${ESSENTIA_INCLUDE_DIRS})

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

# VTR Network test
add_executable(test_vtr_network 
    tests/test_vtr_network.cpp 
    Source/VTR/VTRNetwork.cpp
)
target_compile_features(test_vtr_network PRIVATE cxx_std_17)
target_link_libraries(test_vtr_network PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)
target_include_directories(test_vtr_network PRIVATE Source)

# VTR17 Feature validation test
add_executable(test_vtr17_feature_validation 
    tests/test_vtr17_feature_validation.cpp 
    Source/SpectrumAnalyzer.cpp
    Source/VTR/VTRNetwork.cpp
)
target_compile_features(test_vtr17_feature_validation PRIVATE cxx_std_17)
target_link_libraries(test_vtr17_feature_validation PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)
target_include_directories(test_vtr17_feature_validation PRIVATE Source)

# VTR18 Network validation test
add_executable(test_vtr18_network_validation 
    tests/test_vtr18_network_validation.cpp 
    Source/VTR/VTRNetwork.cpp
)
target_compile_features(test_vtr18_network_validation PRIVATE cxx_std_17)
target_link_libraries(test_vtr18_network_validation PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)
target_include_directories(test_vtr18_network_validation PRIVATE Source)

# VTR19 End-to-end integration test (simplified)
add_executable(test_vtr19_end_to_end 
    tests/test_vtr19_end_to_end_simple.cpp 
    Source/VTR/VTRNetwork.cpp
)
target_compile_features(test_vtr19_end_to_end PRIVATE cxx_std_17)
target_link_libraries(test_vtr19_end_to_end PRIVATE
    juce::juce_audio_basics
    juce::juce_core
    juce::juce_data_structures
)
target_include_directories(test_vtr19_end_to_end PRIVATE Source)

# Feature extraction comparison test
add_executable(test_features 
    test_features.cpp 
    Source/SpectrumAnalyzer.cpp
)
target_compile_features(test_features PRIVATE cxx_std_17)
target_link_libraries(test_features PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
    juce::juce_graphics
    juce::juce_events
)
target_include_directories(test_features PRIVATE Source)

target_compile_definitions(test_features
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

# Flexible feature extraction test
add_executable(test_features_flexible 
    test_features_flexible.cpp 
    Source/SpectrumAnalyzer.cpp
)
target_compile_features(test_features_flexible PRIVATE cxx_std_17)
target_link_libraries(test_features_flexible PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
    juce::juce_graphics
    juce::juce_events
)
target_include_directories(test_features_flexible PRIVATE Source)

target_compile_definitions(test_features_flexible
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

# Essentia feature extraction validation test
add_executable(test_essentia_validation 
    tests/test_essentia_validation.cpp 
    Source/VTR/EssentiaFeatureExtractor.cpp
)
target_compile_features(test_essentia_validation PRIVATE cxx_std_17)
target_link_libraries(test_essentia_validation PRIVATE
    ${ESSENTIA_LIBRARIES}
    gtest
    gtest_main
)
target_include_directories(test_essentia_validation PRIVATE Source ${ESSENTIA_INCLUDE_DIRS})

target_compile_definitions(test_essentia_validation
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