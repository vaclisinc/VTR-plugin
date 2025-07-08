#include "LevelMeter.h"

LevelMeter::LevelMeter()
{
    // Start timer for updates
    startTimer(static_cast<int>(1000.0f / UPDATE_RATE_HZ));
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    
    // Get current levels
    float currentDB = currentLevel.load();
    float peakDB = peakLevel.load();
    
    // Calculate positions
    float currentPos = dbToPosition(currentDB);
    float peakPos = dbToPosition(peakDB);
    
    // Draw level meter
    if (horizontal)
    {
        // Horizontal meter
        float meterWidth = bounds.getWidth() * currentPos;
        
        // Choose color based on level
        juce::Colour meterColour = levelColour;
        if (currentDB > -6.0f) meterColour = criticalColour;
        else if (currentDB > -12.0f) meterColour = warningColour;
        
        // Draw level bar
        if (meterWidth > 0)
        {
            auto levelRect = bounds.withWidth(meterWidth).reduced(1.0f);
            g.setColour(meterColour);
            g.fillRoundedRectangle(levelRect, 1.0f);
        }
        
        // Draw peak indicator
        if (peakPos > 0.01f)
        {
            float peakX = bounds.getWidth() * peakPos;
            g.setColour(peakColour);
            g.drawLine(peakX, bounds.getY() + 1, peakX, bounds.getBottom() - 1, 2.0f);
        }
    }
    else
    {
        // Vertical meter
        float meterHeight = bounds.getHeight() * currentPos;
        
        // Choose color based on level
        juce::Colour meterColour = levelColour;
        if (currentDB > -6.0f) meterColour = criticalColour;
        else if (currentDB > -12.0f) meterColour = warningColour;
        
        // Draw level bar (from bottom up)
        if (meterHeight > 0)
        {
            auto levelRect = bounds.withHeight(meterHeight).withBottomY(bounds.getBottom()).reduced(1.0f);
            g.setColour(meterColour);
            g.fillRoundedRectangle(levelRect, 1.0f);
        }
        
        // Draw peak indicator
        if (peakPos > 0.01f)
        {
            float peakY = bounds.getBottom() - (bounds.getHeight() * peakPos);
            g.setColour(peakColour);
            g.drawLine(bounds.getX() + 1, peakY, bounds.getRight() - 1, peakY, 2.0f);
        }
    }
}

void LevelMeter::resized()
{
    // Component resized - nothing specific to do
}

void LevelMeter::updateLevel(float newLevel)
{
    // Convert to dB
    float levelDB = newLevel > 0.0f ? juce::Decibels::gainToDecibels(newLevel) : -60.0f;
    
    // Update current level
    currentLevel.store(levelDB);
    
    // Update peak hold
    float currentPeakDB = peakLevel.load();
    if (levelDB > currentPeakDB)
    {
        peakLevel.store(levelDB);
        peakHoldTime = PEAK_HOLD_SECONDS;
    }
}

void LevelMeter::setOrientation(bool isHorizontal)
{
    horizontal = isHorizontal;
    repaint();
}

void LevelMeter::setRange(float minDecibels, float maxDecibels)
{
    minDB = minDecibels;
    maxDB = maxDecibels;
    repaint();
}

void LevelMeter::timerCallback()
{
    // Update peak hold decay
    if (peakHoldTime > 0.0f)
    {
        peakHoldTime -= (1.0f / UPDATE_RATE_HZ);
        if (peakHoldTime <= 0.0f)
        {
            // Start decaying peak
            float currentPeakDB = peakLevel.load();
            float decayedPeak = currentPeakDB - 0.5f; // 0.5dB per update decay
            peakLevel.store(juce::jmax(decayedPeak, currentLevel.load()));
        }
    }
    else
    {
        // Continue decaying peak
        float currentPeakDB = peakLevel.load();
        float currentLevelDB = currentLevel.load();
        if (currentPeakDB > currentLevelDB)
        {
            float decayedPeak = currentPeakDB - 0.3f; // 0.3dB per update decay
            peakLevel.store(juce::jmax(decayedPeak, currentLevelDB));
        }
    }
    
    // Trigger repaint
    repaint();
}

float LevelMeter::dbToPosition(float db) const
{
    // Clamp to range
    db = juce::jlimit(minDB, maxDB, db);
    
    // Convert to normalized position (0.0 to 1.0)
    return (db - minDB) / (maxDB - minDB);
}