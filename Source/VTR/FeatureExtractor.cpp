#include "FeatureExtractor.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <algorithm>
#include <cmath>
#include <iostream>

FeatureExtractor::FeatureExtractor()
    : sampleRate_(44100.0)
    , fftSize_(512)
    , isInitialized_(false)
    , currentBackend_(Backend::JUCE_BASED)
#ifdef HAVE_LIBXTRACT
    , window_(nullptr)
    , libxtractInitialized_(false)
#endif
    , pythonExtractor_(nullptr)
{
}

FeatureExtractor::~FeatureExtractor()
{
#ifdef HAVE_LIBXTRACT
    if (window_)
    {
        xtract_free_window(window_);
        window_ = nullptr;
    }
#endif
}

void FeatureExtractor::initialize(double sampleRate, int fftSize, Backend backend)
{
    sampleRate_ = sampleRate;
    fftSize_ = fftSize;
    currentBackend_ = backend;
    
    // Initialize FFT
    int fftOrder = static_cast<int>(std::log2(fftSize));
    fft_ = std::make_unique<juce::dsp::FFT>(fftOrder);
    
    // Initialize working buffers
    workBuffer_.resize(fftSize * 2);
    fftBuffer_.resize(fftSize);
    
#ifdef HAVE_LIBXTRACT
    if (backend == Backend::LIBXTRACT_BASED)
    {
        initializeLibXtract();
    }
#else
    if (backend == Backend::LIBXTRACT_BASED)
    {
        std::cerr << "LibXtract not available, using JUCE backend" << std::endl;
        currentBackend_ = Backend::JUCE_BASED;
    }
#endif
    
    // Initialize Python backend if requested
    if (backend == Backend::PYTHON_LIBROSA)
    {
        std::cout << "Initializing Python backend..." << std::endl;
        juce::Logger::writeToLog("FeatureExtractor: Initializing Python backend...");
        pythonExtractor_ = std::make_unique<PythonFeatureExtractor>();
        if (!pythonExtractor_->initialize())
        {
            std::cerr << "Python librosa backend initialization failed, falling back to JUCE" << std::endl;
            juce::Logger::writeToLog("FeatureExtractor: Python initialization FAILED - falling back to JUCE");
            pythonExtractor_.reset();
            currentBackend_ = Backend::JUCE_BASED;
        }
        else
        {
            std::cout << "✅ Python librosa backend initialized successfully!" << std::endl;
            juce::Logger::writeToLog("FeatureExtractor: Python backend initialized SUCCESSFULLY");
        }
    }
    
    isInitialized_ = true;
}

void FeatureExtractor::setBackend(Backend backend)
{
    if (backend != currentBackend_)
    {
        currentBackend_ = backend;
        
#ifdef HAVE_LIBXTRACT
        if (backend == Backend::LIBXTRACT_BASED && !libxtractInitialized_)
        {
            initializeLibXtract();
        }
#endif
    }
}

std::vector<float> FeatureExtractor::extractFeatures(const std::vector<float>& audioData)
{
    if (!isInitialized_)
    {
        std::cerr << "FeatureExtractor not initialized!" << std::endl;
        return std::vector<float>(FEATURE_VECTOR_SIZE, 0.0f);
    }
    
    // Use Python librosa backend if available (100% compatibility!)
    if (currentBackend_ == Backend::PYTHON_LIBROSA && pythonExtractor_)
    {
        std::cout << "DEBUG: Using Python librosa backend!" << std::endl;
        std::cout << "DEBUG: Audio data size: " << audioData.size() << std::endl;
        juce::Logger::writeToLog("FeatureExtractor: Using Python librosa backend!");
        auto result = pythonExtractor_->extractFeatures(audioData, sampleRate_);
        std::cout << "DEBUG: Python backend returned " << result.size() << " features" << std::endl;
        juce::Logger::writeToLog("FeatureExtractor: Python returned " + juce::String(result.size()) + " features");
        return result;
    }
    else
    {
        juce::Logger::writeToLog("FeatureExtractor: Using JUCE backend (Python not available)");
    }
    
    std::vector<float> features(FEATURE_VECTOR_SIZE);
    
    // Extract individual features based on backend
    // IMPORTANT: Order must match Python training data!
    // Training order: [spectral_centroid, spectral_bandwidth, spectral_rolloff, mfcc1-13, rms_energy]
    features[0] = extractSpectralCentroid(audioData);             // spectral_centroid
    features[1] = extractSpectralBandwidth(audioData);            // spectral_bandwidth
    features[2] = extractSpectralRolloff(audioData);              // spectral_rolloff
    
    // Extract MFCCs (indices 3-15)
    auto mfccs = extractMFCC(audioData, NUM_MFCC_COEFFS);
    for (int i = 0; i < NUM_MFCC_COEFFS && i < mfccs.size(); ++i)
    {
        features[3 + i] = mfccs[i];
    }
    
    features[16] = extractRMSEnergy(audioData);                   // rms_energy
    
    return features;
}

