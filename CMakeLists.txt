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

# Find Python for embedded Python support
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Add libxtract
set(LIBXTRACT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/libxtract/src")
file(GLOB LIBXTRACT_SOURCES 
    "${LIBXTRACT_SRC_DIR}/*.c"
    "${LIBXTRACT_SRC_DIR}/ooura/*.c"
    "${LIBXTRACT_SRC_DIR}/dywapitchtrack/*.c"
)
# Exclude c-ringbuf due to compilation issues (not needed for basic features)
add_library(libxtract STATIC ${LIBXTRACT_SOURCES})
target_include_directories(libxtract PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/libxtract/include"
)
target_compile_options(libxtract PRIVATE -O3 -fPIC)
if(APPLE)
    target_compile_definitions(libxtract PRIVATE MACOS)
    target_link_libraries(libxtract PRIVATE "-framework Accelerate")
endif()

# Fix compilation issues with missing headers
target_compile_definitions(libxtract PRIVATE _GNU_SOURCE)
if(UNIX)
    target_compile_definitions(libxtract PRIVATE _POSIX_C_SOURCE=200809L)
endif()

# Plugin configuration
juce_add_plugin(VTR-smartEQ
    COMPANY_NAME "vaclis."
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
        Source/VTR/FeatureExtractor.cpp
        Source/VTR/FeatureExtractor.h
        Source/VTR/PythonFeatureExtractor.cpp
        Source/VTR/PythonFeatureExtractor.h
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
        libxtract
        Python3::Python
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)


