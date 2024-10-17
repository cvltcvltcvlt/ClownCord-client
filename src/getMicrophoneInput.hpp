#ifndef GET_MICROPHONE_INPUT_HPP
#define GET_MICROPHONE_INPUT_HPP

#include <portaudio.h>
#include <atomic>
#include <thread>
#include <opus/opus.h>
#include "connection.hpp"
#include "renderGui.hpp"
#include <condition_variable>

extern std::atomic<bool> joinedVC;

// Define the data structure for audio recording
struct paTestData {
    float* recordedSamples;
    int frameIndex;
    int maxFrameIndex;
    std::atomic<bool> isRecording;
    OpusEncoder* opusEncoder;

    paTestData() {
        isRecording = true;
    }
};

// Function declarations
int setupAndStartRecording(const char* outputFileName);
int recordCallback(const void* inputBuffer, 
    void* outputBuffer, 
    unsigned long framesPerBuffer, 
    const PaStreamCallbackTimeInfo* timeInfo, 
    PaStreamCallbackFlags statusFlags, 
    void* userData);
void writeWavFile(const char* fileName, 
    const paTestData& data, 
    int sampleRate);

// Exposed functions for starting/stopping voice chat
void startVoiceChat();  // Start recording and connection
void stopVoiceChat();   // Stop recording, connection, and threads

void startDecoding();

#endif
