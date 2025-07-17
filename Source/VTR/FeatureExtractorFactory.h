#pragma once

#include "FeatureExtractor.h"
#include <memory>
#include <string>

namespace VTR {

/**
 * Factory class for creating feature extractors
 * Allows runtime selection between different implementations
 */
class FeatureExtractorFactory
{
public:
    enum class ExtractorType
    {
        JUCE_BASED,          // Pure JUCE implementation
        PYTHON_LIBROSA,      // Embedded Python (requires Python installed)
        EXTERNAL_PROCESS,    // External executable (no Python required)
        AUTO                 // Automatically choose best available
    };
    
    /**
     * Create a feature extractor of the specified type
     * @param type The type of extractor to create
     * @param fallbackToJuce If true, falls back to JUCE if requested type fails
     * @return A unique_ptr to the created extractor
     */
    static std::unique_ptr<FeatureExtractor> create(
        ExtractorType type = ExtractorType::AUTO,
        bool fallbackToJuce = true
    );
    
    /**
     * Get the name of an extractor type
     */
    static std::string getExtractorTypeName(ExtractorType type);
    
    /**
     * Check if a specific extractor type is available
     */
    static bool isExtractorAvailable(ExtractorType type);
    
    /**
     * Get the currently preferred extractor type based on availability
     */
    static ExtractorType getPreferredExtractorType();
    
private:
    static std::unique_ptr<FeatureExtractor> createJuceExtractor();
    static std::unique_ptr<FeatureExtractor> createPythonExtractor();
    static std::unique_ptr<FeatureExtractor> createExternalExtractor();
};

} // namespace VTR