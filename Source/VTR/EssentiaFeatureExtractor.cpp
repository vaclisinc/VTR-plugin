#include "EssentiaFeatureExtractor.h"
#include <algorithm>
#include <cmath>
#include <numeric>

EssentiaFeatureExtractor::EssentiaFeatureExtractor()
    : frameCutter(nullptr)
    , windowing(nullptr)
    , spectrum(nullptr)
    , mfcc(nullptr)
    , spectralCentroid(nullptr)
    , spectralBandwidth(nullptr)
    , spectralRolloff(nullptr)
    , rmsEnergy(nullptr)
    , currentSampleRate(44100.0)
    , algorithmsInitialized(false)
    , bufferPos(0)
    , bufferReady(false)
{
    essentia::init();
}

EssentiaFeatureExtractor::~EssentiaFeatureExtractor()
{
    cleanupAlgorithms();
    essentia::shutdown();
}

void EssentiaFeatureExtractor::prepare(double sampleRate)
{
    if (sampleRate != currentSampleRate)
    {
        currentSampleRate = sampleRate;
        cleanupAlgorithms();
        initializeAlgorithms();
    }
    
    // Initialize frame buffer for real-time processing
    frameBuffer.resize(FRAME_SIZE, 0.0f);
    bufferPos = 0;
    bufferReady = false;
}

void EssentiaFeatureExtractor::initializeAlgorithms()
{
    if (algorithmsInitialized)
        return;
    
    essentia::standard::AlgorithmFactory& factory = essentia::standard::AlgorithmFactory::instance();
    
    // Initialize frame cutter
    frameCutter = factory.create("FrameCutter",
                                "frameSize", FRAME_SIZE,
                                "hopSize", HOP_SIZE,
                                "startFromZero", true);
    
    // Initialize windowing
    windowing = factory.create("Windowing",
                              "type", std::string(WINDOW_TYPE));
    
    // Initialize spectrum
    spectrum = factory.create("Spectrum");
    
    // Initialize MFCC
    mfcc = factory.create("MFCC",
                         "sampleRate", currentSampleRate,
                         "numberBands", NUM_MEL_FILTERS,
                         "numberCoefficients", NUM_MFCC_COEFFS,
                         "lowFrequencyBound", FMIN,
                         "highFrequencyBound", std::min(FMAX, currentSampleRate / 2.0));
    
    // Initialize spectral features
    spectralCentroid = factory.create("SpectralCentroid",
                                     "sampleRate", currentSampleRate);
    
    spectralBandwidth = factory.create("SpectralBandwidth",
                                      "sampleRate", currentSampleRate);
    
    spectralRolloff = factory.create("SpectralRolloff",
                                    "sampleRate", currentSampleRate);
    
    // Initialize RMS energy
    rmsEnergy = factory.create("RMS");
    
    algorithmsInitialized = true;
}

void EssentiaFeatureExtractor::cleanupAlgorithms()
{
    if (!algorithmsInitialized)
        return;
    
    delete frameCutter;
    delete windowing;
    delete spectrum;
    delete mfcc;
    delete spectralCentroid;
    delete spectralBandwidth;
    delete spectralRolloff;
    delete rmsEnergy;
    
    frameCutter = nullptr;
    windowing = nullptr;
    spectrum = nullptr;
    mfcc = nullptr;
    spectralCentroid = nullptr;
    spectralBandwidth = nullptr;
    spectralRolloff = nullptr;
    rmsEnergy = nullptr;
    
    algorithmsInitialized = false;
}

std::vector<float> EssentiaFeatureExtractor::extractFeatures(const std::vector<float>& audioData, double sampleRate)
{
    if (sampleRate != currentSampleRate)
    {
        prepare(sampleRate);
    }
    
    if (!algorithmsInitialized)
    {
        initializeAlgorithms();
    }
    
    std::vector<float> features;
    features.reserve(TOTAL_FEATURES);
    
    // Extract RMS energy
    float rms = extractRMSEnergy(audioData);
    features.push_back(rms);
    
    // Extract spectral features
    float centroid = extractSpectralCentroid(audioData, sampleRate);
    features.push_back(centroid);
    
    float bandwidth = extractSpectralBandwidth(audioData, sampleRate);
    features.push_back(bandwidth);
    
    float rolloff = extractSpectralRolloff(audioData, sampleRate);
    features.push_back(rolloff);
    
    // Extract MFCC coefficients
    std::vector<float> mfccCoeffs = extractMFCC(audioData, sampleRate);
    features.insert(features.end(), mfccCoeffs.begin(), mfccCoeffs.end());
    
    // Ensure we have exactly TOTAL_FEATURES
    features.resize(TOTAL_FEATURES, 0.0f);
    
    return features;
}

