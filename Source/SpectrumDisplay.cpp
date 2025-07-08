#include "SpectrumDisplay.h"

SpectrumDisplay::SpectrumDisplay(SpectrumAnalyzer& analyzer)
    : spectrumAnalyzer(analyzer)
{
    // Start timer for 30Hz updates
    startTimer(static_cast<int>(1000.0f / UPDATE_RATE_HZ));
    
    // Make this component paint over other components
    setAlwaysOnTop(false);
    setInterceptsMouseClicks(false, false);
}

void SpectrumDisplay::paint(juce::Graphics& g)
{
    // Draw test lines if no audio data available
    if (inputSpectrum.empty() && outputSpectrum.empty())
    {
        g.setColour(juce::Colours::yellow);
        g.drawText("Spectrum Display Active (No Audio)", getLocalBounds(), juce::Justification::centred);
        return;
    }
    
    // Set alpha for secondary display priority
    g.setOpacity(alpha);
    
    // Draw input spectrum
    if ((displayMode == DisplayMode::Input || displayMode == DisplayMode::Both) && !inputSpectrum.empty())
    {
        juce::Path inputPath = createSpectrumPath(inputSpectrum, true);
        g.setColour(inputColor);
        g.strokePath(inputPath, juce::PathStrokeType(3.0f)); // Thicker line
    }
    
    // Draw output spectrum
    if ((displayMode == DisplayMode::Output || displayMode == DisplayMode::Both) && !outputSpectrum.empty())
    {
        juce::Path outputPath = createSpectrumPath(outputSpectrum, false);
        g.setColour(outputColor);
        g.strokePath(outputPath, juce::PathStrokeType(3.0f)); // Thicker line
    }
}

void SpectrumDisplay::resized()
{
    // Component resized, paths will be recreated on next paint
}

void SpectrumDisplay::setDisplayMode(DisplayMode mode)
{
    displayMode = mode;
    repaint();
}

void SpectrumDisplay::setAlpha(float newAlpha)
{
    alpha = juce::jlimit(0.0f, 1.0f, newAlpha);
    repaint();
}

void SpectrumDisplay::timerCallback()
{
    updateSpectrumData();
}

void SpectrumDisplay::updateSpectrumData()
{
    // Get fresh spectrum data
    inputSpectrum = spectrumAnalyzer.getInputSpectrum();
    outputSpectrum = spectrumAnalyzer.getOutputSpectrum();
    
    // Trigger repaint
    repaint();
}

juce::Path SpectrumDisplay::createSpectrumPath(const std::vector<float>& spectrum, bool isInput)
{
    juce::ignoreUnused(isInput);
    juce::Path path;
    
    if (spectrum.empty())
        return path;
    
    const auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    
    // Start path
    bool pathStarted = false;
    
    // Convert spectrum data to path
    for (size_t i = 1; i < spectrum.size(); ++i) // Skip DC component
    {
        // Calculate frequency for this bin
        const float frequency = (static_cast<float>(i) * static_cast<float>(spectrumAnalyzer.getSampleRate())) / (2.0f * SpectrumAnalyzer::FFT_SIZE);
        
        // Skip frequencies outside our range
        if (frequency < MIN_FREQUENCY || frequency > MAX_FREQUENCY)
            continue;
        
        // Convert to screen coordinates
        const float x = frequencyToX(frequency);
        const float y = magnitudeToY(spectrum[i]);
        
        // Add to path
        if (!pathStarted)
        {
            path.startNewSubPath(x, y);
            pathStarted = true;
        }
        else
        {
            path.lineTo(x, y);
        }
    }
    
    return path;
}

float SpectrumDisplay::frequencyToX(float frequency) const
{
    const float bounds = getLocalBounds().getWidth();
    const float logFreq = std::log10(frequency);
    const float logMin = std::log10(MIN_FREQUENCY);
    const float logMax = std::log10(MAX_FREQUENCY);
    
    return bounds * (logFreq - logMin) / (logMax - logMin);
}

float SpectrumDisplay::magnitudeToY(float magnitudeDB) const
{
    const float bounds = getLocalBounds().getHeight();
    const float normalizedMagnitude = (magnitudeDB - MIN_MAGNITUDE_DB) / (MAX_MAGNITUDE_DB - MIN_MAGNITUDE_DB);
    
    return bounds * (1.0f - normalizedMagnitude); // Invert Y axis
}