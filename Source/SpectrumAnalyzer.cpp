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

// VTR3 Feature extraction methods
std::vector<float> SpectrumAnalyzer::extractFeatures(const std::vector<float>& audioData, double sampleRate)
{
    std::vector<float> features(TOTAL_FEATURES);
    
    // Extract RMS energy (index 0)
    features[0] = extractRMSEnergy(audioData);
    
    // Compute power spectrum for frequency-domain features
    std::vector<float> powerSpectrum = computePowerSpectrum(audioData);
    
    // Extract spectral centroid (index 1)
    features[1] = extractSpectralCentroid(powerSpectrum, sampleRate);
    
    // Extract MFCC coefficients (indices 2-14)
    std::vector<float> mfccCoeffs = extractMFCC(powerSpectrum, sampleRate);
    for (int i = 0; i < NUM_MFCC_COEFFS && i < static_cast<int>(mfccCoeffs.size()); ++i)
    {
        features[2 + i] = mfccCoeffs[i];
    }
    
    // Add two additional features (spectral rolloff and flux)
    features[15] = 0.0f; // Placeholder for spectral rolloff
    features[16] = 0.0f; // Placeholder for spectral flux
    
    return features;
}

std::vector<float> SpectrumAnalyzer::extractMFCC(const std::vector<float>& powerSpectrum, double sampleRate)
{
    // Compute mel filterbank energies
    std::vector<float> melEnergies = computeMelFilterbank(powerSpectrum, sampleRate);
    
    // Apply log to mel energies
    for (auto& energy : melEnergies)
    {
        energy = std::log(std::max(energy, 1e-10f)); // Prevent log(0)
    }
    
    // Apply DCT to get MFCC coefficients
    return computeDCT(melEnergies);
}

float SpectrumAnalyzer::extractSpectralCentroid(const std::vector<float>& powerSpectrum, double sampleRate)
{
    double weightedSum = 0.0;
    double totalEnergy = 0.0;
    
    for (size_t i = 1; i < powerSpectrum.size(); ++i) // Start from 1 to skip DC
    {
        // Convert bin index to frequency
        double frequency = (i * sampleRate) / (2.0 * powerSpectrum.size());
        
        // Weight by power and frequency
        double power = powerSpectrum[i];
        weightedSum += frequency * power;
        totalEnergy += power;
    }
    
    // Return weighted average frequency
    return totalEnergy > 0.0 ? static_cast<float>(weightedSum / totalEnergy) : 0.0f;
}

float SpectrumAnalyzer::extractRMSEnergy(const std::vector<float>& audioData)
{
    double sumSquares = 0.0;
    
    for (float sample : audioData)
    {
        sumSquares += sample * sample;
    }
    
    return std::sqrt(sumSquares / audioData.size());
}

std::vector<float> SpectrumAnalyzer::computeMelFilterbank(const std::vector<float>& powerSpectrum, double sampleRate)
{
    std::vector<float> melEnergies(NUM_MEL_FILTERS, 0.0f);
    
    // Frequency range for mel filters (0 Hz to Nyquist)
    double maxFreq = sampleRate / 2.0;
    
    // Convert to mel scale
    float minMel = melScale(0.0f);
    float maxMel = melScale(maxFreq);
    
    // Create mel filter bank
    for (int m = 0; m < NUM_MEL_FILTERS; ++m)
    {
        // Calculate mel frequencies for this filter
        float leftMel = minMel + (m * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        float centerMel = minMel + ((m + 1) * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        float rightMel = minMel + ((m + 2) * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        
        // Convert back to linear frequency
        float leftFreq = invMelScale(leftMel);
        float centerFreq = invMelScale(centerMel);
        float rightFreq = invMelScale(rightMel);
        
        // Convert frequencies to bin indices
        int leftBin = static_cast<int>((leftFreq * 2.0f * powerSpectrum.size()) / sampleRate);
        int centerBin = static_cast<int>((centerFreq * 2.0f * powerSpectrum.size()) / sampleRate);
        int rightBin = static_cast<int>((rightFreq * 2.0f * powerSpectrum.size()) / sampleRate);
        
        // Ensure bins are within bounds
        leftBin = std::max(0, std::min(leftBin, static_cast<int>(powerSpectrum.size()) - 1));
        centerBin = std::max(0, std::min(centerBin, static_cast<int>(powerSpectrum.size()) - 1));
        rightBin = std::max(0, std::min(rightBin, static_cast<int>(powerSpectrum.size()) - 1));
        
        // Apply triangular filter
        for (int i = leftBin; i <= rightBin; ++i)
        {
            float weight = 0.0f;
            
            if (i <= centerBin)
            {
                // Rising edge
                if (centerBin > leftBin)
                {
                    weight = static_cast<float>(i - leftBin) / (centerBin - leftBin);
                }
            }
            else
            {
                // Falling edge
                if (rightBin > centerBin)
                {
                    weight = static_cast<float>(rightBin - i) / (rightBin - centerBin);
                }
            }
            
            melEnergies[m] += weight * powerSpectrum[i];
        }
    }
    
    return melEnergies;
}

std::vector<float> SpectrumAnalyzer::computeDCT(const std::vector<float>& melEnergies)
{
    std::vector<float> dctCoeffs(NUM_MFCC_COEFFS, 0.0f);
    
    const int N = melEnergies.size();
    const float sqrt2OverN = std::sqrt(2.0f / N);
    
    for (int k = 0; k < NUM_MFCC_COEFFS; ++k)
    {
        double sum = 0.0;
        
        for (int n = 0; n < N; ++n)
        {
            double angle = juce::MathConstants<double>::pi * k * (n + 0.5) / N;
            sum += melEnergies[n] * std::cos(angle);
        }
        
        dctCoeffs[k] = sqrt2OverN * sum;
    }
    
    return dctCoeffs;
}

// Helper methods
float SpectrumAnalyzer::melScale(float frequency)
{
    return 2595.0f * std::log10(1.0f + frequency / 700.0f);
}

float SpectrumAnalyzer::invMelScale(float mel)
{
    return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
}

std::vector<float> SpectrumAnalyzer::computePowerSpectrum(const std::vector<float>& audioData)
{
    // Ensure we have enough data for FFT
    std::vector<float> paddedData = audioData;
    paddedData.resize(FFT_SIZE, 0.0f);
    
    // Apply Hann window
    applyHannWindow(paddedData);
    
    // Prepare FFT data (real + imaginary)
    std::vector<float> fftData(FFT_SIZE * 2, 0.0f);
    
    // Copy real part
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        fftData[i] = paddedData[i];
    }
    
    // Perform FFT
    fft.performFrequencyOnlyForwardTransform(fftData.data());
    
    // Convert to power spectrum (magnitude squared)
    std::vector<float> powerSpectrum(FFT_SIZE / 2);
    for (int i = 0; i < FFT_SIZE / 2; ++i)
    {
        powerSpectrum[i] = fftData[i] * fftData[i];
    }
    
    return powerSpectrum;
}