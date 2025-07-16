#include "SpectrumAnalyzer.h"
#include <thread>

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
    
    // Initialize VTR3 feature extraction
    latestFeatures.resize(TOTAL_FEATURES, 0.0f);
    featureUpdateInterval = static_cast<int>(UPDATE_RATE_HZ / featureUpdateRateHz);
    
    // Initialize Essentia feature extractor
#ifdef HAVE_ESSENTIA
    essentiaExtractor = std::make_unique<EssentiaFeatureExtractor>();
#endif
    
    // Initialize feature extractor 
    featureExtractor = std::make_unique<FeatureExtractor>();
    
    // Initialize with JUCE backend first for immediate availability
    featureExtractor->initialize(44100.0, FFT_SIZE, FeatureExtractor::Backend::JUCE_BASED);
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
    
    // Prepare Essentia feature extractor
#ifdef HAVE_ESSENTIA
    if (essentiaExtractor)
    {
        essentiaExtractor->prepare(sampleRateToUse);
    }
#endif
    
    // Initialize feature extractor with current sample rate
    if (featureExtractor)
    {
        FeatureExtractor::Backend backend = FeatureExtractor::Backend::JUCE_BASED;
        
        if (currentBackend == FeatureExtractionBackend::LIBXTRACT_BASED)
        {
            backend = FeatureExtractor::Backend::LIBXTRACT_BASED;
        }
        else if (currentBackend == FeatureExtractionBackend::PYTHON_LIBROSA)
        {
            backend = FeatureExtractor::Backend::PYTHON_LIBROSA;
            std::cout << "SpectrumAnalyzer: Requesting Python backend initialization..." << std::endl;
        }
        
        featureExtractor->initialize(sampleRateToUse, FFT_SIZE, backend);
    }
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
        
        // VTR3 Feature extraction (if enabled)
        if (featureExtractionEnabled.load())
        {
            featureUpdateCounter++;
            if (featureUpdateCounter >= featureUpdateInterval)
            {
                extractAndStoreFeatures();
                featureUpdateCounter = 0;
            }
        }
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
    // Use the FeatureExtractor which supports Python backend
    if (featureExtractor)
    {
        std::cout << "SpectrumAnalyzer: Using FeatureExtractor backend" << std::endl;
        juce::Logger::writeToLog("SpectrumAnalyzer: Using FeatureExtractor (with Python support)");
        return featureExtractor->extractFeatures(audioData);
    }
    
    // Use Essentia-based feature extraction if available
#ifdef HAVE_ESSENTIA
    if (currentBackend == FeatureExtractionBackend::ESSENTIA_BASED && essentiaExtractor)
    {
        return essentiaExtractor->extractFeatures(audioData, sampleRate);
    }
