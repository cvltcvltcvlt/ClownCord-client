#include <iostream>
#include <thread>
#include <Windows.h>
#include <opus/opus.h>

struct AudioPacket {
    int sequenceNumber;
    unsigned long dataSize;
    float audioData[1024];
};

void encodeSound();