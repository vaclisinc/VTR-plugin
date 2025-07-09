#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SpectrumAnalyzer.h"
#include <chowdsp_visualizers/chowdsp_visualizers.h>

class VaclisDynamicEQAudioProcessor;

class FrequencyResponseDisplay : public juce::Component, private juce::Timer
{
public:
    enum class DisplayMode
    {
        Input,
        Output,
        Both
    };
    
    struct EQPoint {
        int bandIndex;
        float frequency;
        float gainDB;
        float Q;
        juce::Point<float> screenPosition;
        bool isActive;
        bool isHovered;
        
        EQPoint() : bandIndex(-1), frequency(1000.0f), gainDB(0.0f), Q(1.0f), 
                   screenPosition(0, 0), isActive(false), isHovered(false) {}
    };
    
    FrequencyResponseDisplay(SpectrumAnalyzer& analyzer);
    FrequencyResponseDisplay(SpectrumAnalyzer& analyzer, VaclisDynamicEQAudioProcessor& processor);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setDisplayMode(DisplayMode mode);
    void setSpectrumVisible(bool visible);
    
    // Mouse interaction methods
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    
    // EQ interaction methods
    int findNearestEQPoint(juce::Point<float> position);
    void updateEQPointPosition(int bandIndex, juce::Point<float> position);
    void updateParameterFromEQPoint(int bandIndex);
    void updateEQPointsFromParameters();
    float getDistanceToPoint(juce::Point<float> position, int bandIndex);
    
    // EQ curve calculation methods
    juce::Path createEQCurvePath();
    std::vector<float> calculateBandResponse(int bandIndex, int numPoints = 512);
    std::vector<float> calculateCombinedEQResponse(int numPoints = 512);
    
    // Filter response calculation methods
    float calculateBellResponse(float freq, float centerFreq, float gainDB, float Q) const;
    float calculateHighShelfResponse(float freq, float cutoffFreq, float gainDB, float Q) const;
    float calculateLowShelfResponse(float freq, float cutoffFreq, float gainDB, float Q) const;
    float calculateHighPassResponse(float freq, float cutoffFreq, float Q) const;
    float calculateLowPassResponse(float freq, float cutoffFreq, float Q) const;
    
    // Coordinate conversion helpers
    float xToFrequency(float x) const;
    float yToGainDB(float y) const;
    
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
    VaclisDynamicEQAudioProcessor* audioProcessor;
    
    // Display settings
    DisplayMode displayMode = DisplayMode::Both;
    bool spectrumVisible = true;
    
    // EQ interaction state
    std::array<EQPoint, 4> eqPoints;
    int draggingBandIndex = -1;
    bool isDragging = false;
    juce::Point<float> dragStartPosition;
    
    // EQ curve settings
    bool showEQCurve = true;
    bool showIndividualBands = false;
    
    // Curve calculation cache
    std::vector<float> cachedCombinedResponse;
    bool responseCacheValid = false;
    
    // Spectrum data
    std::vector<float> inputSpectrum;
    std::vector<float> outputSpectrum;
    
    // Visual parameters
    static constexpr float MIN_FREQUENCY = 20.0f;
    static constexpr float MAX_FREQUENCY = 20000.0f;
    static constexpr float MIN_MAGNITUDE_DB = -24.0f;
    static constexpr float MAX_MAGNITUDE_DB = 12.0f;
    static constexpr float UPDATE_RATE_HZ = 30.0f;
    
    // Grid parameters
    static constexpr int FREQUENCY_GRID_LINES = 10; // 20Hz, 50Hz, 100Hz, 200Hz, 500Hz, 1kHz, 2kHz, 5kHz, 10kHz, 20kHz
    static constexpr int MAGNITUDE_GRID_LINES = 7; // -24, -18, -12, -6, 0, +6, +12 dB
    
    // Colors
    juce::Colour backgroundColour = juce::Colour(0xFF1A1A1A);
    juce::Colour gridColour = juce::Colour(0xFF404040);
    juce::Colour textColour = juce::Colour(0xFFB0B0B0);
    juce::Colour inputSpectrumColour = juce::Colour(0xFF00AA00);   // Green
    juce::Colour outputSpectrumColour = juce::Colour(0xFFFF6600);  // Orange
    juce::Colour eqCurveColour = juce::Colour(0xFFFFFF00);         // Yellow
    
    // EQ band colors (matching existing band colors)
    juce::Colour getBandColour(int bandIndex) const;
    
    // Helper methods
    void invalidateResponseCache();
    void updateEQPointScreenPositions();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponseDisplay)
};