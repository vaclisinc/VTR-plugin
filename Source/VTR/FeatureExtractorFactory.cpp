#include "FeatureExtractorFactory.h"
#include "JuceFeatureExtractor.h"
#include "PythonFeatureExtractor.h"
#include "ExternalFeatureExtractor.h"
#include <juce_core/juce_core.h>

namespace VTR {

std::unique_ptr<FeatureExtractor> FeatureExtractorFactory::create(
    ExtractorType type, bool fallbackToJuce)
{
    // Handle AUTO type
    if (type == ExtractorType::AUTO)
    {
        type = getPreferredExtractorType();
    }
    
    std::unique_ptr<FeatureExtractor> extractor;
    
    switch (type)
    {
        case ExtractorType::EXTERNAL_PROCESS:
            extractor = createExternalExtractor();
            if (extractor)
            {
                DBG("Created External Process feature extractor");
                return extractor;
            }
            DBG("Failed to create External Process extractor");
            break;
            
        case ExtractorType::PYTHON_LIBROSA:
            extractor = createPythonExtractor();
            if (extractor)
            {
                DBG("Created Python/Librosa feature extractor");
                return extractor;
            }
            DBG("Failed to create Python/Librosa extractor");
            break;
            
        case ExtractorType::JUCE_BASED:
            extractor = createJuceExtractor();
            if (extractor)
            {
                DBG("Created JUCE-based feature extractor");
                return extractor;
            }
            DBG("Failed to create JUCE extractor");
            break;
            
        default:
            break;
    }
    
    // Fallback to JUCE if requested
    if (fallbackToJuce && type != ExtractorType::JUCE_BASED)
    {
        DBG("Falling back to JUCE-based feature extractor");
        return createJuceExtractor();
    }
    
    return nullptr;
}

std::string FeatureExtractorFactory::getExtractorTypeName(ExtractorType type)
{
    switch (type)
    {
        case ExtractorType::JUCE_BASED:
            return "JUCE-based";
        case ExtractorType::PYTHON_LIBROSA:
            return "Python/Librosa";
        case ExtractorType::EXTERNAL_PROCESS:
            return "External Process";
        case ExtractorType::AUTO:
            return "Auto";
        default:
            return "Unknown";
    }
}

bool FeatureExtractorFactory::isExtractorAvailable(ExtractorType type)
{
    switch (type)
    {
        case ExtractorType::JUCE_BASED:
            // JUCE is always available
            return true;
            
        case ExtractorType::PYTHON_LIBROSA:
            // Check if Python is available
            try
            {
                auto extractor = std::make_unique<PythonFeatureExtractor>();
                return true;
            }
            catch (...)
            {
                return false;
            }
            
        case ExtractorType::EXTERNAL_PROCESS:
            // Check if external executable exists
            return !ExternalFeatureExtractor::getExecutablePath().empty();
            
        case ExtractorType::AUTO:
            return true; // AUTO is always "available"
            
        default:
            return false;
    }
}

FeatureExtractorFactory::ExtractorType FeatureExtractorFactory::getPreferredExtractorType()
{
    // Priority order:
    // 1. External process (no Python required, most reliable)
    // 2. Python/Librosa (if Python is installed)
    // 3. JUCE (always available as fallback)
    
    if (isExtractorAvailable(ExtractorType::EXTERNAL_PROCESS))
    {
        return ExtractorType::EXTERNAL_PROCESS;
    }
    
    if (isExtractorAvailable(ExtractorType::PYTHON_LIBROSA))
    {
        return ExtractorType::PYTHON_LIBROSA;
    }
    
    return ExtractorType::JUCE_BASED;
}

std::unique_ptr<FeatureExtractor> FeatureExtractorFactory::createJuceExtractor()
{
    try
    {
        return std::make_unique<JuceFeatureExtractor>();
    }
    catch (const std::exception& e)
    {
        DBG("Failed to create JUCE extractor: " << e.what());
        return nullptr;
    }
}

std::unique_ptr<FeatureExtractor> FeatureExtractorFactory::createPythonExtractor()
{
    try
    {
        return std::make_unique<PythonFeatureExtractor>();
    }
    catch (const std::exception& e)
    {
        DBG("Failed to create Python extractor: " << e.what());
        return nullptr;
    }
}

std::unique_ptr<FeatureExtractor> FeatureExtractorFactory::createExternalExtractor()
{
    try
    {
        // Check if executable exists first
        if (ExternalFeatureExtractor::getExecutablePath().empty())
        {
            DBG("External extractor executable not found");
            return nullptr;
        }
        
        auto extractor = std::make_unique<ExternalFeatureExtractor>();
        
        // Verify it can start
        if (!extractor->isProcessRunning())
        {
            DBG("External extractor process failed to start");
            return nullptr;
        }
        
        return extractor;
    }
    catch (const std::exception& e)
    {
        DBG("Failed to create External extractor: " << e.what());
        return nullptr;
    }
}

} // namespace VTR