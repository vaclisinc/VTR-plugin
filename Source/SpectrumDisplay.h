#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SpectrumAnalyzer.h"

class SpectrumDisplay : public juce::Component, private juce::Timer
{
public:
    enum class DisplayMode
    {
        Input,
        Output,
        Both
    };
    
    SpectrumDisplay(SpectrumAnalyzer& analyzer);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setDisplayMode(DisplayMode mode);
    void setAlpha(float alpha);
    
private:
    void timerCallback() override;
    void updateSpectrumData();
    juce::Path createSpectrumPath(const std::vector<float>& spectrum, bool isInput);
    
    float frequencyToX(float frequency) const;
    float magnitudeToY(float magnitudeDB) const;
    
    SpectrumAnalyzer& spectrumAnalyzer;
    
    // Display settings
    DisplayMode displayMode = DisplayMode::Both;
    float alpha = 0.3f; // Secondary display priority
    
    // Spectrum data
    std::vector<float> inputSpectrum;
    std::vector<float> outputSpectrum;
    
    // Visual parameters
    static constexpr float MIN_FREQUENCY = 20.0f;
    static constexpr float MAX_FREQUENCY = 20000.0f;
    static constexpr float MIN_MAGNITUDE_DB = -40.0f;
    static constexpr float MAX_MAGNITUDE_DB = 40.0f;
    static constexpr float UPDATE_RATE_HZ = 30.0f;
    
    // Colors (more visible for testing)
    juce::Colour inputColor = juce::Colour(0xFF00FF00);   // Bright green
    juce::Colour outputColor = juce::Colour(0xFFFF6600);  // Bright orange
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumDisplay)
};