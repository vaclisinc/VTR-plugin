#pragma once

#include "FeatureExtractor.h"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace VTR {

/**
 * External feature extractor that communicates with standalone Python executable
 * Uses JSON protocol over stdin/stdout pipes
 */
class ExternalFeatureExtractor : public FeatureExtractor
{
public:
    ExternalFeatureExtractor();
    ~ExternalFeatureExtractor() override;

    // FeatureExtractor interface
    std::vector<float> extractFeatures(const float* audioData, 
                                     size_t numSamples, 
                                     double sampleRate) override;
    
    float extractSpectralCentroid(const float* audioData, 
                                 size_t numSamples, 
                                 double sampleRate) override;
    
    float extractSpectralBandwidth(const float* audioData, 
                                  size_t numSamples, 
                                  double sampleRate) override;
    
    float extractSpectralRolloff(const float* audioData, 
                                size_t numSamples, 
                                double sampleRate) override;
    
    std::vector<float> extractMFCC(const float* audioData, 
                                  size_t numSamples, 
                                  double sampleRate, 
                                  int numCoefficients = 13) override;
    
    float extractRMS(const float* audioData, 
                    size_t numSamples, 
                    double sampleRate) override;

    // Process management
    bool startProcess();
    void stopProcess();
    bool isProcessRunning() const { return processRunning.load(); }
    
    // Get path to the external executable
    static std::string getExecutablePath();

private:
    // Process communication
    struct ProcessHandles {
        FILE* stdinPipe = nullptr;
        FILE* stdoutPipe = nullptr;
        pid_t pid = -1;
    };
    
    std::unique_ptr<ProcessHandles> process;
    std::atomic<bool> processRunning{false};
    std::mutex processMutex;
    
    // Message sending/receiving
    bool sendMessage(const nlohmann::json& message);
    bool receiveMessage(nlohmann::json& message, int timeoutMs = 5000);
    
    // Helper to extract all features at once (more efficient)
    std::vector<float> extractAllFeatures(const float* audioData, 
                                         size_t numSamples, 
                                         double sampleRate);
    
    // Base64 encoding for audio data
    static std::string base64Encode(const std::vector<float>& data);
    
    // Error handling
    std::string lastError;
    void setError(const std::string& error) { lastError = error; }
};

} // namespace VTR