#endif
    
    // Fall back to JUCE-based implementation
    std::vector<float> features(TOTAL_FEATURES);
    
    // Frame-by-frame processing like librosa
    const int hopLength = 512;  // librosa default
    const int frameSize = FFT_SIZE;
    
    // Calculate number of frames
    int numFrames = 0;
    if (audioData.size() >= frameSize)
    {
        numFrames = (audioData.size() - frameSize) / hopLength + 1;
    }
    
    if (numFrames == 0)
    {
        // Not enough data, return zeros
        std::fill(features.begin(), features.end(), 0.0f);
        return features;
    }
    
    // Collect features from all frames
    std::vector<float> allSpectralCentroids;
    std::vector<float> allSpectralBandwidths;
    std::vector<float> allSpectralRolloffs;
    std::vector<float> allRMSValues;
    std::vector<std::vector<float>> allMFCCFrames;
    
    for (int frameIdx = 0; frameIdx < numFrames; ++frameIdx)
    {
        int startIdx = frameIdx * hopLength;
        
        // Extract frame
        std::vector<float> frame(frameSize, 0.0f);
        for (int i = 0; i < frameSize && (startIdx + i) < audioData.size(); ++i)
        {
            frame[i] = audioData[startIdx + i];
        }
        
        // Calculate RMS for this frame
        float frameRMS = extractRMSEnergy(frame);
        allRMSValues.push_back(frameRMS);
        
        // Compute power spectrum for this frame
        std::vector<float> powerSpectrum = computePowerSpectrum(frame);
        
        // Extract spectral features for this frame
        float centroid = extractSpectralCentroid(powerSpectrum, sampleRate);
        float bandwidth = extractSpectralBandwidth(powerSpectrum, sampleRate);
        float rolloff = extractSpectralRolloff(powerSpectrum, sampleRate);
        
        allSpectralCentroids.push_back(centroid);
        allSpectralBandwidths.push_back(bandwidth);
        allSpectralRolloffs.push_back(rolloff);
        
        // Extract MFCC for this frame
        std::vector<float> mfccFrame;
        if (currentBackend == FeatureExtractionBackend::LIBXTRACT_BASED && featureExtractor)
        {
            // Use LibXtract approach via FeatureExtractor
            mfccFrame = featureExtractor->extractMFCC(frame, NUM_MFCC_COEFFS);
        }
        else
        {
            // Use traditional approach with power spectrum
            mfccFrame = extractMFCC(powerSpectrum, sampleRate);
        }
        allMFCCFrames.push_back(mfccFrame);
    }
    
    // Average RMS across all frames (index 0)
    float avgRMS = 0.0f;
    for (float rms : allRMSValues)
    {
        avgRMS += rms;
    }
    avgRMS /= allRMSValues.size();
    features[0] = avgRMS;
    
    // Average spectral centroid across all frames (index 1)
    float avgCentroid = 0.0f;
    for (float centroid : allSpectralCentroids)
    {
        avgCentroid += centroid;
    }
    avgCentroid /= allSpectralCentroids.size();
    features[1] = avgCentroid;
    
    // Average MFCC coefficients across all frames (indices 2-14)
    for (int mfccIdx = 0; mfccIdx < NUM_MFCC_COEFFS; ++mfccIdx)
    {
        float avgMFCC = 0.0f;
        for (const auto& mfccFrame : allMFCCFrames)
        {
            if (mfccIdx < mfccFrame.size())
            {
                avgMFCC += mfccFrame[mfccIdx];
            }
        }
        avgMFCC /= allMFCCFrames.size();
        features[2 + mfccIdx] = avgMFCC;
    }
    
    // Average spectral bandwidth across all frames (index 15)
    float avgBandwidth = 0.0f;
    for (float bandwidth : allSpectralBandwidths)
    {
        avgBandwidth += bandwidth;
    }
    avgBandwidth /= allSpectralBandwidths.size();
    features[15] = avgBandwidth;
    
    // Average spectral rolloff across all frames (index 16)
    float avgRolloff = 0.0f;
    for (float rolloff : allSpectralRolloffs)
    {
        avgRolloff += rolloff;
    }
    avgRolloff /= allSpectralRolloffs.size();
    features[16] = avgRolloff;
    
    return features;
}

std::vector<float> SpectrumAnalyzer::extractMFCC(const std::vector<float>& powerSpectrum, double sampleRate)
{
    // Use LibXtract backend if selected
    if (currentBackend == FeatureExtractionBackend::LIBXTRACT_BASED && featureExtractor)
    {
        // Convert power spectrum to audio data (approximation for compatibility)
        std::vector<float> audioData(powerSpectrum.size() * 2);
        for (size_t i = 0; i < powerSpectrum.size(); ++i) {
            audioData[i] = std::sqrt(powerSpectrum[i]); // Convert power to magnitude
        }
        return featureExtractor->extractMFCC(audioData, NUM_MFCC_COEFFS);
    }
    
#ifdef HAVE_ESSENTIA
    // Use Essentia backend if selected and available
    if (currentBackend == FeatureExtractionBackend::ESSENTIA_BASED && essentiaExtractor)
    {
        return essentiaExtractor->extractMFCC(powerSpectrum, sampleRate);
    }
#endif
    
    // Default to JUCE-based implementation
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
        // Convert bin index to frequency - librosa uses: freq = i * sr / n_fft
        // For power spectrum size N, the FFT size is 2 * (N - 1)
        int fftSize = 2 * (powerSpectrum.size() - 1);
        double frequency = (i * sampleRate) / fftSize;
        
        // Weight by power and frequency
        double power = powerSpectrum[i];
        weightedSum += frequency * power;
        totalEnergy += power;
    }
    
    // Return weighted average frequency
    return totalEnergy > 0.0 ? static_cast<float>(weightedSum / totalEnergy) : 0.0f;
}

