// version 1.0

#pragma once

#include "include/BinaryReader.h"

namespace WAV {
    class WAV_INFO : public BinaryReader {
    private:
        u32 mWaveDataSize = 0;
        u32 mSampleRate = 0;
        u32 mChannelCount = 0;
        u32 mNumSamples = 0;

    public:
        WAV_INFO(const char* pFile) : BinaryReader(pFile) {}

        bool parseFile();
        s16** makeWaveDataBuffers();
        u32 getWaveDataSize() const;
        u32 getNumSamples() const;
        u32 getNumChannels() const;
        u32 getSamplerate() const;
    };
}