#include "ExternalFeatureExtractor.h"
#include <nlohmann/json.hpp>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <chrono>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define popen _popen
    #define pclose _pclose
#else
    #include <spawn.h>
    extern char **environ;
#endif

namespace VTR {

using json = nlohmann::json;

// Base64 encoding table
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

ExternalFeatureExtractor::ExternalFeatureExtractor()
    : process(std::make_unique<ProcessHandles>())
{
    startProcess();
}

ExternalFeatureExtractor::~ExternalFeatureExtractor()
{
    stopProcess();
}

std::string ExternalFeatureExtractor::getExecutablePath()
{
    // Determine platform-specific executable name
    std::string execName = "vtr-feature-extractor";
#ifdef _WIN32
    execName += ".exe";
#endif
    
    // Try multiple locations for the executable
    std::vector<std::string> possiblePaths;
    
    // Get the plugin/app location
    auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    
    // Platform-specific resource locations
#ifdef JUCE_MAC
    // macOS: Inside app bundle
    possiblePaths.push_back(
        appFile.getChildFile("Contents")
               .getChildFile("Resources")
               .getChildFile(execName)
               .getFullPathName().toStdString()
    );
#elif defined(_WIN32)
    // Windows: Next to the plugin DLL
    possiblePaths.push_back(
        appFile.getParentDirectory()
               .getChildFile(execName)
               .getFullPathName().toStdString()
    );
#else
    // Linux: Next to the plugin SO
    possiblePaths.push_back(
        appFile.getParentDirectory()
               .getChildFile(execName)
               .getFullPathName().toStdString()
    );
#endif
    
    // Development locations (cross-platform)
    auto projectRoot = appFile.getParentDirectory()
                              .getParentDirectory()
                              .getParentDirectory();
    
    possiblePaths.push_back(
        projectRoot.getChildFile("standalone_extractor")
                   .getChildFile("dist")
                   .getChildFile(execName)
                   .getFullPathName().toStdString()
    );
    
    // Current directory variations
    possiblePaths.push_back("./" + execName);
    possiblePaths.push_back("./standalone_extractor/dist/" + execName);
    possiblePaths.push_back("../standalone_extractor/dist/" + execName);
    
    // Environment variable override
    const char* envPath = std::getenv("VTR_EXTRACTOR_PATH");
    if (envPath)
    {
        possiblePaths.insert(possiblePaths.begin(), envPath);
    }
    
    // System PATH (as fallback)
    possiblePaths.push_back(execName);
    
    // Try each path
    for (const auto& path : possiblePaths)
    {
        if (juce::File(path).existsAsFile())
        {
            DBG("Found feature extractor at: " << path);
            return path;
        }
    }
    
    DBG("Feature extractor not found in any of the expected locations");
    DBG("Searched paths:");
    for (const auto& path : possiblePaths)
    {
        DBG("  - " << path);
    }
    
    return "";
}

bool ExternalFeatureExtractor::startProcess()
{
    std::lock_guard<std::mutex> lock(processMutex);
    
    if (processRunning)
        return true;
    
    std::string execPath = getExecutablePath();
    if (execPath.empty())
    {
        setError("Feature extractor executable not found");
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation using CreateProcess
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr;
    
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0) ||
        !CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
    {
        setError("Failed to create pipes");
        return false;
    }
    
    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStdoutWr;
    siStartInfo.hStdOutput = hChildStdoutWr;
    siStartInfo.hStdInput = hChildStdinRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    
    std::string cmdLine = execPath + " --daemon";
    if (!CreateProcess(NULL, const_cast<char*>(cmdLine.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
    {
        setError("Failed to start process");
        return false;
    }
    
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);
    
    process->stdinPipe = _fdopen(_open_osfhandle((intptr_t)hChildStdinWr, 0), "wb");
    process->stdoutPipe = _fdopen(_open_osfhandle((intptr_t)hChildStdoutRd, 0), "rb");
    process->pid = piProcInfo.dwProcessId;
    
#else
    // POSIX implementation using pipe() and fork()
    int stdinPipe[2];
    int stdoutPipe[2];
    
    if (pipe(stdinPipe) != 0 || pipe(stdoutPipe) != 0)
    {
        setError("Failed to create pipes");
        return false;
    }
    
    pid_t pid = fork();
    if (pid < 0)
    {
        setError("Failed to fork process");
        return false;
    }
    else if (pid == 0)
    {
        // Child process
        close(stdinPipe[1]);  // Close write end of stdin pipe
        close(stdoutPipe[0]); // Close read end of stdout pipe
        
        // Redirect stdin/stdout
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stdoutPipe[1], STDERR_FILENO);
        
        // Close unused descriptors
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        
        // Execute the Python extractor
        execl(execPath.c_str(), "vtr-feature-extractor", "--daemon", nullptr);
        
        // If we get here, exec failed
        exit(1);
    }
    else
    {
        // Parent process
        close(stdinPipe[0]);  // Close read end of stdin pipe
        close(stdoutPipe[1]); // Close write end of stdout pipe
        
        process->stdinPipe = fdopen(stdinPipe[1], "wb");
        process->stdoutPipe = fdopen(stdoutPipe[0], "rb");
        process->pid = pid;
    }
#endif
    
    // Wait for ready signal
    json response;
    if (!receiveMessage(response) || response["status"] != "ready")
    {
        setError("Failed to receive ready signal from extractor");
        stopProcess();
        return false;
    }
    
    processRunning = true;
    DBG("External feature extractor started successfully");
    return true;
}

void ExternalFeatureExtractor::stopProcess()
{
    std::lock_guard<std::mutex> lock(processMutex);
    
    if (!processRunning)
        return;
    
    // Send exit command
    json exitCmd;
    exitCmd["command"] = "exit";
    sendMessage(exitCmd);
    
    // Close pipes
    if (process->stdinPipe)
    {
        fclose(process->stdinPipe);
        process->stdinPipe = nullptr;
    }
    
    if (process->stdoutPipe)
    {
        fclose(process->stdoutPipe);
        process->stdoutPipe = nullptr;
    }
    
    // Wait for process to exit
    if (process->pid > 0)
    {
#ifdef _WIN32
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, process->pid);
        if (hProcess)
        {
            WaitForSingleObject(hProcess, 5000);
            CloseHandle(hProcess);
        }
#else
        int status;
        waitpid(process->pid, &status, 0);
#endif
        process->pid = -1;
    }
    
    processRunning = false;
    DBG("External feature extractor stopped");
}

std::string ExternalFeatureExtractor::base64Encode(const std::vector<float>& data)
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data.data());
    size_t len = data.size() * sizeof(float);
    
    std::string encoded;
    encoded.reserve(((len + 2) / 3) * 4);
    
    for (size_t i = 0; i < len; i += 3)
    {
        unsigned char b1 = bytes[i];
        unsigned char b2 = (i + 1 < len) ? bytes[i + 1] : 0;
        unsigned char b3 = (i + 2 < len) ? bytes[i + 2] : 0;
        
        encoded += base64_table[b1 >> 2];
        encoded += base64_table[((b1 & 0x03) << 4) | (b2 >> 4)];
        encoded += (i + 1 < len) ? base64_table[((b2 & 0x0f) << 2) | (b3 >> 6)] : '=';
        encoded += (i + 2 < len) ? base64_table[b3 & 0x3f] : '=';
    }
    
    return encoded;
}

