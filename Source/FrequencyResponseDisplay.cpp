#include "FrequencyResponseDisplay.h"
#include "PluginProcessor.h"
#include "DSP/EQBand.h"

FrequencyResponseDisplay::FrequencyResponseDisplay(SpectrumAnalyzer& analyzer)
    : spectrumAnalyzer(analyzer), audioProcessor(nullptr)
{
    // Start timer for 30Hz updates
    startTimer(static_cast<int>(1000.0f / UPDATE_RATE_HZ));
    
    // Initialize EQ points
    for (int i = 0; i < 4; ++i) {
        eqPoints[i].bandIndex = i;
        eqPoints[i].isActive = true;
    }
}

FrequencyResponseDisplay::FrequencyResponseDisplay(SpectrumAnalyzer& analyzer, VaclisDynamicEQAudioProcessor& processor)
    : spectrumAnalyzer(analyzer), audioProcessor(&processor)
{
    // Start timer for 30Hz updates
    startTimer(static_cast<int>(1000.0f / UPDATE_RATE_HZ));
    
    // Initialize EQ points
    for (int i = 0; i < 4; ++i) {
        eqPoints[i].bandIndex = i;
        eqPoints[i].isActive = true;
    }
    
    // Update EQ points from current parameters
    updateEQPointsFromParameters();
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
        // Save graphics state and create clipping region
        g.saveState();
        g.reduceClipRegion(getLocalBounds());
        
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
        
        // Restore graphics state
        g.restoreState();
    }
    
    // Draw EQ curve if enabled and processor available
    if (showEQCurve && audioProcessor != nullptr)
    {
        g.setColour(eqCurveColour.withAlpha(0.8f));
        g.strokePath(createEQCurvePath(), juce::PathStrokeType(2.0f));
    }
    
    // Draw EQ points
    for (int band = 0; band < 4; ++band)
    {
        auto& point = eqPoints[band];
        if (point.isActive)
        {
            g.setColour(getBandColour(band));
            
            if (point.isHovered || draggingBandIndex == band)
            {
                // Draw highlighted point
                g.fillEllipse(point.screenPosition.x - 6, point.screenPosition.y - 6, 12, 12);
                g.setColour(juce::Colours::white);
                g.drawEllipse(point.screenPosition.x - 6, point.screenPosition.y - 6, 12, 12, 1.5f);
            }
            else
            {
                // Draw normal point
                g.fillEllipse(point.screenPosition.x - 4, point.screenPosition.y - 4, 8, 8);
            }
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
    
    // Update EQ point screen positions after resize
    if (audioProcessor != nullptr)
    {
        updateEQPointScreenPositions();
    }
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
    
    // Update EQ points from parameters if processor is available
    if (audioProcessor != nullptr)
    {
        updateEQPointsFromParameters();
    }
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
    
    // Magnitude grid lines matching the actual EQ range (+12 to -24 dB)
    const float magnitudes[] = {-24.0f, -18.0f, -12.0f, -6.0f, 0.0f, 6.0f, 12.0f};
    
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
    
    // Magnitude labels matching the actual EQ range (+12 to -24 dB)
    const float magnitudes[] = {-24.0f, -18.0f, -12.0f, -6.0f, 0.0f, 6.0f, 12.0f};
    
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
        
        // Clamp spectrum value to display range
        const float clampedMagnitude = juce::jlimit(MIN_MAGNITUDE_DB, MAX_MAGNITUDE_DB, spectrum[i]);
        
        // Convert to screen coordinates
        const float x = frequencyToX(frequency);
        const float y = magnitudeToY(clampedMagnitude);
        
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

// Mouse interaction methods
void FrequencyResponseDisplay::mouseDown(const juce::MouseEvent& event)
{
    auto position = event.position;
    int nearestPoint = findNearestEQPoint(position);
    
    if (nearestPoint != -1 && getDistanceToPoint(position, nearestPoint) < 12.0f)
    {
        draggingBandIndex = nearestPoint;
        isDragging = true;
        dragStartPosition = position;
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    }
}

void FrequencyResponseDisplay::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging && draggingBandIndex != -1)
    {
        updateEQPointPosition(draggingBandIndex, event.position);
        updateParameterFromEQPoint(draggingBandIndex);
        invalidateResponseCache();
        repaint();
    }
}

