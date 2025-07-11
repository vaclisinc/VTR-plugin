#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <iomanip>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "Source/SpectrumAnalyzer.h"

class SimpleFeatureExtractor
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
            // Mix to mono
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
            // Already mono
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
        
        // Print expected Python values for comparison
        std::cout << "\nExpected Python values:" << std::endl;
        float expectedValues[] = {
            0.006516f,      // rms_energy
            1383.135591f,   // spectral_centroid
            -660.778870f,   // mfcc_1
            164.444977f,    // mfcc_2
            59.139645f,     // mfcc_3
            -6.343942f,     // mfcc_4
            -10.870620f,    // mfcc_5
            5.779136f,      // mfcc_6
            11.385731f,     // mfcc_7
            5.200136f,      // mfcc_8
            -0.208707f,     // mfcc_9
            1.907096f,      // mfcc_10
            6.202437f,      // mfcc_11
            4.327792f,      // mfcc_12
            -2.147642f,     // mfcc_13
            2115.860989f,   // spectral_bandwidth
            2280.678286f    // spectral_rolloff
        };
        
        for (int i = 0; i < 17; ++i)
        {
            std::cout << "  expected[" << std::setw(2) << i << "] = " 
                      << std::setw(12) << std::fixed << std::setprecision(6) << expectedValues[i] 
                      << "  // " << featureNames[i] << std::endl;
        }
        
        // Calculate differences
        std::cout << "\nDifferences (C++ - Python):" << std::endl;
        for (size_t i = 0; i < features.size() && i < 17; ++i)
        {
            float diff = features[i] - expectedValues[i];
            float relativeError = expectedValues[i] != 0.0f ? 
                std::abs(diff / expectedValues[i]) * 100.0f : 0.0f;
            
            std::cout << "  diff[" << std::setw(2) << i << "] = " 
                      << std::setw(12) << std::fixed << std::setprecision(6) << diff
                      << " (" << std::setw(8) << std::setprecision(2) << relativeError << "%)"
                      << "  // " << featureNames[i] << std::endl;
        }
    }
    
private:
    double sampleRate = 44100.0;
};

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <audio_file>" << std::endl;
        return 1;
    }
    
    juce::initialiseJuce_GUI();
    
    SimpleFeatureExtractor extractor;
    extractor.testFeatureExtraction(argv[1]);
    
    juce::shutdownJuce_GUI();
    return 0;
}