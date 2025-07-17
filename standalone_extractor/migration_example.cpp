/**
 * Example: How to migrate from PythonFeatureExtractor to ExternalFeatureExtractor
 * 
 * This shows how to modify existing code to support both implementations
 * with a runtime switch.
 */

#include "../Source/VTR/PythonFeatureExtractor.h"
#include "../Source/VTR/ExternalFeatureExtractor.h"
#include <memory>
#include <iostream>

// Base interface for feature extraction
class IFeatureExtractor
{
public:
    virtual ~IFeatureExtractor() = default;
    virtual bool initialize() = 0;
    virtual std::vector<float> extractFeatures(const std::vector<float>& audioData, double sampleRate = 44100.0) = 0;
    virtual float extractSpectralCentroid(const std::vector<float>& audioData, double sampleRate = 44100.0) = 0;
    virtual float extractSpectralBandwidth(const std::vector<float>& audioData, double sampleRate = 44100.0) = 0;
    virtual float extractSpectralRolloff(const std::vector<float>& audioData, double sampleRate = 44100.0) = 0;
    virtual std::vector<float> extractMFCC(const std::vector<float>& audioData, int numCoeffs = 13, double sampleRate = 44100.0) = 0;
    virtual float extractRMSEnergy(const std::vector<float>& audioData, double sampleRate = 44100.0) = 0;
};

// Wrapper for PythonFeatureExtractor
class PythonFeatureExtractorWrapper : public IFeatureExtractor
{
private:
    PythonFeatureExtractor extractor_;
    
public:
    bool initialize() override
    {
        return extractor_.initialize();
    }
    
    std::vector<float> extractFeatures(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractFeatures(audioData, sampleRate);
    }
    
    float extractSpectralCentroid(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractSpectralCentroid(audioData, sampleRate);
    }
    
    float extractSpectralBandwidth(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractSpectralBandwidth(audioData, sampleRate);
    }
    
    float extractSpectralRolloff(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractSpectralRolloff(audioData, sampleRate);
    }
    
    std::vector<float> extractMFCC(const std::vector<float>& audioData, int numCoeffs, double sampleRate) override
    {
        return extractor_.extractMFCC(audioData, numCoeffs, sampleRate);
    }
    
    float extractRMSEnergy(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractRMSEnergy(audioData, sampleRate);
    }
};

// Wrapper for ExternalFeatureExtractor
class ExternalFeatureExtractorWrapper : public IFeatureExtractor
{
private:
    ExternalFeatureExtractor extractor_;
    std::string executablePath_;
    
public:
    ExternalFeatureExtractorWrapper(const std::string& execPath = "")
        : executablePath_(execPath.empty() ? ExternalFeatureExtractor::getDefaultExecutablePath() : execPath)
    {
    }
    
    bool initialize() override
    {
        return extractor_.initialize(executablePath_);
    }
    
    std::vector<float> extractFeatures(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractFeatures(audioData, sampleRate);
    }
    
    float extractSpectralCentroid(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractSpectralCentroid(audioData, sampleRate);
    }
    
    float extractSpectralBandwidth(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractSpectralBandwidth(audioData, sampleRate);
    }
    
    float extractSpectralRolloff(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractSpectralRolloff(audioData, sampleRate);
    }
    
    std::vector<float> extractMFCC(const std::vector<float>& audioData, int numCoeffs, double sampleRate) override
    {
        return extractor_.extractMFCC(audioData, numCoeffs, sampleRate);
    }
    
    float extractRMSEnergy(const std::vector<float>& audioData, double sampleRate) override
    {
        return extractor_.extractRMSEnergy(audioData, sampleRate);
    }
};

// Factory function to create appropriate extractor
std::unique_ptr<IFeatureExtractor> createFeatureExtractor(bool useExternal = false, const std::string& execPath = "")
{
    if (useExternal)
    {
        std::cout << "Using external feature extractor" << std::endl;
        return std::make_unique<ExternalFeatureExtractorWrapper>(execPath);
    }
    else
    {
        std::cout << "Using embedded Python feature extractor" << std::endl;
        return std::make_unique<PythonFeatureExtractorWrapper>();
    }
}

// Example usage in a VST processor class
class AudioProcessor
{
private:
    std::unique_ptr<IFeatureExtractor> featureExtractor_;
    bool useExternalExtractor_;
    
public:
    AudioProcessor(bool useExternal = false)
        : useExternalExtractor_(useExternal)
    {
    }
    
    bool initialize()
    {
        // Create feature extractor based on configuration
        featureExtractor_ = createFeatureExtractor(useExternalExtractor_);
        
        if (!featureExtractor_->initialize())
        {
            std::cerr << "Failed to initialize feature extractor!" << std::endl;
            
            // Fallback to embedded Python if external fails
            if (useExternalExtractor_)
            {
                std::cout << "Falling back to embedded Python extractor..." << std::endl;
                featureExtractor_ = createFeatureExtractor(false);
                return featureExtractor_->initialize();
            }
            
            return false;
        }
        
        return true;
    }
    
    void processAudioBlock(const std::vector<float>& audioData, double sampleRate)
    {
        // Extract features
        auto features = featureExtractor_->extractFeatures(audioData, sampleRate);
        
        // Use features for processing...
        std::cout << "Extracted " << features.size() << " features" << std::endl;
    }
};

// Configuration example
class PluginConfiguration
{
public:
    static bool shouldUseExternalExtractor()
    {
        // This could read from:
        // - User preferences
        // - Environment variable
        // - Configuration file
        // - Command line argument
        
        // For now, check environment variable
        const char* env = std::getenv("VTR_USE_EXTERNAL_EXTRACTOR");
        return env != nullptr && std::string(env) == "1";
    }
    
    static std::string getExternalExtractorPath()
    {
        const char* env = std::getenv("VTR_EXTRACTOR_PATH");
        return env ? std::string(env) : "";
    }
};

// Main example
int main()
{
    // Determine which extractor to use
    bool useExternal = PluginConfiguration::shouldUseExternalExtractor();
    
    // Create audio processor
    AudioProcessor processor(useExternal);
    
    // Initialize
    if (!processor.initialize())
    {
        std::cerr << "Failed to initialize audio processor!" << std::endl;
        return 1;
    }
    
    // Process some test audio
    std::vector<float> testAudio(44100, 0.0f);  // 1 second of silence
    processor.processAudioBlock(testAudio, 44100.0);
    
    std::cout << "Processing completed successfully!" << std::endl;
    return 0;
}