void FrequencyResponseDisplay::mouseUp(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        isDragging = false;
        draggingBandIndex = -1;
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void FrequencyResponseDisplay::mouseMove(const juce::MouseEvent& event)
{
    bool repaintNeeded = false;
    
    // Check hover state for all points
    for (int i = 0; i < 4; ++i)
    {
        bool wasHovered = eqPoints[i].isHovered;
        eqPoints[i].isHovered = (getDistanceToPoint(event.position, i) < 12.0f);
        
        if (wasHovered != eqPoints[i].isHovered)
            repaintNeeded = true;
    }
    
    if (repaintNeeded)
        repaint();
}

int FrequencyResponseDisplay::findNearestEQPoint(juce::Point<float> position)
{
    float minDistance = std::numeric_limits<float>::max();
    int nearestPoint = -1;
    
    for (int i = 0; i < 4; ++i)
    {
        if (eqPoints[i].isActive)
        {
            float distance = getDistanceToPoint(position, i);
            if (distance < minDistance)
            {
                minDistance = distance;
                nearestPoint = i;
            }
        }
    }
    
    return nearestPoint;
}

void FrequencyResponseDisplay::updateEQPointPosition(int bandIndex, juce::Point<float> position)
{
    if (bandIndex >= 0 && bandIndex < 4)
    {
        eqPoints[bandIndex].screenPosition = position;
        
        // Convert screen position to parameter values
        eqPoints[bandIndex].frequency = xToFrequency(position.x);
        eqPoints[bandIndex].gainDB = yToGainDB(position.y);
        
        // Clamp values to valid ranges
        eqPoints[bandIndex].frequency = juce::jlimit(20.0f, 20000.0f, eqPoints[bandIndex].frequency);
        eqPoints[bandIndex].gainDB = juce::jlimit(-24.0f, 12.0f, eqPoints[bandIndex].gainDB);
    }
}

void FrequencyResponseDisplay::updateParameterFromEQPoint(int bandIndex)
{
    if (audioProcessor == nullptr || bandIndex < 0 || bandIndex >= 4)
        return;
        
    auto& point = eqPoints[bandIndex];
    
    // Update frequency parameter
    juce::String freqParamID = "eq_freq_band" + juce::String(bandIndex);
    if (auto* freqParam = audioProcessor->getValueTreeState().getParameter(freqParamID))
    {
        float normalizedFreq = std::log10(point.frequency / 20.0f) / std::log10(20000.0f / 20.0f);
        freqParam->setValueNotifyingHost(normalizedFreq);
    }
    
    // Update gain parameter
    juce::String gainParamID = "eq_gain_band" + juce::String(bandIndex);
    if (auto* gainParam = audioProcessor->getValueTreeState().getParameter(gainParamID))
    {
        // Convert from actual dB range (-24 to +12) to normalized (0-1)
        float normalizedGain = (point.gainDB + 24.0f) / 36.0f;
        gainParam->setValueNotifyingHost(normalizedGain);
    }
}

void FrequencyResponseDisplay::updateEQPointsFromParameters()
{
    if (audioProcessor == nullptr)
        return;
        
    for (int i = 0; i < 4; ++i)
    {
        // Get frequency parameter
        juce::String freqParamID = "eq_freq_band" + juce::String(i);
        if (auto* freqParam = audioProcessor->getValueTreeState().getParameter(freqParamID))
        {
            float normalizedFreq = freqParam->getValue();
            eqPoints[i].frequency = 20.0f * std::pow(20000.0f / 20.0f, normalizedFreq);
        }
        else
        {
            // Set default frequency if parameter not found
            const float defaultFreqs[4] = {100.0f, 500.0f, 2000.0f, 8000.0f};
            eqPoints[i].frequency = defaultFreqs[i];
        }
        
        // Get gain parameter
        juce::String gainParamID = "eq_gain_band" + juce::String(i);
        if (auto* gainParam = audioProcessor->getValueTreeState().getParameter(gainParamID))
        {
            float normalizedGain = gainParam->getValue();
            // Convert from normalized (0-1) to actual dB range (-24 to +12)
            eqPoints[i].gainDB = -24.0f + (normalizedGain * 36.0f);
        }
        else
        {
            // Set default gain if parameter not found
            eqPoints[i].gainDB = 0.0f;
        }
        
        // Get Q parameter
        juce::String qParamID = "eq_q_band" + juce::String(i);
        if (auto* qParam = audioProcessor->getValueTreeState().getParameter(qParamID))
        {
            float normalizedQ = qParam->getValue();
            eqPoints[i].Q = 0.1f + (normalizedQ * 9.9f); // 0.1 to 10.0
        }
        else
        {
            // Set default Q if parameter not found
            eqPoints[i].Q = 1.0f;
        }
        
        // Update screen position
        eqPoints[i].screenPosition.x = frequencyToX(eqPoints[i].frequency);
        eqPoints[i].screenPosition.y = magnitudeToY(eqPoints[i].gainDB);
        
        // Ensure point is marked as active
        eqPoints[i].isActive = true;
    }
}

float FrequencyResponseDisplay::getDistanceToPoint(juce::Point<float> position, int bandIndex)
{
    if (bandIndex < 0 || bandIndex >= 4)
        return std::numeric_limits<float>::max();
        
    return position.getDistanceFrom(eqPoints[bandIndex].screenPosition);
}

// EQ curve calculation methods
juce::Path FrequencyResponseDisplay::createEQCurvePath()
{
    juce::Path curvePath;
    
    if (audioProcessor == nullptr)
        return curvePath;
    
    auto combinedResponse = calculateCombinedEQResponse(512);
    
    if (combinedResponse.empty())
        return curvePath;
    
    // Create smooth curve path
    curvePath.startNewSubPath(0, magnitudeToY(combinedResponse[0]));
    
    for (size_t i = 1; i < combinedResponse.size(); ++i)
    {
        float x = (float)i / (combinedResponse.size() - 1) * getWidth();
        float y = magnitudeToY(combinedResponse[i]);
        curvePath.lineTo(x, y);
    }
    
    return curvePath;
}

std::vector<float> FrequencyResponseDisplay::calculateBandResponse(int bandIndex, int numPoints)
{
    std::vector<float> response(numPoints, 0.0f);
    
    if (audioProcessor == nullptr || bandIndex < 0 || bandIndex >= 4)
        return response;
    
    // Get current band parameters
    auto& point = eqPoints[bandIndex];
    
    // Get filter type from parameter
    DynamicEQ::FilterType filterType = DynamicEQ::FilterType::Bell; // Default
    juce::String typeParamID = "eq_type_band" + juce::String(bandIndex);
    if (auto* typeParam = audioProcessor->getValueTreeState().getParameter(typeParamID))
    {
        int typeIndex = static_cast<int>(typeParam->getValue() * 4.99f); // 0-4 range
        filterType = static_cast<DynamicEQ::FilterType>(typeIndex);
    }
    
    const float sampleRate = 48000.0f; // Assume 48kHz for visualization
    
    // Calculate response for each frequency point
    for (int i = 0; i < numPoints; ++i)
    {
        float freq = MIN_FREQUENCY * std::pow(MAX_FREQUENCY / MIN_FREQUENCY, (float)i / (numPoints - 1));
        float magnitude = 0.0f;
        
        // Calculate different filter responses based on type
        switch (filterType)
        {
            case DynamicEQ::FilterType::Bell:
                magnitude = calculateBellResponse(freq, point.frequency, point.gainDB, point.Q);
                break;
                
            case DynamicEQ::FilterType::HighShelf:
                magnitude = calculateHighShelfResponse(freq, point.frequency, point.gainDB, point.Q);
                break;
                
            case DynamicEQ::FilterType::LowShelf:
                magnitude = calculateLowShelfResponse(freq, point.frequency, point.gainDB, point.Q);
                break;
                
            case DynamicEQ::FilterType::HighPass:
                magnitude = calculateHighPassResponse(freq, point.frequency, point.Q);
                break;
                
            case DynamicEQ::FilterType::LowPass:
                magnitude = calculateLowPassResponse(freq, point.frequency, point.Q);
                break;
                
            default:
                magnitude = 0.0f;
                break;
        }
        
        response[i] = magnitude;
    }
    
    return response;
}

std::vector<float> FrequencyResponseDisplay::calculateCombinedEQResponse(int numPoints)
{
    std::vector<float> combinedResponse(numPoints, 0.0f);
    
    if (audioProcessor == nullptr)
        return combinedResponse;
    
    // Check if we can use cached response
    if (responseCacheValid && cachedCombinedResponse.size() == numPoints)
        return cachedCombinedResponse;
    
    // Calculate combined response by summing all active bands
    for (int band = 0; band < 4; ++band)
    {
        if (eqPoints[band].isActive)
        {
            auto bandResponse = calculateBandResponse(band, numPoints);
            for (int i = 0; i < numPoints; ++i)
            {
                combinedResponse[i] += bandResponse[i];
            }
        }
    }
    
    // Cache the result
    cachedCombinedResponse = combinedResponse;
    responseCacheValid = true;
    
    return combinedResponse;
}

// Coordinate conversion helpers
float FrequencyResponseDisplay::xToFrequency(float x) const
{
    const float bounds = static_cast<float>(getLocalBounds().getWidth());
    const float normalizedX = x / bounds;
    const float logMin = std::log10(MIN_FREQUENCY);
    const float logMax = std::log10(MAX_FREQUENCY);
    
    return std::pow(10.0f, logMin + normalizedX * (logMax - logMin));
}

float FrequencyResponseDisplay::yToGainDB(float y) const
{
    const float bounds = static_cast<float>(getLocalBounds().getHeight());
    const float normalizedY = 1.0f - (y / bounds); // Invert Y axis
    
    return MIN_MAGNITUDE_DB + normalizedY * (MAX_MAGNITUDE_DB - MIN_MAGNITUDE_DB);
}

// Band color helper
juce::Colour FrequencyResponseDisplay::getBandColour(int bandIndex) const
{
    const juce::Colour bandColours[4] = {
        juce::Colour(0xff4a9eff),  // LOW - Blue
        juce::Colour(0xff4aff9e),  // LOW-MID - Green
        juce::Colour(0xffff9e4a),  // HIGH-MID - Orange
        juce::Colour(0xffff4a4a)   // HIGH - Red
    };
    
    return (bandIndex >= 0 && bandIndex < 4) ? bandColours[bandIndex] : juce::Colours::white;
}

// Cache management
void FrequencyResponseDisplay::invalidateResponseCache()
{
    responseCacheValid = false;
}

void FrequencyResponseDisplay::updateEQPointScreenPositions()
{
    for (int i = 0; i < 4; ++i)
    {
        eqPoints[i].screenPosition.x = frequencyToX(eqPoints[i].frequency);
        eqPoints[i].screenPosition.y = magnitudeToY(eqPoints[i].gainDB);
    }
}

// Filter response calculation methods
float FrequencyResponseDisplay::calculateBellResponse(float freq, float centerFreq, float gainDB, float Q) const
{
    if (std::abs(gainDB) < 0.01f) return 0.0f; // No gain, no response
    
    // Use chowdsp's actual bell filter response
    chowdsp::EQ::BellPlot bellPlot;
    bellPlot.setCutoffFrequency(centerFreq);
    bellPlot.setGainDecibels(gainDB);
    bellPlot.setQValue(Q);
    
    // Get the actual magnitude response and convert to dB
    float magnitude = bellPlot.getMagnitudeForFrequency(freq);
    return juce::Decibels::gainToDecibels(magnitude);
}

float FrequencyResponseDisplay::calculateHighShelfResponse(float freq, float cutoffFreq, float gainDB, float Q) const
{
    if (std::abs(gainDB) < 0.01f) return 0.0f;
    
    // Manual high shelf calculation (corrected)
    const float omega = freq / cutoffFreq;
    const float A = std::pow(10.0f, gainDB / 40.0f); // Convert dB to linear amplitude
    
    // High shelf: gain at high frequencies, unity at low frequencies
    if (omega >= 4.0f) // High frequencies
    {
        return gainDB; // Full gain
    }
    else if (omega <= 0.25f) // Low frequencies
    {
        return 0.0f; // Unity gain (0dB)
    }
    else // Transition region
    {
        const float logOmega = std::log2(omega);
        const float sharpness = Q * 0.5f; // Q controls transition sharpness
        const float transition = 0.5f * (1.0f + std::tanh(logOmega * sharpness));
        return gainDB * transition;
    }
}

float FrequencyResponseDisplay::calculateLowShelfResponse(float freq, float cutoffFreq, float gainDB, float Q) const
{
    if (std::abs(gainDB) < 0.01f) return 0.0f;
    
    // Manual low shelf calculation (corrected)
    const float omega = freq / cutoffFreq;
    const float A = std::pow(10.0f, gainDB / 40.0f); // Convert dB to linear amplitude
    
    // Low shelf: gain at low frequencies, unity at high frequencies
    if (omega <= 0.25f) // Low frequencies
    {
        return gainDB; // Full gain
    }
    else if (omega >= 4.0f) // High frequencies
    {
        return 0.0f; // Unity gain (0dB)
    }
    else // Transition region
    {
        const float logOmega = std::log2(omega);
        const float sharpness = Q * 0.5f; // Q controls transition sharpness
        const float transition = 0.5f * (1.0f - std::tanh(logOmega * sharpness));
        return gainDB * transition;
    }
}

float FrequencyResponseDisplay::calculateHighPassResponse(float freq, float cutoffFreq, float Q) const
{
    // Proper 2nd order high-pass filter response
    const float omega = freq / cutoffFreq;
    const float omega2 = omega * omega;
    
    // Standard 2nd order high-pass transfer function: H(s) = s^2 / (s^2 + s/Q + 1)
    // At s = jω, |H(jω)|^2 = ω^4 / (ω^4 + ω^2/Q^2 + 1)
    const float numerator = omega2 * omega2;
    const float denominator = numerator + (omega2 / (Q * Q)) + 1.0f;
    
    const float magnitude_squared = numerator / denominator;
    
    // Convert to dB
    if (magnitude_squared > 1e-10f)
        return 10.0f * std::log10(magnitude_squared);
    else
        return -100.0f; // Very low magnitude
}

float FrequencyResponseDisplay::calculateLowPassResponse(float freq, float cutoffFreq, float Q) const
{
    // Proper 2nd order low-pass filter response
    const float omega = freq / cutoffFreq;
    const float omega2 = omega * omega;
    
    // Standard 2nd order low-pass transfer function: H(s) = 1 / (s^2 + s/Q + 1)
    // At s = jω, |H(jω)|^2 = 1 / (ω^4 + ω^2/Q^2 + 1)
    const float denominator = omega2 * omega2 + (omega2 / (Q * Q)) + 1.0f;
    
    const float magnitude_squared = 1.0f / denominator;
    
    // Convert to dB
    if (magnitude_squared > 1e-10f)
        return 10.0f * std::log10(magnitude_squared);
    else
        return -100.0f; // Very low magnitude
}