#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer()
    : fft(FFT_ORDER)
{
    // Initialize buffers
    inputFifo.resize(FFT_SIZE, 0.0f);
    outputFifo.resize(FFT_SIZE, 0.0f);
    inputFFTData.resize(FFT_SIZE * 2, 0.0f);
    outputFFTData.resize(FFT_SIZE * 2, 0.0f);
    
    // Initialize spectrum data (half of FFT size for real spectrum)
    const int spectrumSize = FFT_SIZE / 2;
    inputSpectrum.resize(spectrumSize, 0.0f);
    outputSpectrum.resize(spectrumSize, 0.0f);
    inputPeakHold.resize(spectrumSize, 0.0f);
    outputPeakHold.resize(spectrumSize, 0.0f);
    inputPeakTimer.resize(spectrumSize, 0.0f);
    outputPeakTimer.resize(spectrumSize, 0.0f);
}

void SpectrumAnalyzer::prepare(double sampleRateToUse, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    this->sampleRate = sampleRateToUse;
    
    // Calculate peak decay rate based on update rate and hold time
    const float updateInterval = 1.0f / UPDATE_RATE_HZ;
    peakDecayRate = updateInterval / PEAK_HOLD_TIME_SECONDS;
    
    // Reset buffers
    std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
    std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
    std::fill(inputPeakHold.begin(), inputPeakHold.end(), 0.0f);
    std::fill(outputPeakHold.begin(), outputPeakHold.end(), 0.0f);
    std::fill(inputPeakTimer.begin(), inputPeakTimer.end(), 0.0f);
    std::fill(outputPeakTimer.begin(), outputPeakTimer.end(), 0.0f);
    
    fifoIndex = 0;
    nextFFTBlockReady = false;
}

void SpectrumAnalyzer::processBlock(const juce::AudioBuffer<float>& inputBuffer, const juce::AudioBuffer<float>& outputBuffer)
{
    const int numSamples = inputBuffer.getNumSamples();
    const int numChannels = inputBuffer.getNumChannels();
    
    // Process samples into FIFO
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Mix channels to mono for spectrum analysis
        float inputSample = 0.0f;
        float outputSample = 0.0f;
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            inputSample += inputBuffer.getSample(channel, sample);
            outputSample += outputBuffer.getSample(channel, sample);
        }
        
        // Average the channels
        if (numChannels > 1)
        {
            inputSample /= numChannels;
            outputSample /= numChannels;
        }
        
        // Store in FIFO
        inputFifo[fifoIndex] = inputSample;
        outputFifo[fifoIndex] = outputSample;
        
        fifoIndex++;
        
        // Check if we have enough samples for FFT
        if (fifoIndex >= FFT_SIZE)
        {
            nextFFTBlockReady = true;
            fifoIndex = 0;
        }
    }
    
    // Perform FFT if we have enough data
    if (nextFFTBlockReady)
    {
        // Create temporary buffers for FFT
        float* inputData = inputFifo.data();
        float* outputData = outputFifo.data();
        
        juce::AudioBuffer<float> inputFFTBuffer(&inputData, 1, FFT_SIZE);
        juce::AudioBuffer<float> outputFFTBuffer(&outputData, 1, FFT_SIZE);
        
        performFFT(inputFFTBuffer, inputSpectrum);
        performFFT(outputFFTBuffer, outputSpectrum);
        
        // Update peak hold
        updatePeakHold(inputSpectrum, inputPeakHold);
        updatePeakHold(outputSpectrum, outputPeakHold);
        
        nextFFTBlockReady = false;
    }
}

void SpectrumAnalyzer::performFFT(const juce::AudioBuffer<float>& buffer, std::vector<float>& spectrumData)
{
    std::lock_guard<std::mutex> lock(spectrumMutex);
    
    // Copy data to FFT buffer
    std::vector<float> fftData(FFT_SIZE * 2, 0.0f);
    
    // Copy audio data to real part of FFT buffer
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        fftData[i] = buffer.getSample(0, i);
    }
    
    // Apply Hann window
    applyHannWindow(fftData);
    
    // Perform FFT
    fft.performFrequencyOnlyForwardTransform(fftData.data());
    
    // Convert to magnitude spectrum
    const int spectrumSize = FFT_SIZE / 2;
    for (int i = 0; i < spectrumSize; ++i)
    {
        const float magnitude = fftData[i];
        
        // Convert to dB with floor to prevent log(0)
        const float magnitudeDB = magnitude > 0.0f ? 
            juce::Decibels::gainToDecibels(magnitude) : 
            -120.0f; // -120dB floor
        
        spectrumData[i] = magnitudeDB;
    }
}

void SpectrumAnalyzer::applyHannWindow(std::vector<float>& data)
{
    // Apply Hann window manually
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        const float windowValue = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (FFT_SIZE - 1)));
        data[i] *= windowValue;
    }
}

void SpectrumAnalyzer::updatePeakHold(std::vector<float>& spectrum, std::vector<float>& peakHold)
{
    for (size_t i = 0; i < spectrum.size(); ++i)
    {
        if (spectrum[i] > peakHold[i])
        {
            // New peak detected
            peakHold[i] = spectrum[i];
            
            // Reset timer for this bin
            if (i < inputPeakTimer.size())
                inputPeakTimer[i] = PEAK_HOLD_TIME_SECONDS;
            if (i < outputPeakTimer.size())
                outputPeakTimer[i] = PEAK_HOLD_TIME_SECONDS;
        }
        else
        {
            // Decay peak hold
            auto& timer = (peakHold.data() == inputPeakHold.data()) ? inputPeakTimer : outputPeakTimer;
            
            if (i < timer.size())
            {
                timer[i] -= peakDecayRate;
                if (timer[i] <= 0.0f)
                {
                    // Peak hold time expired, start decaying
                    peakHold[i] = juce::jmax(peakHold[i] - 0.5f, spectrum[i]); // 0.5dB/update decay
                    timer[i] = 0.0f;
                }
            }
        }
    }
}

std::vector<float> SpectrumAnalyzer::getInputSpectrum() const
{
    std::lock_guard<std::mutex> lock(spectrumMutex);
    return inputPeakHold; // Return peak hold data for better visualization
}

std::vector<float> SpectrumAnalyzer::getOutputSpectrum() const
{
    std::lock_guard<std::mutex> lock(spectrumMutex);
    return outputPeakHold; // Return peak hold data for better visualization
}