std::vector<float> FeatureExtractor::extractMFCC(const std::vector<float>& audioData, int numCoeffs)
{
    if (!isInitialized_)
    {
        return std::vector<float>(numCoeffs, 0.0f);
    }
    
#ifdef HAVE_LIBXTRACT
    if (currentBackend_ == Backend::LIBXTRACT_BASED && libxtractInitialized_)
    {
        return extractMFCC_LibXtract(audioData, numCoeffs);
    }
#endif
    
    // Default to JUCE implementation
    auto powerSpectrum = computePowerSpectrum(audioData);
    return extractMFCC_JUCE(powerSpectrum);
}

float FeatureExtractor::extractSpectralCentroid(const std::vector<float>& audioData)
{
    if (!isInitialized_)
    {
        return 0.0f;
    }
    
#ifdef HAVE_LIBXTRACT
    if (currentBackend_ == Backend::LIBXTRACT_BASED && libxtractInitialized_)
    {
        return extractSpectralCentroid_LibXtract(audioData);
    }
#endif
    
    // Default to JUCE implementation
    auto powerSpectrum = computePowerSpectrum(audioData);
    return extractSpectralCentroid_JUCE(powerSpectrum);
}

float FeatureExtractor::extractSpectralBandwidth(const std::vector<float>& audioData)
{
    if (!isInitialized_)
    {
        return 0.0f;
    }
    
#ifdef HAVE_LIBXTRACT
    if (currentBackend_ == Backend::LIBXTRACT_BASED && libxtractInitialized_)
    {
        return extractSpectralBandwidth_LibXtract(audioData);
    }
#endif
    
    // Default to JUCE implementation  
    auto powerSpectrum = computePowerSpectrum(audioData);
    return extractSpectralBandwidth_JUCE(powerSpectrum);
}

float FeatureExtractor::extractSpectralRolloff(const std::vector<float>& audioData, float rolloffPercent)
{
    if (!isInitialized_)
    {
        return 0.0f;
    }
    
#ifdef HAVE_LIBXTRACT
    if (currentBackend_ == Backend::LIBXTRACT_BASED && libxtractInitialized_)
    {
        return extractSpectralRolloff_LibXtract(audioData, rolloffPercent);
    }
#endif
    
    // Default to JUCE implementation
    auto powerSpectrum = computePowerSpectrum(audioData);
    return extractSpectralRolloff_JUCE(powerSpectrum, rolloffPercent);
}

float FeatureExtractor::extractRMSEnergy(const std::vector<float>& audioData)
{
    if (audioData.empty())
    {
        return 0.0f;
    }
    
    double sumSquares = 0.0;
    for (float sample : audioData)
    {
        sumSquares += sample * sample;
    }
    
    return std::sqrt(sumSquares / audioData.size());
}

std::vector<float> FeatureExtractor::loadAudioFile(const std::string& filePath, double targetSampleRate)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    juce::File file(filePath);
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (!reader)
    {
        std::cerr << "Failed to load audio file: " << filePath << std::endl;
        return {};
    }
    
    // Safety check for file size
    if (reader->lengthInSamples > 44100 * 60)  // More than 60 seconds at 44.1kHz
    {
        std::cerr << "ERROR: Audio file too large: " << reader->lengthInSamples << " samples (" 
                  << (reader->lengthInSamples / reader->sampleRate) << " seconds). Skipping." << std::endl;
        return {};
    }
    
    if (reader->lengthInSamples <= 0)
    {
        std::cerr << "ERROR: Audio file has no samples: " << filePath << std::endl;
        return {};
    }
    
    // Read audio data
    juce::AudioBuffer<float> buffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
    
    // Convert to mono and resample if needed
    std::vector<float> audioData;
    int numSamples = buffer.getNumSamples();
    audioData.reserve(numSamples);
    
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            sample += buffer.getSample(ch, i);
        }
        audioData.push_back(sample / buffer.getNumChannels());
    }
    
    // Simple resampling if needed (basic implementation)
    if (std::abs(reader->sampleRate - targetSampleRate) > 1.0)
    {
        double ratio = targetSampleRate / reader->sampleRate;
        std::vector<float> resampledData;
        resampledData.reserve(static_cast<size_t>(audioData.size() * ratio));
        
        for (size_t i = 0; i < audioData.size(); ++i)
        {
            double originalIndex = i / ratio;
            size_t index = static_cast<size_t>(originalIndex);
            if (index < audioData.size() - 1)
            {
                double fraction = originalIndex - index;
                float interpolated = audioData[index] * (1.0f - fraction) + audioData[index + 1] * fraction;
                resampledData.push_back(interpolated);
            }
        }
        
        return resampledData;
    }
    
    return audioData;
}

