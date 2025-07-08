#include "FrequencyResponseDisplay.h"

FrequencyResponseDisplay::FrequencyResponseDisplay(SpectrumAnalyzer& analyzer)
    : spectrumAnalyzer(analyzer)
{
    // Start timer for 30Hz updates
    startTimer(static_cast<int>(1000.0f / UPDATE_RATE_HZ));
}

void FrequencyResponseDisplay::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(backgroundColour);
    
    // Draw border
    g.setColour(gridColour.brighter(0.3f));
    g.drawRect(getLocalBounds(), 1);
    
    // Draw grid and labels
    drawFrequencyGrid(g);
    drawMagnitudeGrid(g);
    drawFrequencyLabels(g);
    drawMagnitudeLabels(g);
    
    // Draw spectrum if visible and data available
    if (spectrumVisible)
    {
        // Draw input spectrum
        if ((displayMode == DisplayMode::Input || displayMode == DisplayMode::Both) && !inputSpectrum.empty())
        {
            juce::Path inputPath = createSpectrumPath(inputSpectrum);
            g.setColour(inputSpectrumColour);
            g.strokePath(inputPath, juce::PathStrokeType(2.0f));
        }
        
        // Draw output spectrum
        if ((displayMode == DisplayMode::Output || displayMode == DisplayMode::Both) && !outputSpectrum.empty())
        {
            juce::Path outputPath = createSpectrumPath(outputSpectrum);
            g.setColour(outputSpectrumColour);
            g.strokePath(outputPath, juce::PathStrokeType(2.0f));
        }
    }
    
    // Draw title
    g.setColour(textColour);
    g.setFont(12.0f);
    g.drawText("Frequency Response", getLocalBounds().removeFromTop(15).reduced(5), juce::Justification::centredLeft);
}

void FrequencyResponseDisplay::resized()
{
    // Component resized, paths will be recreated on next paint
}

void FrequencyResponseDisplay::setDisplayMode(DisplayMode mode)
{
    displayMode = mode;
    repaint();
}

void FrequencyResponseDisplay::setSpectrumVisible(bool visible)
{
    spectrumVisible = visible;
    repaint();
}

void FrequencyResponseDisplay::timerCallback()
{
    updateSpectrumData();
}

void FrequencyResponseDisplay::updateSpectrumData()
{
    // Get fresh spectrum data
    inputSpectrum = spectrumAnalyzer.getInputSpectrum();
    outputSpectrum = spectrumAnalyzer.getOutputSpectrum();
    
    // Trigger repaint
    repaint();
}

void FrequencyResponseDisplay::drawFrequencyGrid(juce::Graphics& g)
{
    g.setColour(gridColour);
    
    // Frequency grid lines (logarithmic)
    const float frequencies[] = {20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f};
    
    for (float freq : frequencies)
    {
        float x = frequencyToX(freq);
        g.drawVerticalLine(static_cast<int>(x), 0.0f, static_cast<float>(getHeight()));
    }
}

void FrequencyResponseDisplay::drawMagnitudeGrid(juce::Graphics& g)
{
    g.setColour(gridColour);
    
    // Magnitude grid lines (linear in dB) - equal positive and negative ranges
    const float magnitudes[] = {-40.0f, -30.0f, -20.0f, -10.0f, 0.0f, 10.0f, 20.0f, 30.0f, 40.0f};
    
    for (float mag : magnitudes)
    {
        float y = magnitudeToY(mag);
        
        // Draw 0dB line thicker
        if (std::abs(mag) < 0.1f)
        {
            g.setColour(gridColour.brighter(0.5f));
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, static_cast<float>(getWidth()));
            g.setColour(gridColour);
        }
        else
        {
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, static_cast<float>(getWidth()));
        }
    }
}

void FrequencyResponseDisplay::drawFrequencyLabels(juce::Graphics& g)
{
    g.setColour(textColour);
    g.setFont(9.0f);
    
    // Frequency labels
    const struct { float freq; const char* label; } frequencies[] = {
        {20.0f, "20Hz"}, {50.0f, "50"}, {100.0f, "100"}, {200.0f, "200"}, {500.0f, "500"},
        {1000.0f, "1k"}, {2000.0f, "2k"}, {5000.0f, "5k"}, {10000.0f, "10k"}, {20000.0f, "20k"}
    };
    
    for (const auto& freq : frequencies)
    {
        float x = frequencyToX(freq.freq);
        g.drawText(freq.label, static_cast<int>(x - 15), getHeight() - 15, 30, 12, juce::Justification::centred);
    }
}

void FrequencyResponseDisplay::drawMagnitudeLabels(juce::Graphics& g)
{
    g.setColour(textColour);
    g.setFont(9.0f);
    
    // Magnitude labels - equal positive and negative ranges
    const float magnitudes[] = {-40.0f, -30.0f, -20.0f, -10.0f, 0.0f, 10.0f, 20.0f, 30.0f, 40.0f};
    
    for (float mag : magnitudes)
    {
        float y = magnitudeToY(mag);
        juce::String label = (mag == 0.0f) ? "0dB" : juce::String(static_cast<int>(mag));
        g.drawText(label, 2, static_cast<int>(y - 6), 30, 12, juce::Justification::centredLeft);
    }
}

juce::Path FrequencyResponseDisplay::createSpectrumPath(const std::vector<float>& spectrum)
{
    juce::Path path;
    
    if (spectrum.empty())
        return path;
    
    const auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    
    // Account for margins
    const float margin = 20.0f;
    const float effectiveWidth = width - (margin * 2);
    const float effectiveHeight = height - (margin * 2);
    
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
        
        // Convert to screen coordinates with margins
        const float x = margin + (frequencyToX(frequency) - margin) * (effectiveWidth / (width - margin * 2));
        const float y = margin + (magnitudeToY(spectrum[i]) - margin) * (effectiveHeight / (height - margin * 2));
        
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

float FrequencyResponseDisplay::frequencyToX(float frequency) const
{
    const float bounds = static_cast<float>(getLocalBounds().getWidth());
    const float logFreq = std::log10(frequency);
    const float logMin = std::log10(MIN_FREQUENCY);
    const float logMax = std::log10(MAX_FREQUENCY);
    
    return bounds * (logFreq - logMin) / (logMax - logMin);
}

float FrequencyResponseDisplay::magnitudeToY(float magnitudeDB) const
{
    const float bounds = static_cast<float>(getLocalBounds().getHeight());
    const float normalizedMagnitude = (magnitudeDB - MIN_MAGNITUDE_DB) / (MAX_MAGNITUDE_DB - MIN_MAGNITUDE_DB);
    
    return bounds * (1.0f - normalizedMagnitude); // Invert Y axis
}