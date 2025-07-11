#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <iomanip>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "Source/SpectrumAnalyzer.h"

class FlexibleFeatureExtractor
{
public:
    std::vector<float> loadAudioFile(const std::string& filePath)
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        
        juce::File audioFile(filePath);
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));
        
        if (!reader)
        {
            std::cerr << "Failed to load audio file: " << filePath << std::endl;
            return {};
        }
        
        // Read audio data
        juce::AudioBuffer<float> buffer(reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
        
        // Convert to mono
        std::vector<float> audioData;
        if (buffer.getNumChannels() > 1)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float sample = 0.0f;
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    sample += buffer.getSample(ch, i);
                }
                audioData.push_back(sample / buffer.getNumChannels());
            }
        }
        else
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                audioData.push_back(buffer.getSample(0, i));
            }
        }
        
        sampleRate = reader->sampleRate;
        return audioData;
    }
    
    void testFeatureExtraction(const std::string& audioFile)
    {
        std::cout << "Testing JUCE feature extraction..." << std::endl;
        std::cout << "Audio file: " << audioFile << std::endl;
        
        // Load audio
        std::vector<float> audioData = loadAudioFile(audioFile);
        if (audioData.empty())
        {
            std::cerr << "Failed to load audio data" << std::endl;
            return;
        }
        
        std::cout << "Loaded " << audioData.size() << " samples at " << sampleRate << " Hz" << std::endl;
        
        // Extract features
        SpectrumAnalyzer analyzer;
        std::vector<float> features = analyzer.extractFeatures(audioData, sampleRate);
        
        std::cout << "\nExtracted " << features.size() << " features:" << std::endl;
        
        // Print features in C++ order
        const char* featureNames[] = {
            "rms_energy",
            "spectral_centroid",
            "mfcc_1", "mfcc_2", "mfcc_3", "mfcc_4", "mfcc_5", "mfcc_6", "mfcc_7",
            "mfcc_8", "mfcc_9", "mfcc_10", "mfcc_11", "mfcc_12", "mfcc_13",
            "spectral_bandwidth",
            "spectral_rolloff"
        };
        
        for (size_t i = 0; i < features.size() && i < 17; ++i)
        {
            std::cout << "  features[" << std::setw(2) << i << "] = " 
                      << std::setw(12) << std::fixed << std::setprecision(6) << features[i] 
                      << "  // " << featureNames[i] << std::endl;
        }
        
        // Extract filename for Python command
        std::string filename = audioFile;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            filename = filename.substr(lastSlash + 1);
        }
        
        std::cout << "\n" << "=" << std::setfill('=') << std::setw(60) << "" << std::endl;
        std::cout << "TO GET PYTHON COMPARISON VALUES:" << std::endl;
        std::cout << "=" << std::setfill('=') << std::setw(60) << "" << std::setfill(' ') << std::endl;
        std::cout << "Run this command:" << std::endl;
        std::cout << "cd /Users/vaclis./Documents/project/VTR-plugin/vtr-model" << std::endl;
        std::cout << "source venv/bin/activate" << std::endl;
        std::cout << "python test_any_audio.py '" << audioFile << "'" << std::endl;
        
        std::cout << "\nThen compare the Python values with the C++ values above." << std::endl;
        std::cout << "For file: " << filename << std::endl;
    }
    
private:
    double sampleRate = 44100.0;
};

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <audio_file>" << std::endl;
        std::cerr << "Example: " << argv[0] << " path/to/audio.wav" << std::endl;
        return 1;
    }
    
    FlexibleFeatureExtractor extractor;
    extractor.testFeatureExtraction(argv[1]);
    
    return 0;
}