// Private implementation methods

std::vector<float> FeatureExtractor::computePowerSpectrum(const std::vector<float>& audioData)
{
    // Ensure we have enough data
    std::vector<float> paddedData = audioData;
    paddedData.resize(fftSize_, 0.0f);
    
    // Apply Hann window
    applyHannWindow(paddedData);
    
    // Prepare FFT data (interleaved real/imaginary)
    std::fill(workBuffer_.begin(), workBuffer_.end(), 0.0f);
    for (int i = 0; i < fftSize_; ++i)
    {
        workBuffer_[i * 2] = paddedData[i];     // Real part
        workBuffer_[i * 2 + 1] = 0.0f;          // Imaginary part
    }
    
    // Perform FFT
    fft_->performFrequencyOnlyForwardTransform(workBuffer_.data());
    
    // Compute power spectrum
    std::vector<float> powerSpectrum(fftSize_ / 2 + 1);
    for (int i = 0; i < powerSpectrum.size(); ++i)
    {
        float real = workBuffer_[i * 2];
        float imag = workBuffer_[i * 2 + 1];
        powerSpectrum[i] = real * real + imag * imag;
    }
    
    return powerSpectrum;
}

void FeatureExtractor::applyHannWindow(std::vector<float>& data)
{
    for (int i = 0; i < data.size(); ++i)
    {
        float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (data.size() - 1)));
        data[i] *= window;
    }
}

// JUCE-based implementations
std::vector<float> FeatureExtractor::extractMFCC_JUCE(const std::vector<float>& powerSpectrum)
{
    auto melEnergies = computeMelFilterbank_JUCE(powerSpectrum);
    
    // Apply log to mel energies
    for (auto& energy : melEnergies)
    {
        energy = std::log(std::max(energy, 1e-10f));
    }
    
    return computeDCT_JUCE(melEnergies);
}

float FeatureExtractor::extractSpectralCentroid_JUCE(const std::vector<float>& powerSpectrum)
{
    double weightedSum = 0.0;
    double totalEnergy = 0.0;
    
    for (size_t i = 1; i < powerSpectrum.size(); ++i)
    {
        double frequency = (i * sampleRate_) / (2 * (powerSpectrum.size() - 1));
        double power = powerSpectrum[i];
        weightedSum += frequency * power;
        totalEnergy += power;
    }
    
    return totalEnergy > 0.0 ? static_cast<float>(weightedSum / totalEnergy) : 0.0f;
}

float FeatureExtractor::extractSpectralBandwidth_JUCE(const std::vector<float>& powerSpectrum)
{
    double centroid = extractSpectralCentroid_JUCE(powerSpectrum);
    double weightedVariance = 0.0;
    double totalEnergy = 0.0;
    
    for (size_t i = 1; i < powerSpectrum.size(); ++i)
    {
        double frequency = (i * sampleRate_) / (2 * (powerSpectrum.size() - 1));
        double power = powerSpectrum[i];
        double deviation = frequency - centroid;
        weightedVariance += deviation * deviation * power;
        totalEnergy += power;
    }
    
    return totalEnergy > 0.0 ? static_cast<float>(std::sqrt(weightedVariance / totalEnergy)) : 0.0f;
}

float FeatureExtractor::extractSpectralRolloff_JUCE(const std::vector<float>& powerSpectrum, float rolloffPercent)
{
    double totalEnergy = 0.0;
    for (size_t i = 1; i < powerSpectrum.size(); ++i)
    {
        totalEnergy += powerSpectrum[i];
    }
    
    double targetEnergy = totalEnergy * rolloffPercent;
    double cumulativeEnergy = 0.0;
    
    for (size_t i = 1; i < powerSpectrum.size(); ++i)
    {
        cumulativeEnergy += powerSpectrum[i];
        if (cumulativeEnergy >= targetEnergy)
        {
            double frequency = (i * sampleRate_) / (2 * (powerSpectrum.size() - 1));
            return static_cast<float>(frequency);
        }
    }
    
    return static_cast<float>(sampleRate_ / 2.0);
}

