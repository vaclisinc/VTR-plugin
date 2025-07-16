#include "PythonFeatureExtractor.h"
#include <Python.h>
#include <iostream>
#include <sstream>

PythonFeatureExtractor::PythonFeatureExtractor()
    : pythonInitialized_(false)
    , pModule_(nullptr)
    , pExtractFeatures_(nullptr)
    , pExtractCentroid_(nullptr)
    , pExtractBandwidth_(nullptr)
    , pExtractRolloff_(nullptr)
    , pExtractMFCC_(nullptr)
    , pExtractRMS_(nullptr)
{
}

PythonFeatureExtractor::~PythonFeatureExtractor()
{
    cleanup();
}

bool PythonFeatureExtractor::initialize()
{
    if (pythonInitialized_)
    {
        return true;
    }
    
    return initializePython();
}

bool PythonFeatureExtractor::initializePython()
{
    std::cout << "PythonFeatureExtractor: Starting Python initialization..." << std::endl;
    
    // Initialize Python interpreter with thread support
    if (!Py_IsInitialized())
    {
        std::cout << "PythonFeatureExtractor: Initializing Python interpreter..." << std::endl;
        
        // Enable thread support for Python
        PyEval_InitThreads();
        Py_Initialize();
        
        if (!Py_IsInitialized())
        {
            std::cerr << "Failed to initialize Python interpreter" << std::endl;
            return false;
        }
        
        // Release the GIL so other threads can use Python
        PyEval_SaveThread();
    }
    
    std::cout << "PythonFeatureExtractor: Python interpreter initialized" << std::endl;
    
    // Acquire GIL for this thread before executing Python code
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    // Add current directory to Python path to find our script
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("print(f'Python executable: {sys.executable}', file=sys.stderr)");
    PyRun_SimpleString("print(f'Python version: {sys.version}', file=sys.stderr)");
    PyRun_SimpleString("print(f'Python path: {sys.path}', file=sys.stderr)");
    PyRun_SimpleString("sys.stderr.flush()");
    PyRun_SimpleString("sys.path.append('.')");
    PyRun_SimpleString("sys.path.append('./vtr-model')");
    
    std::cout << "PythonFeatureExtractor: About to execute Python script..." << std::endl;
    
    // Create embedded Python script for feature extraction
    const char* pythonScript = R"(
import sys
import numpy as np
import librosa

def extract_features_vector(audio_data, sr=44100):
    """Extract feature vector matching original VTR model"""
    try:
        y = np.array(audio_data, dtype=np.float32)
        
        # Safety check for audio data size
        if len(y) > 44100 * 60:  # More than 60 seconds at 44.1kHz
            sys.stderr.write(f"ERROR: Audio data too large: {len(y)} samples. Skipping.\n")
            sys.stderr.flush()
            return [0.0] * 17
        
        if len(y) == 0:
            sys.stderr.write("ERROR: Empty audio data\n")
            sys.stderr.flush()
            return [0.0] * 17
        
        # Extract features using librosa (exactly matching extract_features.py)
        
        # Spectral features
        spectral_centroid = np.mean(librosa.feature.spectral_centroid(y=y, sr=sr))
        spectral_bandwidth = np.mean(librosa.feature.spectral_bandwidth(y=y, sr=sr))
        spectral_rolloff = np.mean(librosa.feature.spectral_rolloff(y=y, sr=sr))
        
        # MFCC features
        mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=13)
        mfcc_means = [np.mean(mfccs[i]) for i in range(13)]
        
        # RMS energy
        rms_energy = np.mean(librosa.feature.rms(y=y))
        
        # Return in the order expected by VTR model: spectral_centroid, spectral_bandwidth,
        # spectral_rolloff, mfcc_1...mfcc_13, rms_energy (total 17 features)
        features = [spectral_centroid, spectral_bandwidth, spectral_rolloff] + mfcc_means + [rms_energy]
        return features
        
    except Exception as e:
        sys.stderr.write(f"ERROR in extract_features_vector: {e}\n")
        sys.stderr.flush()
        return [0.0] * 17

def extract_spectral_centroid(audio_data, sr=44100):
    y = np.array(audio_data, dtype=np.float32)
    return float(np.mean(librosa.feature.spectral_centroid(y=y, sr=sr)))

def extract_spectral_bandwidth(audio_data, sr=44100):
    y = np.array(audio_data, dtype=np.float32)
    return float(np.mean(librosa.feature.spectral_bandwidth(y=y, sr=sr)))

def extract_spectral_rolloff(audio_data, sr=44100):
    y = np.array(audio_data, dtype=np.float32)
    return float(np.mean(librosa.feature.spectral_rolloff(y=y, sr=sr)))

def extract_mfcc(audio_data, n_mfcc=13, sr=44100):
    y = np.array(audio_data, dtype=np.float32)
    mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=n_mfcc)
    return [float(np.mean(mfccs[i])) for i in range(n_mfcc)]

