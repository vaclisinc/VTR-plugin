#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SpectrumAnalyzer.h"

class FrequencyResponseDisplay : public juce::Component, private juce::Timer
{
public:
    enum class DisplayMode
    {
        Input,
        Output,
        Both
    };
    
    FrequencyResponseDisplay(SpectrumAnalyzer& analyzer);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setDisplayMode(DisplayMode mode);
    void setSpectrumVisible(bool visible);
    
private:
    void timerCallback() override;
    void updateSpectrumData();
    
    void drawFrequencyGrid(juce::Graphics& g);
    void drawMagnitudeGrid(juce::Graphics& g);
    void drawFrequencyLabels(juce::Graphics& g);
    void drawMagnitudeLabels(juce::Graphics& g);
    
    juce::Path createSpectrumPath(const std::vector<float>& spectrum);
    
    float frequencyToX(float frequency) const;
    float magnitudeToY(float magnitudeDB) const;
    
    SpectrumAnalyzer& spectrumAnalyzer;
    
    // Display settings
    DisplayMode displayMode = DisplayMode::Both;
    bool spectrumVisible = true;
    
    // Spectrum data
    std::vector<float> inputSpectrum;
    std::vector<float> outputSpectrum;
    
    // Visual parameters
    static constexpr float MIN_FREQUENCY = 20.0f;
    static constexpr float MAX_FREQUENCY = 20000.0f;
    static constexpr float MIN_MAGNITUDE_DB = -40.0f;
    static constexpr float MAX_MAGNITUDE_DB = 40.0f;
    static constexpr float UPDATE_RATE_HZ = 30.0f;
    
    // Grid parameters
    static constexpr int FREQUENCY_GRID_LINES = 9; // 20Hz, 50Hz, 100Hz, 200Hz, 500Hz, 1kHz, 2kHz, 5kHz, 10kHz, 20kHz
    static constexpr int MAGNITUDE_GRID_LINES = 9; // -40, -30, -20, -10, 0, +10, +20, +30, +40 dB
    
    // Colors
    juce::Colour backgroundColour = juce::Colour(0xFF1A1A1A);
    juce::Colour gridColour = juce::Colour(0xFF404040);
    juce::Colour textColour = juce::Colour(0xFFB0B0B0);
    juce::Colour inputSpectrumColour = juce::Colour(0xFF00AA00);   // Green
    juce::Colour outputSpectrumColour = juce::Colour(0xFFFF6600);  // Orange
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponseDisplay)
};