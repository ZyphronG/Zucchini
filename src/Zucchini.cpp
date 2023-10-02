#include <ios>
#include <cstring>
#include <unistd.h>
#include "include/BinaryReader.h"
#include "include/afctool.h"
#include "include/wavfile.h"
#include "include/asttool.h"

void printUsage() {
    printf("Usage:\n \
    Zucchini enc <in.raw> <out.adpcm> [loop_start] [loop_end]    - Encodes 16-bit PCM WAV to ADPCM4\n \
    Zucchini dec <in.adpcm> <out.raw>                            - Decodes ADPCM4 data to raw 16-bit PCM little endian\n \
    Zucchini ast <in.wav> <out.ast> [loop_start] [loop_end]      - Encodes 16-bit PCM WAV to ADPCM4 AST\n");

    sleep(10);
}

int main(int argv, char* argc[]) {
    s32 action = 0;
    const char* pInFile = 0;
    const char* pOutFile = 0;

    // drag'n'drop
    if (argv == 2) {
        pInFile = argc[1];

        if (!strcasecmp(pInFile + strlen(pInFile) - 4, ".wav")) {
            pOutFile = "out.ast";
            action = 3;
        }
        else if (!strcasecmp(pInFile + strlen(pInFile) - 6, ".adpcm") || !strcasecmp(pInFile + strlen(pInFile) - 4, ".adp")) {
            pOutFile = "out.raw";
            action = 2;
        }
        else {
            printUsage();
            return 1;
        }
    }
    else if (argv != 4 && argv != 6) {
        printUsage();
        return 1;
    }
    else {
        pInFile = argc[2];
        pOutFile = argc[3];

        if (!strcmp(argc[1], "enc"))
            action = 1;
        else if (!strcmp(argc[1], "dec"))
            action = 2;
        else if (!strcmp(argc[1], "ast"))
            action = 3;
        else {
            printUsage();
            return 1;
        }
    }

    if (action == 1) {
        WAV::WAV_INFO wav(pInFile);
        FileHolder out_file(pOutFile);
        AFC::ADPCM_INFO adpcm;

        if (!wav.loadFile()) {
            printf("Could not open input file!\n");
            sleep(5);
            return 1;
        }

        printf("Input file size: %llu\n", wav.mFileSize);

        if (!wav.parseFile()) {
            sleep(5);
            return 1;
        }

        s16** wavData = wav.makeWaveDataBuffers();

        if (!wavData) {
            sleep(5);
            return 1;
        }

        adpcm.setInputPCMData(wavData[0], wav.getNumSamples());

        if (argv == 6)
            adpcm.setLoopData(strtoul(argc[4], 0, 10), strtoul(argc[5], 0, 10));
        
        u8* buffer = adpcm.createADPCMBuffer();

        adpcm.encode(buffer);
        out_file.mFileData = (char*)buffer;
        out_file.mFileSize = adpcm.getADPCMBufferSize();
        
        if (!out_file.writeFile()) {
            printf("Could not write output file!\n");
            sleep(5);
            return 1;
        }

        if (argv == 6)
            printf("Last: %d, Penult: %d\n", adpcm.getLoopLast(), adpcm.getLoopPenult());
    }
    else if (action == 2) {
        BinaryReader reader(pInFile);
        FileHolder out_file(pOutFile);
        AFC::ADPCM_INFO adpcm;

        if (!reader.loadFile()) {
            printf("Could not open input file!\n");
            sleep(5);
            return 1;
        }

        printf("Input file size: %llu\n", reader.mFileSize);

        if (!adpcm.setInputADPCMData(reader, reader.mFileSize)) {
            sleep(5);
            return 1;
        }
        
        s16* buffer = adpcm.createPCMBuffer();

        printf("Number of samples: %d\n", adpcm.getNumSamples());

        adpcm.decode(buffer);
        out_file.mFileData = (char*)buffer;
        out_file.mFileSize = adpcm.getPCMBufferSize();
        
        if (!out_file.writeFile()) {
            printf("Could not write output file!\n");
            sleep(5);
            return 1;
        }
    }
    else if (action == 3) {
        WAV::WAV_INFO wav(pInFile);
        BinaryReader out_file(pOutFile);
        s16** WaveData = 0;

        if (!wav.loadFile()) {
            printf("Could not open input file!\n");
            sleep(5);
            return 1;
        }

        if (!wav.parseFile()) {
            sleep(5);
            return 1;
        }

        if (wav.getNumChannels() > 6) {
            printf("The WAV file has too many channels! An AST file can only hold up to 6,\n");
            sleep(5);
            return 1;
        }

        WaveData = wav.makeWaveDataBuffers();

        if (!WaveData) {
            sleep(5);
            return 1;
        }

        bool isLooped = (argv == 2); // check drag'n'drop
        u32 loopStart = 0;
        u32 loopEnd = wav.getNumSamples();

        if (argv == 6) {
            isLooped = true;
            loopStart = strtoul(argc[4], 0, 10);
            loopEnd = strtoul(argc[5], 0, 10);
        }

        if (!AST::makeAst(WaveData, out_file, wav.getNumSamples(), wav.getNumChannels(), wav.getSamplerate(), isLooped, loopStart, loopEnd)) {
            sleep(5);
            return 1;
        }
        
        if (!out_file.writeFile()) {
            printf("Could not write output file!\n");
            sleep(5);
            return 1;
        }
    }

    printf("Done!\n");
    sleep(10);

    return 0;
}