bool ExternalFeatureExtractor::sendMessage(const json& message)
{
    if (!process->stdinPipe)
        return false;
    
    std::string msgStr = message.dump();
    uint32_t msgLen = static_cast<uint32_t>(msgStr.length());
    
    // Write 4-byte length prefix
    if (fwrite(&msgLen, sizeof(uint32_t), 1, process->stdinPipe) != 1)
        return false;
    
    // Write message
    if (fwrite(msgStr.c_str(), 1, msgLen, process->stdinPipe) != msgLen)
        return false;
    
    fflush(process->stdinPipe);
    return true;
}

bool ExternalFeatureExtractor::receiveMessage(json& message, int timeoutMs)
{
    if (!process->stdoutPipe)
        return false;
    
    // Read 4-byte length prefix
    uint32_t msgLen;
    if (fread(&msgLen, sizeof(uint32_t), 1, process->stdoutPipe) != 1)
        return false;
    
    // Sanity check
    if (msgLen > 10 * 1024 * 1024) // 10MB max
        return false;
    
    // Read message
    std::vector<char> buffer(msgLen + 1);
    if (fread(buffer.data(), 1, msgLen, process->stdoutPipe) != msgLen)
        return false;
    
    buffer[msgLen] = '\0';
    
    try
    {
        message = json::parse(buffer.data());
        return true;
    }
    catch (const json::exception& e)
    {
        setError("Failed to parse JSON response: " + std::string(e.what()));
        return false;
    }
}