std::vector<float> FeatureExtractor::computeMelFilterbank_JUCE(const std::vector<float>& powerSpectrum)
{
    std::vector<float> melEnergies(NUM_MEL_FILTERS, 0.0f);
    
    float minFreq = 0.0f;
    float maxFreq = sampleRate_ / 2.0f;
    float minMel = melScale(minFreq);
    float maxMel = melScale(maxFreq);
    
    for (int m = 0; m < NUM_MEL_FILTERS; ++m)
    {
        float leftMel = minMel + (m * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        float centerMel = minMel + ((m + 1) * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        float rightMel = minMel + ((m + 2) * (maxMel - minMel)) / (NUM_MEL_FILTERS + 1);
        
        float leftFreq = invMelScale(leftMel);
        float centerFreq = invMelScale(centerMel);
        float rightFreq = invMelScale(rightMel);
        
        int fftSize = 2 * (powerSpectrum.size() - 1);
        int leftBin = static_cast<int>((leftFreq * fftSize) / sampleRate_);
        int centerBin = static_cast<int>((centerFreq * fftSize) / sampleRate_);
        int rightBin = static_cast<int>((rightFreq * fftSize) / sampleRate_);
        
        leftBin = std::max(0, std::min(leftBin, static_cast<int>(powerSpectrum.size()) - 1));
        centerBin = std::max(0, std::min(centerBin, static_cast<int>(powerSpectrum.size()) - 1));
        rightBin = std::max(0, std::min(rightBin, static_cast<int>(powerSpectrum.size()) - 1));
        
        for (int i = leftBin; i <= rightBin; ++i)
        {
            float weight = 0.0f;
            
            if (i <= centerBin && centerBin > leftBin)
            {
                weight = static_cast<float>(i - leftBin) / (centerBin - leftBin);
            }
            else if (i > centerBin && rightBin > centerBin)
            {
                weight = static_cast<float>(rightBin - i) / (rightBin - centerBin);
            }
            
            melEnergies[m] += weight * powerSpectrum[i];
        }
    }
    
    return melEnergies;
}

std::vector<float> FeatureExtractor::computeDCT_JUCE(const std::vector<float>& melEnergies)
{
    std::vector<float> dctCoeffs(NUM_MFCC_COEFFS, 0.0f);
    
    const int N = melEnergies.size();
    
    for (int k = 0; k < NUM_MFCC_COEFFS; ++k)
    {
        double sum = 0.0;
        
        for (int n = 0; n < N; ++n)
        {
            double angle = juce::MathConstants<double>::pi * k * (n + 0.5) / N;
            sum += melEnergies[n] * std::cos(angle);
        }
        
        float normFactor = (k == 0) ? std::sqrt(1.0f / N) : std::sqrt(2.0f / N);
        dctCoeffs[k] = normFactor * sum;
    }
    
    return dctCoeffs;
}

float FeatureExtractor::melScale(float frequency)
{
    return 2595.0f * std::log10(1.0f + frequency / 700.0f);
}

float FeatureExtractor::invMelScale(float mel)
{
    return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
}

#ifdef HAVE_LIBXTRACT
// LibXtract initialization and implementations

void FeatureExtractor::initializeLibXtract()
{
    // Initialize working buffers
    int spectrumSize = fftSize_ / 2 + 1;
    workBufferDouble_.resize(spectrumSize);
    melEnergies_.resize(NUM_MEL_FILTERS);
    allMFCCCoeffs_.resize(NUM_MEL_FILTERS);
    
    // Create Hann window
    window_ = xtract_init_window(fftSize_, XTRACT_HANN);
    
    // Initialize mel filter bank
    melFilters_.resize(NUM_MEL_FILTERS);
    melFilterPtrs_.resize(NUM_MEL_FILTERS);
    
    for (int i = 0; i < NUM_MEL_FILTERS; ++i)
    {
        melFilters_[i].resize(spectrumSize, 0.0);
        melFilterPtrs_[i] = melFilters_[i].data();
    }
    
    // Initialize mel filter bank using LibXtract
    double freqMin = 20.0;
    double freqMax = sampleRate_ / 2.0;
    double nyquist = sampleRate_ / 2.0;
    
    int result = xtract_init_mfcc(spectrumSize, nyquist, XTRACT_EQUAL_GAIN, 
                                  freqMin, freqMax, NUM_MEL_FILTERS, 
                                  melFilterPtrs_.data());
    
    if (result == XTRACT_SUCCESS)
    {
        libxtractInitialized_ = true;
        std::cout << "LibXtract MFCC initialized successfully" << std::endl;
    }
    else
    {
        std::cerr << "Failed to initialize LibXtract MFCC. Error code: " << result << std::endl;
        libxtractInitialized_ = false;
    }
}

std::vector<float> FeatureExtractor::extractMFCC_LibXtract(const std::vector<float>& audioData, int numCoeffs)
{
    std::vector<float> result(numCoeffs, 0.0f);
    
    if (!libxtractInitialized_ || audioData.size() < fftSize_)
    {
        return result;
    }
    
    // Prepare audio data for LibXtract (needs double)
    std::vector<double> audioDouble(fftSize_);
    for (int i = 0; i < fftSize_ && i < audioData.size(); ++i)
    {
        audioDouble[i] = static_cast<double>(audioData[i]);
    }
    
    // Apply windowing
    std::vector<double> windowedData(fftSize_);
    xtract_windowed(audioDouble.data(), fftSize_, window_, windowedData.data());
    
    // Compute spectrum
    std::vector<double> spectrum(fftSize_);
    double argd[4] = {0};
    argd[0] = sampleRate_ / static_cast<double>(fftSize_);
    argd[1] = XTRACT_MAGNITUDE_SPECTRUM;
    argd[2] = 0.0;
    argd[3] = 0.0;
    
    xtract_init_fft(fftSize_, XTRACT_SPECTRUM);
    xtract[XTRACT_SPECTRUM](windowedData.data(), fftSize_, argd, spectrum.data());
    
    // Extract MFCC
    xtract_mel_filter melFilterData;
    melFilterData.n_filters = NUM_MEL_FILTERS;
    melFilterData.filters = melFilterPtrs_.data();
    
    int halfSize = fftSize_ / 2;
    int mfccResult = xtract_mfcc(spectrum.data(), halfSize, &melFilterData, allMFCCCoeffs_.data());
    
    xtract_free_fft();
    
    if (mfccResult == XTRACT_SUCCESS)
    {
        for (int i = 0; i < numCoeffs && i < allMFCCCoeffs_.size(); ++i)
        {
            result[i] = static_cast<float>(allMFCCCoeffs_[i]);
        }
    }
    
    return result;
}

float FeatureExtractor::extractSpectralCentroid_LibXtract(const std::vector<float>& audioData)
{
    if (!libxtractInitialized_ || audioData.size() < fftSize_)
    {
        return 0.0f;
    }
    
    // Prepare audio data for LibXtract
    std::vector<double> audioDouble(fftSize_);
    for (int i = 0; i < fftSize_ && i < audioData.size(); ++i)
    {
        audioDouble[i] = static_cast<double>(audioData[i]);
    }
    
    // Apply windowing and compute spectrum
    std::vector<double> windowedData(fftSize_);
    std::vector<double> spectrum(fftSize_);
    
    xtract_windowed(audioDouble.data(), fftSize_, window_, windowedData.data());
    
    // First compute the complex spectrum
    std::vector<double> complexSpectrum(fftSize_);
    double argd[4] = {0};
    argd[0] = sampleRate_ / static_cast<double>(fftSize_);
    argd[1] = XTRACT_MAGNITUDE_SPECTRUM;
    
    xtract_init_fft(fftSize_, XTRACT_SPECTRUM);
    xtract[XTRACT_SPECTRUM](windowedData.data(), fftSize_, argd, complexSpectrum.data());
    
    // Now compute magnitude spectrum for spectral features
    xtract[XTRACT_MAGNITUDE_SPECTRUM](complexSpectrum.data(), fftSize_ / 2, argd, spectrum.data());
    
    // Extract spectral centroid
    double centroid = 0.0;
    argd[0] = sampleRate_ / static_cast<double>(fftSize_);  // Set frequency resolution
    
    // Debug: Check if spectrum has values
    double spectrumSum = 0.0;
    for (int i = 0; i < fftSize_ / 2; i++) {
        spectrumSum += spectrum[i];
    }
    
    int result = xtract_spectral_centroid(spectrum.data(), fftSize_ / 2, argd, &centroid);
    
    // Debug output
    if (spectrumSum == 0.0) {
        std::cerr << "WARNING: Spectrum is all zeros!" << std::endl;
    }
    if (result != XTRACT_SUCCESS) {
        std::cerr << "LibXtract spectral_centroid failed with code: " << result << std::endl;
    }
    
    xtract_free_fft();
    
    return (result == XTRACT_SUCCESS) ? static_cast<float>(centroid) : 0.0f;
}

float FeatureExtractor::extractSpectralBandwidth_LibXtract(const std::vector<float>& audioData)
{
    if (!libxtractInitialized_ || audioData.size() < fftSize_)
    {
        return 0.0f;
    }
    
    // First get the centroid
    float centroid = extractSpectralCentroid_LibXtract(audioData);
    
    // Prepare data
    std::vector<double> audioDouble(fftSize_);
    for (int i = 0; i < fftSize_ && i < audioData.size(); ++i)
    {
        audioDouble[i] = static_cast<double>(audioData[i]);
    }
    
    std::vector<double> windowedData(fftSize_);
    std::vector<double> spectrum(fftSize_);
    
    xtract_windowed(audioDouble.data(), fftSize_, window_, windowedData.data());
    
    // First compute the complex spectrum
    std::vector<double> complexSpectrum(fftSize_);
    double argd[4] = {0};
    argd[0] = sampleRate_ / static_cast<double>(fftSize_);
    argd[1] = XTRACT_MAGNITUDE_SPECTRUM;
    
    xtract_init_fft(fftSize_, XTRACT_SPECTRUM);
    xtract[XTRACT_SPECTRUM](windowedData.data(), fftSize_, argd, complexSpectrum.data());
    
    // Now compute magnitude spectrum for spectral features
    xtract[XTRACT_MAGNITUDE_SPECTRUM](complexSpectrum.data(), fftSize_ / 2, argd, spectrum.data());
    
    // Extract spectral variance (used to compute bandwidth)
    double variance = 0.0;
    double centroidArg = static_cast<double>(centroid);
    int result = xtract_spectral_variance(spectrum.data(), fftSize_ / 2, &centroidArg, &variance);
    
    xtract_free_fft();
    
    // Bandwidth is the square root of variance
    return (result == XTRACT_SUCCESS) ? static_cast<float>(std::sqrt(variance)) : 0.0f;
}

float FeatureExtractor::extractSpectralRolloff_LibXtract(const std::vector<float>& audioData, float rolloffPercent)
{
    // LibXtract doesn't have a direct rolloff function, so compute it manually
    // using the spectrum data
    if (!libxtractInitialized_ || audioData.size() < fftSize_)
    {
        return 0.0f;
    }
    
    // Prepare data and compute spectrum
    std::vector<double> audioDouble(fftSize_);
    for (int i = 0; i < fftSize_ && i < audioData.size(); ++i)
    {
        audioDouble[i] = static_cast<double>(audioData[i]);
    }
    
    std::vector<double> windowedData(fftSize_);
    std::vector<double> spectrum(fftSize_);
    
    xtract_windowed(audioDouble.data(), fftSize_, window_, windowedData.data());
    
    // First compute the complex spectrum
    std::vector<double> complexSpectrum(fftSize_);
    double argd[4] = {0};
    argd[0] = sampleRate_ / static_cast<double>(fftSize_);
    argd[1] = XTRACT_MAGNITUDE_SPECTRUM;
    
    xtract_init_fft(fftSize_, XTRACT_SPECTRUM);
    xtract[XTRACT_SPECTRUM](windowedData.data(), fftSize_, argd, complexSpectrum.data());
    
    // Now compute magnitude spectrum for spectral features
    xtract[XTRACT_MAGNITUDE_SPECTRUM](complexSpectrum.data(), fftSize_ / 2, argd, spectrum.data());
    
    // Convert magnitude to power and compute rolloff manually
    int halfSize = fftSize_ / 2;
    double totalEnergy = 0.0;
    
    for (int i = 1; i < halfSize; ++i)
    {
        double power = spectrum[i] * spectrum[i];
        totalEnergy += power;
    }
    
    double targetEnergy = totalEnergy * rolloffPercent;
    double cumulativeEnergy = 0.0;
    
    for (int i = 1; i < halfSize; ++i)
    {
        double power = spectrum[i] * spectrum[i];
        cumulativeEnergy += power;
        
        if (cumulativeEnergy >= targetEnergy)
        {
            double frequency = (i * sampleRate_) / fftSize_;
            xtract_free_fft();
            return static_cast<float>(frequency);
        }
    }
    
    xtract_free_fft();
    return static_cast<float>(sampleRate_ / 2.0);
}

#endif