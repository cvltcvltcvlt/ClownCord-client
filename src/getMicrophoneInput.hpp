#ifndef GET_MICROPHONE_INPUT_HPP
#define GET_MICROPHONE_INPUT_HPP

#include <portaudio.h>
#include <atomic>  // For using atomic

struct paTestData {
    float* recordedSamples;
    int frameIndex;
    int maxFrameIndex;
    std::atomic<bool> isRecording;

    paTestData()
    {
        isRecording = true;
    }
};

int setupAndStartRecording(const char* outputFileName);
int recordCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData);
void writeWavFile(const char* fileName, const paTestData& data, int sampleRate);

#endif
