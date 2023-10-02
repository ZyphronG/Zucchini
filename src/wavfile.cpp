// version 1.0

#include <ios>
#include "include/wavfile.h"


bool WAV::WAV_INFO::parseFile() {
    mCurrentPos = 0;

    // parse RIFF
    if (readU32BE() != 'RIFF' || readU32LE() == 0 || readU32BE() != 'WAVE') {
        printf("Input file is not a valid WAV file!\n");
        return false;
    }

    // parse WAVE format
    if (readU32BE() != 'fmt ') {
        printf("Input file is not a valid WAV file!\n");
        return false;
    }

    u32 nextChunkPos = readU32LE() + mCurrentPos; // read format chunk size

    if (readU16LE() != 1) {
        printf("Input WAV is not PCM!\n");
        return false;
    }

    mChannelCount = readU16LE();
    mSampleRate = readU32LE();
    readU32LE(); // Byte rate, what do i need this for?
    
    readU16LE(); // check for align block skipped, because some exporters don't get it right?
    /*if (readU16LE() != mChannelCount * 2) {
        printf("Input WAV is invalid!\n");
        return false;
    }*/

    if (readU16LE() != 16) {
        printf("Input WAV is not 16-bit PCM!\n");
        return false;
    }

    mCurrentPos = nextChunkPos;

    // read data chunk
    if (readU32BE() != 'data') {
        printf("Input WAV is missing wave data!\n");
        return false;
    }

    mNumSamples = readU32LE(); // chunk size

    if (mNumSamples % (2 * mChannelCount) != 0)  {
        printf("Input WAV is invalid!\n");
        return false;
    }

    mNumSamples /= 2 * mChannelCount;
    
    return true;
}

s16** WAV::WAV_INFO::makeWaveDataBuffers() {
    if (mChannelCount == 0) {
        printf("No channels found in WAV file!\n");
        return 0;
    }

    s16** out = new s16 * [mChannelCount];

    for (u32 i = 0; i < mChannelCount; i++)
        out[i] = new s16 [mNumSamples];

    u32 pos = mCurrentPos;

    // split and copy wave data
    for (u32 i = 0; i < mNumSamples; i++) {
        for (u32 c = 0; c < mChannelCount; c++)
                out[c][i] = readS16LE();
    }

    mCurrentPos = pos;

    return out;
}

u32 WAV::WAV_INFO::getWaveDataSize() const {
    return mNumSamples * mChannelCount * 2;
}

u32 WAV::WAV_INFO::getNumSamples() const {
    return mNumSamples;
}

u32 WAV::WAV_INFO::getNumChannels() const {
    return mChannelCount;
}

u32 WAV::WAV_INFO::getSamplerate() const {
    return mSampleRate;
}