std::vector<float> EssentiaFeatureExtractor::extractMFCC(const std::vector<float>& audioData, double sampleRate)
{
    std::vector<float> mfccCoeffs;
    
    // Process audio in frames
    std::vector<float> frame;
    std::vector<float> windowedFrame;
    std::vector<float> spectrumData;
    std::vector<float> mfccFrame;
    std::vector<float> mfccBands;
    
    frameCutter->input("signal").set(audioData);
    frameCutter->output("frame").set(frame);
    
    std::vector<std::vector<float>> allMfccFrames;
    
    while (true)
    {
        frameCutter->compute();
        
        if (frame.empty())
            break;
        
        // Apply windowing
        windowing->input("frame").set(frame);
        windowing->output("frame").set(windowedFrame);
        windowing->compute();
        
        // Compute spectrum
        spectrum->input("frame").set(windowedFrame);
        spectrum->output("spectrum").set(spectrumData);
        spectrum->compute();
        
        // Compute MFCC
        mfcc->input("spectrum").set(spectrumData);
        mfcc->output("mfcc").set(mfccFrame);
        mfcc->output("bands").set(mfccBands);
        mfcc->compute();
        
        allMfccFrames.push_back(mfccFrame);
    }
    
    // Average MFCC coefficients across frames
    if (!allMfccFrames.empty())
    {
        mfccCoeffs.resize(NUM_MFCC_COEFFS, 0.0f);
        
        for (const auto& mfccFrame : allMfccFrames)
        {
            for (size_t i = 0; i < std::min(mfccFrame.size(), static_cast<size_t>(NUM_MFCC_COEFFS)); ++i)
            {
                mfccCoeffs[i] += mfccFrame[i];
            }
        }
        
        // Normalize by number of frames
        for (auto& coeff : mfccCoeffs)
        {
            coeff /= static_cast<float>(allMfccFrames.size());
        }
    }
    else
    {
        mfccCoeffs.resize(NUM_MFCC_COEFFS, 0.0f);
    }
    
    return mfccCoeffs;
}

float EssentiaFeatureExtractor::extractSpectralCentroid(const std::vector<float>& audioData, double sampleRate)
{
    std::vector<float> frame;
    std::vector<float> windowedFrame;
    std::vector<float> spectrumData;
    float centroid = 0.0f;
    
    frameCutter->input("signal").set(audioData);
    frameCutter->output("frame").set(frame);
    
    std::vector<float> centroids;
    
    while (true)
    {
        frameCutter->compute();
        
        if (frame.empty())
            break;
        
        windowing->input("frame").set(frame);
        windowing->output("frame").set(windowedFrame);
        windowing->compute();
        
        spectrum->input("frame").set(windowedFrame);
        spectrum->output("spectrum").set(spectrumData);
        spectrum->compute();
        
        spectralCentroid->input("spectrum").set(spectrumData);
        spectralCentroid->output("centroid").set(centroid);
        spectralCentroid->compute();
        
        centroids.push_back(centroid);
    }
    
    if (!centroids.empty())
    {
        return std::accumulate(centroids.begin(), centroids.end(), 0.0f) / centroids.size();
    }
    
    return 0.0f;
}

float EssentiaFeatureExtractor::extractSpectralBandwidth(const std::vector<float>& audioData, double sampleRate)
{
    std::vector<float> frame;
    std::vector<float> windowedFrame;
    std::vector<float> spectrumData;
    float bandwidth = 0.0f;
    
    frameCutter->input("signal").set(audioData);
    frameCutter->output("frame").set(frame);
    
    std::vector<float> bandwidths;
    
    while (true)
    {
        frameCutter->compute();
        
        if (frame.empty())
            break;
        
        windowing->input("frame").set(frame);
        windowing->output("frame").set(windowedFrame);
        windowing->compute();
        
        spectrum->input("frame").set(windowedFrame);
        spectrum->output("spectrum").set(spectrumData);
        spectrum->compute();
        
        spectralBandwidth->input("spectrum").set(spectrumData);
        spectralBandwidth->output("bandwidth").set(bandwidth);
        spectralBandwidth->compute();
        
        bandwidths.push_back(bandwidth);
    }
    
    if (!bandwidths.empty())
    {
        return std::accumulate(bandwidths.begin(), bandwidths.end(), 0.0f) / bandwidths.size();
    }
    
    return 0.0f;
}

float EssentiaFeatureExtractor::extractSpectralRolloff(const std::vector<float>& audioData, double sampleRate)
{
    std::vector<float> frame;
    std::vector<float> windowedFrame;
    std::vector<float> spectrumData;
    float rolloff = 0.0f;
    
    frameCutter->input("signal").set(audioData);
    frameCutter->output("frame").set(frame);
    
    std::vector<float> rolloffs;
    
    while (true)
    {
        frameCutter->compute();
        
        if (frame.empty())
            break;
        
        windowing->input("frame").set(frame);
        windowing->output("frame").set(windowedFrame);
        windowing->compute();
        
        spectrum->input("frame").set(windowedFrame);
        spectrum->output("spectrum").set(spectrumData);
        spectrum->compute();
        
        spectralRolloff->input("spectrum").set(spectrumData);
        spectralRolloff->output("rolloff").set(rolloff);
        spectralRolloff->compute();
        
        rolloffs.push_back(rolloff);
    }
    
    if (!rolloffs.empty())
    {
        return std::accumulate(rolloffs.begin(), rolloffs.end(), 0.0f) / rolloffs.size();
    }
    
    return 0.0f;
}

float EssentiaFeatureExtractor::extractRMSEnergy(const std::vector<float>& audioData)
{
    float rms = 0.0f;
    
    rmsEnergy->input("array").set(audioData);
    rmsEnergy->output("rms").set(rms);
    rmsEnergy->compute();
    
    return rms;
}