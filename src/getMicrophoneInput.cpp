#include "getMicrophoneInput.hpp"
#include <iostream>
#include <fstream>
#include <portaudio.h>
#include <opus/opus.h>
#include <queue>
#include "connection.hpp"

const int OPUS_FRAME_SIZE = 960;
const int MAX_PACKET_SIZE = 4000;

std::thread recordingThread;
std::thread audioThread;
std::thread connectionThread;
std::atomic<bool> stopRecording(false);
std::atomic<bool> stopAudio(false);

bool connectionSuccessfull = true;

void startRecording();
void startConnection();

void startDecoding()
{
    //todo
}

void startVoiceChat() {
    if (joinedVC) {
        std::cout << "Attempting to start VC..." << std::endl;
        stopRecording = false;
        stopAudio = false;

        // Start audio decoding thread
        audioThread = std::thread(startDecoding);

        // Start connection thread
        connectionThread = std::thread(startConnection);
        connectionThread.detach();  // Detach the connection thread

        // Check connection status and start recording
        if (joinedVC) {
            recordingThread = std::thread(startRecording);  // Start recording in its own thread
            // Do not detach the recording thread
            std::cout << "Joined VC and started recording!" << std::endl;
        }
        else {
            std::cout << "Connection failed. Cannot start recording." << std::endl;
            stopVoiceChat();  // Stop everything if the connection fails
        }
    }
}

void startRecording() {
    setupAndStartRecording("output.wav");  // This should check stopRecording to exit gracefully
}

void stopVoiceChat() {
    stopRecording = true; // Остановите запись и другие потоки, если это необходимо
    stopAudio = true;

    if (recordingThread.joinable()) {
        recordingThread.join();  // Завершаем поток записи
    }
    if (audioThread.joinable()) {
        audioThread.join();  // Завершаем поток декодирования
    }

    closeConnection();  // Закрываем соединение
    joinedVC = false;
    std::cout << "Voice chat stopped." << std::endl;
}

// Record callback to process audio input and encode it using Opus
int recordCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {

    paTestData* data = (paTestData*)userData;
    const float* in = (const float*)inputBuffer;

    if (data->isRecording) {
        // Handle case where there's no input data (inputBuffer is NULL)
        if (inputBuffer == nullptr) {
            for (unsigned long i = 0; i < framesPerBuffer; i++) {
                data->recordedSamples[data->frameIndex++] = 0.0f; // Silence
            }
        }
        else {
            for (unsigned long i = 0; i < framesPerBuffer; i++) {
                data->recordedSamples[data->frameIndex++] = *in++; // Copy input
            }
        }

        // Once buffer is full, encode the buffer into Opus packets
        if (data->frameIndex >= OPUS_FRAME_SIZE) {
            unsigned char opusPacket[MAX_PACKET_SIZE]; // Buffer to hold the encoded packet
            int encodedBytes = opus_encode_float(data->opusEncoder, data->recordedSamples, OPUS_FRAME_SIZE, opusPacket, MAX_PACKET_SIZE);

            if (encodedBytes < 0) {
                std::cerr << "Opus encoding error: " << opus_strerror(encodedBytes) << std::endl;
            }
            else {
                std::cout << "Encoded Opus packet size: " << encodedBytes << " bytes" << std::endl;
                // Here you can save or transmit the Opus packet
                // For example, write it to a file or send it over the network
            }

            data->frameIndex = 0;  // Reset the frame index after encoding
        }
    }

    if (!data->isRecording) {
        return paComplete;  // Stop stream if recording has ended
    }

    return paContinue;  // Continue streaming
}

    // Function to set up and start recording, initializes PortAudio and Opus
