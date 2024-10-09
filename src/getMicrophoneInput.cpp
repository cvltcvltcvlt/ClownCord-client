#include "getMicrophoneInput.hpp"
#include <iostream>
#include <fstream>
#include <portaudio.h>


int sequenceNumber = 0;

// Record callback function
int recordCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {

    paTestData* data = (paTestData*)userData;
    const float* in = (const float*)inputBuffer;

    if (data->isRecording) {
        if (inputBuffer == nullptr) {
            for (unsigned long i = 0; i < framesPerBuffer; i++) {
                data->recordedSamples[data->frameIndex++] = 0.0f;
            }
        }
        else {
            for (unsigned long i = 0; i < framesPerBuffer; i++) {
                data->recordedSamples[data->frameIndex++] = *in++;
            }
        }

        if (data->frameIndex >= data->maxFrameIndex) {
            data->frameIndex = 0;
        }
    }
    if (!data->isRecording) {
        return paComplete;
    }

    return paContinue;
}



// Function to write recorded data to WAV file
void writeWavFile(const char* fileName, const paTestData& data, int sampleRate) {
    std::ofstream outFile(fileName, std::ios::binary);

    outFile.write("RIFF", 4);
    int chunkSize = 36 + data.frameIndex * sizeof(float);
    outFile.write((const char*)&chunkSize, 4);
    outFile.write("WAVE", 4);
    outFile.write("fmt ", 4);

    int subChunk1Size = 16;
    short audioFormat = 3;
    short numChannels = 1;
    outFile.write((const char*)&subChunk1Size, 4);
    outFile.write((const char*)&audioFormat, 2);
    outFile.write((const char*)&numChannels, 2);
    outFile.write((const char*)&sampleRate, 4);

    int byteRate = sampleRate * sizeof(float) * numChannels;
    short blockAlign = sizeof(float) * numChannels;
    short bitsPerSample = 8 * sizeof(float);
    outFile.write((const char*)&byteRate, 4);
    outFile.write((const char*)&blockAlign, 2);
    outFile.write((const char*)&bitsPerSample, 2);

    outFile.write("data", 4);
    int subChunk2Size = data.frameIndex * sizeof(float);
    outFile.write((const char*)&subChunk2Size, 4);
    outFile.write((const char*)data.recordedSamples, data.frameIndex * sizeof(float));
    outFile.close();

    std::cout << "Audio saved to " << fileName << std::endl;
}

// Function to initialize and start recording audio
int setupAndStartRecording(const char* outputFileName)
{
    PaStreamParameters inputParameters;
    PaStream* stream;
    PaError err;
    paTestData data;

    int sampleRate = 44100;
    int numSamples = sampleRate * 10;  // 10-second buffer
    data.maxFrameIndex = numSamples;
    data.frameIndex = 0;
    data.recordedSamples = new float[numSamples];
    data.isRecording = true;  // Set recording flag

    std::cout << "Initializing PortAudio..." << std::endl;
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization error: " << Pa_GetErrorText(err) << std::endl;
        return err;
    }
    std::cout << "PortAudio initialized successfully." << std::endl;

    // Check for available devices
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "Error getting device count: " << Pa_GetErrorText(numDevices) << std::endl;
        return numDevices;
    }
    std::cout << "Number of devices: " << numDevices << std::endl;

    // List all devices
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        std::cout << "Device #" << i << ": " << deviceInfo->name << std::endl;
    }

    // Set up input parameters
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        std::cerr << "Error: No default input device." << std::endl;
        return paNoDevice;
    }
    inputParameters.channelCount = 1;    // Mono input
    inputParameters.sampleFormat = paFloat32; // 32 bit floating point output
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    // Open stream
    err = Pa_OpenStream(&stream, &inputParameters, nullptr, sampleRate, paFramesPerBufferUnspecified, paClipOff, recordCallback, &data);
    if (err != paNoError) {
        std::cerr << "Error opening stream: " << Pa_GetErrorText(err) << std::endl;
        return err;
    }

    std::cout << "Starting recording..." << std::endl;
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Error starting stream: " << Pa_GetErrorText(err) << std::endl;
        return err;
    }

    // Wait for the recording to finish
    while (data.isRecording) {
        Pa_Sleep(100);
    }

    // Stop the stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        std::cerr << "Error stopping stream: " << Pa_GetErrorText(err) << std::endl;
    }
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::cerr << "Error closing stream: " << Pa_GetErrorText(err) << std::endl;
    }

    // Write the recorded data to a WAV file
    writeWavFile(outputFileName, data, sampleRate);

    // Cleanup
    delete[] data.recordedSamples;
    Pa_Terminate();

    std::cout << "Recording finished." << std::endl;
    return err;
}