float SpectrumAnalyzer::extractSpectralBandwidth(const std::vector<float>& powerSpectrum, double sampleRate)
{
    // First calculate the spectral centroid
    double centroid = extractSpectralCentroid(powerSpectrum, sampleRate);
    
    double weightedVariance = 0.0;
    double totalEnergy = 0.0;
    
    for (size_t i = 1; i < powerSpectrum.size(); ++i) // Start from 1 to skip DC
    {
        // Convert bin index to frequency - librosa uses: freq = i * sr / n_fft
        // For power spectrum size N, the FFT size is 2 * (N - 1)
        int fftSize = 2 * (powerSpectrum.size() - 1);
        double frequency = (i * sampleRate) / fftSize;
        
        // Calculate variance from centroid
        double power = powerSpectrum[i];
        double deviation = frequency - centroid;
        weightedVariance += deviation * deviation * power;
        totalEnergy += power;
    }
    
    // Return square root of weighted variance (standard deviation)
    return totalEnergy > 0.0 ? static_cast<float>(std::sqrt(weightedVariance / totalEnergy)) : 0.0f;
}

float SpectrumAnalyzer::extractSpectralRolloff(const std::vector<float>& powerSpectrum, double sampleRate)
{
    // Calculate total energy
    double totalEnergy = 0.0;
    for (size_t i = 1; i < powerSpectrum.size(); ++i) // Start from 1 to skip DC
    {
        totalEnergy += powerSpectrum[i];
    }
    
    if (totalEnergy <= 0.0)
        return 0.0f;
    
    // Find frequency below which 85% of energy is concentrated
    double energyThreshold = 0.85 * totalEnergy;
    double cumulativeEnergy = 0.0;
    
    for (size_t i = 1; i < powerSpectrum.size(); ++i) // Start from 1 to skip DC
    {
        cumulativeEnergy += powerSpectrum[i];
        
        if (cumulativeEnergy >= energyThreshold)
        {
            // Convert bin index to frequency - librosa uses: freq = i * sr / n_fft
            // For power spectrum size N, the FFT size is 2 * (N - 1)
            int fftSize = 2 * (powerSpectrum.size() - 1);
            double frequency = (i * sampleRate) / fftSize;
            return static_cast<float>(frequency);
        }
    }
    
    // If we reach here, return the Nyquist frequency
    return static_cast<float>(sampleRate / 2.0);
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
    
    // Frequency range for mel filters - using librosa defaults
    double minFreq = FMIN;         // 0.0 Hz
    double maxFreq = FMAX;         // 22050.0 Hz (or sampleRate/2 if smaller)
    maxFreq = std::min(maxFreq, sampleRate / 2.0);
    
    // Convert to mel scale
    float minMel = melScale(minFreq);
    float maxMel = melScale(maxFreq);
    
    // Create mel filter bank - librosa uses NUM_MEL_FILTERS + 2 points
    // to define NUM_MEL_FILTERS triangular filters
    for (int m = 0; m < NUM_MEL_FILTERS; ++m)
    {
        // Calculate mel frequencies for this filter
        // librosa creates n_mels + 2 points to define n_mels triangular filters
        float leftMel = minMel + (m * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        float centerMel = minMel + ((m + 1) * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        float rightMel = minMel + ((m + 2) * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        
        // Convert back to linear frequency
        float leftFreq = invMelScale(leftMel);
        float centerFreq = invMelScale(centerMel);
        float rightFreq = invMelScale(rightMel);
        
        // Convert frequencies to bin indices - librosa uses: bin = freq * n_fft / sr
        // For power spectrum size N, the FFT size is 2 * (N - 1)
        int fftSize = 2 * (powerSpectrum.size() - 1);
        int leftBin = static_cast<int>((leftFreq * fftSize) / sampleRate);
        int centerBin = static_cast<int>((centerFreq * fftSize) / sampleRate);
        int rightBin = static_cast<int>((rightFreq * fftSize) / sampleRate);
        
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
    
    // librosa uses DCT type-II with 'ortho' normalization
    // scipy.fftpack.dct(x, type=2, norm='ortho')
    for (int k = 0; k < NUM_MFCC_COEFFS; ++k)
    {
        double sum = 0.0;
        
        for (int n = 0; n < N; ++n)
        {
            double angle = juce::MathConstants<double>::pi * k * (n + 0.5) / N;
            sum += melEnergies[n] * std::cos(angle);
        }
        
        // Apply orthogonal normalization (librosa default)
        float norm_factor;
        if (k == 0)
        {
            norm_factor = std::sqrt(1.0f / N);  // First coefficient
        }
        else
        {
            norm_factor = std::sqrt(2.0f / N);  // Other coefficients
        }
        
        dctCoeffs[k] = norm_factor * sum;
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
    
    // Calculate Hann window sum for normalization (librosa compatibility)
    float windowSum = 0.0f;
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        const float windowValue = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (FFT_SIZE - 1)));
        windowSum += windowValue;
    }
    
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
    // JUCE's performFrequencyOnlyForwardTransform returns magnitude values
    // For librosa compatibility, we need magnitude squared (power) with proper normalization
    std::vector<float> powerSpectrum(FFT_SIZE / 2 + 1);  // +1 for DC and Nyquist
    
    // Normalization factor for Hann window: 2 / windowSum (factor of 2 for one-sided spectrum)
    const float normalizationFactor = 2.0f / windowSum;
    
    for (int i = 0; i < FFT_SIZE / 2 + 1; ++i)
    {
        if (i < FFT_SIZE / 2)
        {
            float magnitude = fftData[i] * normalizationFactor;
            powerSpectrum[i] = magnitude * magnitude;  // magnitude^2 = power
        }
        else
        {
            powerSpectrum[i] = 0.0f;  // Nyquist bin
        }
    }
    
    return powerSpectrum;
}

// VTR3 Feature storage and management methods
void SpectrumAnalyzer::extractAndStoreFeatures()
{
    // Extract features from current audio buffer in FIFO
    std::vector<float> audioData(inputFifo.begin(), inputFifo.end());
    
    // Extract features
    std::vector<float> features = extractFeatures(audioData, sampleRate);
    
    // Store features thread-safely
    {
        std::lock_guard<std::mutex> lock(spectrumMutex);
        latestFeatures = std::move(features);
        newFeaturesAvailable.store(true);
    }
}

std::vector<float> SpectrumAnalyzer::getLatestFeatures() const
{
    std::lock_guard<std::mutex> lock(spectrumMutex);
    return latestFeatures;
}

bool SpectrumAnalyzer::hasNewFeatures() const
{
    return newFeaturesAvailable.load();
}

void SpectrumAnalyzer::enableFeatureExtraction(bool enable)
{
    featureExtractionEnabled.store(enable);
    if (enable)
    {
        // Initialize features storage
        std::lock_guard<std::mutex> lock(spectrumMutex);
        latestFeatures.resize(TOTAL_FEATURES, 0.0f);
        newFeaturesAvailable.store(false);
        featureUpdateCounter = 0;
    }
}

void SpectrumAnalyzer::setFeatureUpdateRate(float rateHz)
{
    featureUpdateRateHz = rateHz;
    // Calculate update interval based on rate
    // Assuming FFT updates at UPDATE_RATE_HZ (30Hz)
    featureUpdateInterval = static_cast<int>(UPDATE_RATE_HZ / rateHz);
    featureUpdateInterval = std::max(1, featureUpdateInterval); // At least every FFT
}

void SpectrumAnalyzer::setFeatureExtractionBackend(FeatureExtractionBackend backend)
{
    currentBackend = backend;
    
    // Update feature extractor backend
    if (featureExtractor)
    {
        FeatureExtractor::Backend extractorBackend = FeatureExtractor::Backend::JUCE_BASED;
        
        if (backend == FeatureExtractionBackend::LIBXTRACT_BASED)
        {
            extractorBackend = FeatureExtractor::Backend::LIBXTRACT_BASED;
        }
        else if (backend == FeatureExtractionBackend::PYTHON_LIBROSA)
        {
            extractorBackend = FeatureExtractor::Backend::PYTHON_LIBROSA;
        }
        
        featureExtractor->setBackend(extractorBackend);
    }
    
#ifdef HAVE_ESSENTIA
    // Initialize Essentia extractor if switching to Essentia backend
    if (backend == FeatureExtractionBackend::ESSENTIA_BASED && !essentiaExtractor)
    {
        essentiaExtractor = std::make_unique<EssentiaFeatureExtractor>();
        essentiaExtractor->prepare(sampleRate);
    }
#else
    // Force JUCE backend if Essentia not available and not LibXtract
    if (backend == FeatureExtractionBackend::ESSENTIA_BASED)
    {
        currentBackend = FeatureExtractionBackend::JUCE_BASED;
    }
    juce::ignoreUnused(backend);
#endif
}