def extract_rms(audio_data, sr=44100):
    y = np.array(audio_data, dtype=np.float32)
    return float(np.mean(librosa.feature.rms(y=y)))
)";
    
    // Execute the Python script
    if (PyRun_SimpleString(pythonScript) != 0)
    {
        std::cerr << "Failed to execute Python feature extraction script" << std::endl;
        PyErr_Print();
        return false;
    }
    
    // Get Python main module
    pModule_ = PyImport_AddModule("__main__");
    if (!pModule_)
    {
        std::cerr << "Failed to get Python main module" << std::endl;
        return false;
    }
    
    // Get function references
    pExtractFeatures_ = PyObject_GetAttrString(pModule_, "extract_features_vector");
    pExtractCentroid_ = PyObject_GetAttrString(pModule_, "extract_spectral_centroid");
    pExtractBandwidth_ = PyObject_GetAttrString(pModule_, "extract_spectral_bandwidth");
    pExtractRolloff_ = PyObject_GetAttrString(pModule_, "extract_spectral_rolloff");
    pExtractMFCC_ = PyObject_GetAttrString(pModule_, "extract_mfcc");
    pExtractRMS_ = PyObject_GetAttrString(pModule_, "extract_rms");
    
    if (!pExtractFeatures_ || !PyCallable_Check(pExtractFeatures_))
    {
        std::cerr << "Failed to get extract_features_vector function" << std::endl;
        return false;
    }
    
    pythonInitialized_ = true;
    std::cout << "✅ Python Feature Extractor initialized successfully!" << std::endl;
    
    // Release GIL after initialization
    PyGILState_Release(gstate);
    
    return true;
}

void PythonFeatureExtractor::cleanup()
{
    // Decrease reference counts
    if (pExtractFeatures_) { Py_DECREF(pExtractFeatures_); pExtractFeatures_ = nullptr; }
    if (pExtractCentroid_) { Py_DECREF(pExtractCentroid_); pExtractCentroid_ = nullptr; }
    if (pExtractBandwidth_) { Py_DECREF(pExtractBandwidth_); pExtractBandwidth_ = nullptr; }
    if (pExtractRolloff_) { Py_DECREF(pExtractRolloff_); pExtractRolloff_ = nullptr; }
    if (pExtractMFCC_) { Py_DECREF(pExtractMFCC_); pExtractMFCC_ = nullptr; }
    if (pExtractRMS_) { Py_DECREF(pExtractRMS_); pExtractRMS_ = nullptr; }
    
    pythonInitialized_ = false;
    
    // Note: Don't call Py_Finalize() as it might be used elsewhere
}

std::vector<float> PythonFeatureExtractor::extractFeatures(const std::vector<float>& audioData, double sampleRate)
{
    if (!pythonInitialized_ || !pExtractFeatures_)
    {
        std::cerr << "Python Feature Extractor not initialized!" << std::endl;
        return std::vector<float>(17, 0.0f);
    }
    
    // Acquire GIL for thread safety
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    // Convert audio data to Python list
    PyObject* pyAudioData = convertAudioDataToPython(audioData);
    if (!pyAudioData)
    {
        PyGILState_Release(gstate);
        return std::vector<float>(17, 0.0f);
    }
    
    // Prepare arguments
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, pyAudioData);  // audio_data
    PyTuple_SetItem(pArgs, 1, PyFloat_FromDouble(sampleRate));  // sr
    
    // Call Python function
    PyObject* pResult = PyObject_CallObject(pExtractFeatures_, pArgs);
    Py_DECREF(pArgs);
    
    if (!pResult)
    {
        std::cerr << "Python feature extraction failed!" << std::endl;
        PyErr_Print();
        PyGILState_Release(gstate);
        return std::vector<float>(17, 0.0f);
    }
    
    // Convert result to C++ vector
    std::vector<float> features = convertPythonToVector(pResult);
    Py_DECREF(pResult);
    
    // Debug: print the extracted features
    std::cout << "DEBUG: Extracted features (" << features.size() << "): ";
    for (size_t i = 0; i < features.size() && i < 5; ++i) {
        std::cout << features[i] << " ";
    }
    std::cout << "..." << std::endl;
    
    // Release GIL
    PyGILState_Release(gstate);
    
    return features;
}

float PythonFeatureExtractor::extractSpectralCentroid(const std::vector<float>& audioData, double sampleRate)
{
    if (!pythonInitialized_ || !pExtractCentroid_)
        return 0.0f;
    
    PyObject* pyAudioData = convertAudioDataToPython(audioData);
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, pyAudioData);
    PyTuple_SetItem(pArgs, 1, PyFloat_FromDouble(sampleRate));
    
    PyObject* pResult = PyObject_CallObject(pExtractCentroid_, pArgs);
    Py_DECREF(pArgs);
    
    if (!pResult)
        return 0.0f;
    
    float result = convertPythonToFloat(pResult);
    Py_DECREF(pResult);
    return result;
}

