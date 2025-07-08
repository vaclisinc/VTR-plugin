#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>

class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Update level values (called from audio thread)
    void updateLevel(float newLevel);
    
    // Configuration
    void setOrientation(bool isHorizontal);
    void setRange(float minDB, float maxDB);
    
private:
    void timerCallback() override;
    float dbToPosition(float db) const;
    
    // Level data (atomic for thread safety)
    std::atomic<float> currentLevel{-60.0f};
    std::atomic<float> peakLevel{-60.0f};
    
    // Peak hold
    float peakHoldTime = 0.0f;
    static constexpr float PEAK_HOLD_SECONDS = 1.5f;
    static constexpr float UPDATE_RATE_HZ = 30.0f;
    
    // Configuration
    bool horizontal = false;
    float minDB = -60.0f;
    float maxDB = 0.0f;
    
    // Colors
    juce::Colour backgroundColour = juce::Colour(0xFF2A2A2A);
    juce::Colour levelColour = juce::Colour(0xFF00AA00);      // Green
    juce::Colour warningColour = juce::Colour(0xFFFFAA00);    // Orange
    juce::Colour criticalColour = juce::Colour(0xFFFF0000);   // Red
    juce::Colour peakColour = juce::Colours::white;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};