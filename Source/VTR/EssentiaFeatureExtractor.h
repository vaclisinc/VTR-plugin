#pragma once

#include <vector>
#include <memory>
#include <essentia/essentia.h>
#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>

class EssentiaFeatureExtractor
{
public:
    EssentiaFeatureExtractor();
    ~EssentiaFeatureExtractor();
    
    void prepare(double sampleRate);
    std::vector<float> extractFeatures(const std::vector<float>& audioData, double sampleRate);
    
    static constexpr int TOTAL_FEATURES = 17;
    static constexpr int NUM_MFCC_COEFFS = 13;
    static constexpr int NUM_MEL_FILTERS = 128;
    static constexpr double FMIN = 0.0;
    static constexpr double FMAX = 22050.0;
    
private:
    void initializeAlgorithms();
    void cleanupAlgorithms();
    
    std::vector<float> extractMFCC(const std::vector<float>& audioData, double sampleRate);
    float extractSpectralCentroid(const std::vector<float>& audioData, double sampleRate);
    float extractSpectralBandwidth(const std::vector<float>& audioData, double sampleRate);
    float extractSpectralRolloff(const std::vector<float>& audioData, double sampleRate);
    float extractRMSEnergy(const std::vector<float>& audioData);
    
    // Essentia algorithms
    essentia::standard::Algorithm* frameCutter;
    essentia::standard::Algorithm* windowing;
    essentia::standard::Algorithm* spectrum;
    essentia::standard::Algorithm* mfcc;
    essentia::standard::Algorithm* spectralCentroid;
    essentia::standard::Algorithm* spectralBandwidth;
    essentia::standard::Algorithm* spectralRolloff;
    essentia::standard::Algorithm* rmsEnergy;
    
    double currentSampleRate;
    bool algorithmsInitialized;
    
    // Window settings optimized for real-time processing
    static constexpr int FRAME_SIZE = 2048;
    static constexpr int HOP_SIZE = 512;
    static constexpr std::string_view WINDOW_TYPE = "hann";
    
    // Frame buffering for real-time processing
    std::vector<float> frameBuffer;
    int bufferPos;
    bool bufferReady;
};