float PythonFeatureExtractor::extractSpectralBandwidth(const std::vector<float>& audioData, double sampleRate)
{
    if (!pythonInitialized_ || !pExtractBandwidth_)
        return 0.0f;
    
    PyObject* pyAudioData = convertAudioDataToPython(audioData);
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, pyAudioData);
    PyTuple_SetItem(pArgs, 1, PyFloat_FromDouble(sampleRate));
    
    PyObject* pResult = PyObject_CallObject(pExtractBandwidth_, pArgs);
    Py_DECREF(pArgs);
    
    if (!pResult)
        return 0.0f;
    
    float result = convertPythonToFloat(pResult);
    Py_DECREF(pResult);
    return result;
}

float PythonFeatureExtractor::extractSpectralRolloff(const std::vector<float>& audioData, double sampleRate)
{
    if (!pythonInitialized_ || !pExtractRolloff_)
        return 0.0f;
    
    PyObject* pyAudioData = convertAudioDataToPython(audioData);
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, pyAudioData);
    PyTuple_SetItem(pArgs, 1, PyFloat_FromDouble(sampleRate));
    
    PyObject* pResult = PyObject_CallObject(pExtractRolloff_, pArgs);
    Py_DECREF(pArgs);
    
    if (!pResult)
        return 0.0f;
    
    float result = convertPythonToFloat(pResult);
    Py_DECREF(pResult);
    return result;
}

std::vector<float> PythonFeatureExtractor::extractMFCC(const std::vector<float>& audioData, int numCoeffs, double sampleRate)
{
    if (!pythonInitialized_ || !pExtractMFCC_)
        return std::vector<float>(numCoeffs, 0.0f);
    
    PyObject* pyAudioData = convertAudioDataToPython(audioData);
    PyObject* pArgs = PyTuple_New(3);
    PyTuple_SetItem(pArgs, 0, pyAudioData);
    PyTuple_SetItem(pArgs, 1, PyLong_FromLong(numCoeffs));
    PyTuple_SetItem(pArgs, 2, PyFloat_FromDouble(sampleRate));
    
    PyObject* pResult = PyObject_CallObject(pExtractMFCC_, pArgs);
    Py_DECREF(pArgs);
    
    if (!pResult)
        return std::vector<float>(numCoeffs, 0.0f);
    
    std::vector<float> result = convertPythonToVector(pResult);
    Py_DECREF(pResult);
    return result;
}

float PythonFeatureExtractor::extractRMSEnergy(const std::vector<float>& audioData, double sampleRate)
{
    if (!pythonInitialized_ || !pExtractRMS_)
        return 0.0f;
    
    PyObject* pyAudioData = convertAudioDataToPython(audioData);
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, pyAudioData);
    PyTuple_SetItem(pArgs, 1, PyFloat_FromDouble(sampleRate));
    
    PyObject* pResult = PyObject_CallObject(pExtractRMS_, pArgs);
    Py_DECREF(pArgs);
    
    if (!pResult)
        return 0.0f;
    
    float result = convertPythonToFloat(pResult);
    Py_DECREF(pResult);
    return result;
}

// Helper methods
PyObject* PythonFeatureExtractor::convertAudioDataToPython(const std::vector<float>& audioData)
{
    PyObject* pyList = PyList_New(audioData.size());
    if (!pyList)
        return nullptr;
    
    for (size_t i = 0; i < audioData.size(); ++i)
    {
        PyObject* pyFloat = PyFloat_FromDouble(static_cast<double>(audioData[i]));
        PyList_SetItem(pyList, i, pyFloat);
    }
    
    return pyList;
}

std::vector<float> PythonFeatureExtractor::convertPythonToVector(PyObject* pyList)
{
    if (!PyList_Check(pyList))
    {
        std::cerr << "ERROR: convertPythonToVector - not a list!" << std::endl;
        return {};
    }
    
    Py_ssize_t size = PyList_Size(pyList);
    std::vector<float> result;
    result.reserve(size);
    
    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject* item = PyList_GetItem(pyList, i);
        if (PyFloat_Check(item))
        {
            float val = static_cast<float>(PyFloat_AsDouble(item));
            result.push_back(val);
        }
        else if (PyLong_Check(item))
        {
            float val = static_cast<float>(PyLong_AsLong(item));
            result.push_back(val);
        }
        else
        {
            // Try to convert to float using PyFloat_AsDouble (handles numpy types)
            PyObject* floatObj = PyNumber_Float(item);
            if (floatObj)
            {
                float val = static_cast<float>(PyFloat_AsDouble(floatObj));
                result.push_back(val);
                Py_DECREF(floatObj);
            }
            else
            {
                result.push_back(0.0f);
                PyErr_Clear(); // Clear any Python errors
            }
        }
    }
    
    return result;
}

float PythonFeatureExtractor::convertPythonToFloat(PyObject* pyFloat)
{
    if (PyFloat_Check(pyFloat))
    {
        return static_cast<float>(PyFloat_AsDouble(pyFloat));
    }
    else if (PyLong_Check(pyFloat))
    {
        return static_cast<float>(PyLong_AsLong(pyFloat));
    }
    return 0.0f;
}