#pragma once

#include <vector>
#include <string>
#include <memory>

// Forward declaration to avoid Python.h in header
struct _object;
typedef _object PyObject;

class PythonFeatureExtractor
{
public:
    PythonFeatureExtractor();
    ~PythonFeatureExtractor();
    
    bool initialize();
    void cleanup();
    
    // Extract features using Python librosa (matches original VTR model)
    std::vector<float> extractFeatures(const std::vector<float>& audioData, double sampleRate = 44100.0);
    
    // Individual feature extraction (for compatibility)
    float extractSpectralCentroid(const std::vector<float>& audioData, double sampleRate = 44100.0);
    float extractSpectralBandwidth(const std::vector<float>& audioData, double sampleRate = 44100.0);
    float extractSpectralRolloff(const std::vector<float>& audioData, double sampleRate = 44100.0);
    std::vector<float> extractMFCC(const std::vector<float>& audioData, int numCoeffs = 13, double sampleRate = 44100.0);
    float extractRMSEnergy(const std::vector<float>& audioData, double sampleRate = 44100.0);
    
    bool isInitialized() const { return pythonInitialized_; }
    
private:
    bool initializePython();
    PyObject* convertAudioDataToPython(const std::vector<float>& audioData);
    std::vector<float> convertPythonToVector(PyObject* pyList);
    float convertPythonToFloat(PyObject* pyFloat);
    
    bool pythonInitialized_;
    PyObject* pModule_;           // Python module
    PyObject* pExtractFeatures_;  // extract_features function
    PyObject* pExtractCentroid_;  // extract_spectral_centroid function
    PyObject* pExtractBandwidth_; // extract_spectral_bandwidth function
    PyObject* pExtractRolloff_;   // extract_spectral_rolloff function
    PyObject* pExtractMFCC_;      // extract_mfcc function
    PyObject* pExtractRMS_;       // extract_rms function
    
    // Constants matching librosa defaults
    static constexpr int HOP_LENGTH = 512;
    static constexpr int N_FFT = 2048;
    static constexpr int N_MELS = 128;
    static constexpr int N_MFCC = 13;
};