std::vector<float> ExternalFeatureExtractor::extractAllFeatures(const float* audioData, 
                                                               size_t numSamples, 
                                                               double sampleRate)
{
    if (!processRunning && !startProcess())
    {
        DBG("Failed to start feature extractor process");
        return std::vector<float>(17, 0.0f);
    }
    
    std::lock_guard<std::mutex> lock(processMutex);
    
    // Prepare audio data
    std::vector<float> audioVec(audioData, audioData + numSamples);
    
    // Create request
    json request;
    request["audio_data"] = base64Encode(audioVec);
    request["sr"] = static_cast<int>(sampleRate);
    
    // Send request
    if (!sendMessage(request))
    {
        DBG("Failed to send message to feature extractor");
        return std::vector<float>(17, 0.0f);
    }
    
    // Receive response
    json response;
    if (!receiveMessage(response))
    {
        DBG("Failed to receive response from feature extractor");
        return std::vector<float>(17, 0.0f);
    }
    
    // Check response status
    if (response["status"] != "success")
    {
        std::string error = response.value("message", "Unknown error");
        DBG("Feature extraction failed: " << error);
        return std::vector<float>(17, 0.0f);
    }
    
    // Extract features
    std::vector<float> features = response["features"].get<std::vector<float>>();
    
    if (features.size() != 17)
    {
        DBG("Unexpected number of features: " << features.size());
        return std::vector<float>(17, 0.0f);
    }
    
    return features;
}

std::vector<float> ExternalFeatureExtractor::extractFeatures(const float* audioData, 
                                                            size_t numSamples, 
                                                            double sampleRate)
{
    return extractAllFeatures(audioData, numSamples, sampleRate);
}

float ExternalFeatureExtractor::extractSpectralCentroid(const float* audioData, 
                                                       size_t numSamples, 
                                                       double sampleRate)
{
    auto features = extractAllFeatures(audioData, numSamples, sampleRate);
    return features[0]; // spectral_centroid is first
}

float ExternalFeatureExtractor::extractSpectralBandwidth(const float* audioData, 
                                                        size_t numSamples, 
                                                        double sampleRate)
{
    auto features = extractAllFeatures(audioData, numSamples, sampleRate);
    return features[1]; // spectral_bandwidth is second
}

float ExternalFeatureExtractor::extractSpectralRolloff(const float* audioData, 
                                                      size_t numSamples, 
                                                      double sampleRate)
{
    auto features = extractAllFeatures(audioData, numSamples, sampleRate);
    return features[2]; // spectral_rolloff is third
}

std::vector<float> ExternalFeatureExtractor::extractMFCC(const float* audioData, 
                                                        size_t numSamples, 
                                                        double sampleRate, 
                                                        int numCoefficients)
{
    auto features = extractAllFeatures(audioData, numSamples, sampleRate);
    
    // MFCCs are indices 3-15 (13 coefficients)
    int actualCoeffs = std::min(numCoefficients, 13);
    return std::vector<float>(features.begin() + 3, features.begin() + 3 + actualCoeffs);
}

float ExternalFeatureExtractor::extractRMS(const float* audioData, 
                                          size_t numSamples, 
                                          double sampleRate)
{
    auto features = extractAllFeatures(audioData, numSamples, sampleRate);
    return features[16]; // rms_energy is last
}

} // namespace VTR