int setupAndStartRecording(const char* outputFileName)
{
    PaStreamParameters inputParameters;
    PaStream* stream;
    PaError err;
    paTestData data;

    int sampleRate = 48000; // Opus recommends 48kHz for optimal quality
    data.maxFrameIndex = OPUS_FRAME_SIZE; // We will encode frames of 20ms
    data.frameIndex = 0;
    data.recordedSamples = new float[OPUS_FRAME_SIZE];  // Allocate buffer for Opus encoding

    // Initialize Opus encoder
    int opusError;
    data.opusEncoder = opus_encoder_create(sampleRate, 1, OPUS_APPLICATION_AUDIO, &opusError);
    if (opusError != OPUS_OK) {
        std::cerr << "Failed to create Opus encoder: " << opus_strerror(opusError) << std::endl;
        return opusError;
    }

    // Initialize PortAudio
    std::cout << "Initializing PortAudio..." << std::endl;
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization error: " << Pa_GetErrorText(err) << std::endl;
        return err;
    }
    std::cout << "PortAudio initialized successfully." << std::endl;

    // Get and print device info
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "Error getting device count: " << Pa_GetErrorText(numDevices) << std::endl;
        return numDevices;
    }
    std::cout << "Number of devices: " << numDevices << std::endl;
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        std::cout << "Device #" << i << ": " << deviceInfo->name << std::endl;
    }

    // Set up the input device parameters
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        std::cerr << "Error: No default input device." << std::endl;
        return paNoDevice;
    }
    inputParameters.channelCount = 1; // Mono input
    inputParameters.sampleFormat = paFloat32; // 32-bit floating point
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

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


    while (data.isRecording) {
        Pa_Sleep(100);
        if (!joinedVC)
        {
            Pa_StopStream(stream);
            Pa_CloseStream(stream);
            Pa_Terminate();
            opus_encoder_destroy(data.opusEncoder);
            std::cout << "Disconnected from Voice Chat!";
            break;
        }
    }

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        std::cerr << "Error stopping stream: " << Pa_GetErrorText(err) << std::endl;
    }
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::cerr << "Error closing stream: " << Pa_GetErrorText(err) << std::endl;
    }

    opus_encoder_destroy(data.opusEncoder);
    delete[] data.recordedSamples;
    Pa_Terminate();

    std::cout << "Recording finished." << std::endl;
    return err;
}

void writeWavFile(const char* fileName, const paTestData& data, int sampleRate)
{
    std::ofstream outFile(fileName, std::ios::binary);

    // Write WAV file header
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

std::queue<std::vector<unsigned char>> opusPacketQueue;

// Function to decode Opus packets and play them back
void decodeAndPlayOpusStream() {
    PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;

    int sampleRate = 48000;
    const int numChannels = 1;
    int opusError;

    // Create an Opus decoder
    OpusDecoder* opusDecoder = opus_decoder_create(sampleRate, numChannels, &opusError);
    if (opusError != OPUS_OK) {
        std::cerr << "Failed to create Opus decoder: " << opus_strerror(opusError) << std::endl;
        return;
    }

    // Initialize PortAudio for output
    std::cout << "Initializing PortAudio for playback..." << std::endl;
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization error: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    // Set up the output device parameters
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "Error: No default output device." << std::endl;
        return;
    }
    outputParameters.channelCount = numChannels;
    outputParameters.sampleFormat = paFloat32;  // 32-bit float output
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    // Open audio stream for output
    err = Pa_OpenStream(&stream, nullptr, &outputParameters, sampleRate, paFramesPerBufferUnspecified, paClipOff, nullptr, nullptr);
    if (err != paNoError) {
        std::cerr << "Error opening stream: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    // Start the audio stream for playback
    std::cout << "Starting playback..." << std::endl;
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Error starting stream: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    // Continuous loop to decode Opus packets and play them back
    float decodedAudio[960 * numChannels]; // Buffer to hold decoded PCM audio
    for (;;) {
        // Check if there are packets in the queue
        if (!opusPacketQueue.empty()) {
            std::vector<unsigned char> packet = opusPacketQueue.front();
            opusPacketQueue.pop(); // Remove the packet from the queue

            // Decode the packet into PCM samples
            int decodedSamples = opus_decode_float(opusDecoder, packet.data(), packet.size(), decodedAudio, 960, 0);
            if (decodedSamples < 0) {
                std::cerr << "Opus decoding error: " << opus_strerror(decodedSamples) << std::endl;
            }
            else {
                // Write the decoded PCM audio to the output stream
                err = Pa_WriteStream(stream, decodedAudio, decodedSamples);
                if (err != paNoError) {
                    std::cerr << "Error writing to stream: " << Pa_GetErrorText(err) << std::endl;
                }
            }
        }
        else {
            Pa_Sleep(10); // Sleep to prevent busy-waiting when no packets are available
        }
    }

    // Cleanup and close the stream after exiting loop (in case of manual exit)
    opus_decoder_destroy(opusDecoder);
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "Playback finished." << std::endl;
}