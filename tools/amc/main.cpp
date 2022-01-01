// Copyright (c) 2021-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "../src/Core/Codecs/AMS/Codec.h"
#include "../src/Core/Codecs/WAV/Codec.h"
#include "../src/IO/File.h"
#include "../src/Utils/Audio/Compression/ADPCM/ADPCM.h"

#define AM_FLAG_NOISE_SHAPING 0x1

using namespace SparkyStudios::Audio::Amplitude;

/**
 * @brief Defines in which mode the process should run.
 */
enum ProcessingMode
{
    ePM_UNKNOWN = 0,
    ePM_ENCODE = 1,
    ePM_DECODE = 2,
};

/**
 * @brief Stores the current process state.
 */
struct ProcessingState
{
    /**
     * @brief Defines the current processing mode, should be
     * either encode or decode.
     */
    ProcessingMode mode = ePM_UNKNOWN;

    /**
     * @brief Defines if the process is called in verbose mode.
     */
    bool verbose = false;

    /**
     * @brief The look ahead setting to use when encoding.
     */
    AmUInt32 lookAhead = 3;

    /**
     * @brief Whether to use noise shaping.
     */
    bool noiseShaping = true;

    /**
     * @brief Used to determine the encoded ADPCM block size.
     */
    AmUInt32 blockSizeShift = 0;
};

/**
 * @brief The log function, used in verbose mode.
 *
 * @param fmt The message format.
 * @param args The arguments.
 */
static void log(AmString fmt, va_list args)
{
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(stdout, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(stdout, fmt, args);
#endif
}

static int process(AmOsString inFileName, AmOsString outFileName, const ProcessingState& state)
{
    AmInt32 res = 0;

    auto* engine = Engine::GetInstance();

    if (state.mode == ePM_ENCODE)
    {
        auto* codec = Codec::FindCodecForFile(inFileName);
        if (!codec)
        {
            fprintf(stderr, "Unable to load the input file: " AM_OS_CHAR_FMT ". File not found or codec unavailable.\n", inFileName);
            return EXIT_FAILURE;
        }

        auto* decoder = codec->CreateDecoder();
        if (!decoder->Open(inFileName))
        {
            fprintf(
                stderr, "Unable to load the input file: " AM_OS_CHAR_FMT ". The found codec (%s) was not able to open the input file.\n",
                inFileName, codec->GetName());
            return EXIT_FAILURE;
        }

        SoundFormat format = decoder->GetFormat();

        AmUInt16 numChannels = format.GetNumChannels();
        AmUInt32 sampleRate = format.GetSampleRate(), blockSize, samplesPerBlock;
        AmUInt64 numSamples = format.GetFramesCount();

        auto* encoder = static_cast<Codecs::AMSCodec::AMSEncoder*>(Codecs::ams_codec.CreateEncoder());

        if (state.blockSizeShift > 0)
            blockSize = 1 << state.blockSizeShift;
        else
            blockSize = 256 * numChannels * (sampleRate < 11000 ? 1 : sampleRate / 11000);

        samplesPerBlock = (blockSize - numChannels * 4) * (numChannels ^ 3) + 1;

        encoder->SetFormat(format);
        encoder->SetEncodingParams(
            blockSize, samplesPerBlock, state.lookAhead,
            state.noiseShaping ? (sampleRate > 64000 ? Compression::ADPCM::eNSM_STATIC : Compression::ADPCM::eNSM_DYNAMIC)
                               : Compression::ADPCM::eNSM_OFF);

        if (!encoder->Open(outFileName))
        {
            fprintf(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for writing.\n", outFileName);
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            CallLogFunc("Each %d byte ADPCM block will contain %d samples * %d channels.\n", blockSize, samplesPerBlock, numChannels);
            CallLogFunc("Encoding PCM file \"" AM_OS_CHAR_FMT "\" to ADPCM file \"" AM_OS_CHAR_FMT "\"...\n", inFileName, outFileName);
        }

        auto* pcmData =
            static_cast<AmInt16Buffer>(amMemory->Malloc(MemoryPoolKind::Codec, numSamples * decoder->GetFormat().GetFrameSize()));

        if (decoder->Load(pcmData) != numSamples || !decoder->Close())
        {
            amMemory->Free(MemoryPoolKind::Codec, pcmData);
            fprintf(stderr, "Error while decoding PCM file \"" AM_OS_CHAR_FMT "\".\n", inFileName);
            return EXIT_FAILURE;
        }

        if (encoder->Write(pcmData, 0, numSamples) != numSamples || !encoder->Close())
        {
            amMemory->Free(MemoryPoolKind::Codec, pcmData);
            fprintf(stderr, "Error while encoding ADPCM file \"" AM_OS_CHAR_FMT "\".\n", outFileName);
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            CallLogFunc("Operation completed successfully.\n");
        }

        amMemory->Free(MemoryPoolKind::Codec, pcmData);

        res = EXIT_SUCCESS;
    }
    else if (state.mode == ePM_DECODE)
    {
        auto* decoder = Codecs::ams_codec.CreateDecoder();
        auto* encoder = Codecs::wav_codec.CreateEncoder();

        if (!decoder->Open(inFileName))
        {
            fprintf(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for decoding.\n", inFileName);
            return EXIT_FAILURE;
        }

        const SoundFormat& amsFormat = decoder->GetFormat();

        SoundFormat wavFormat;
        wavFormat.SetAll(
            amsFormat.GetSampleRate(),
            amsFormat.GetNumChannels(),
            16, // always decode in 16 bits per sample
            amsFormat.GetFramesCount(),
            amsFormat.GetNumChannels() * sizeof(AmInt16), // Always decode in 16 bits signed integers
            AM_SAMPLE_FORMAT_INT,
            AM_SAMPLE_INTERLEAVED
        );

        encoder->SetFormat(wavFormat);
        if (!encoder->Open(outFileName))
        {
            fprintf(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for encoding.\n", outFileName);
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            CallLogFunc("Decoding ADPCM file \"" AM_OS_CHAR_FMT "\" to PCM file \"" AM_OS_CHAR_FMT "\"...\n", inFileName, outFileName);
        }

        AmUInt64 numSamples = decoder->GetFormat().GetFramesCount();

        auto* adpcmData =
            static_cast<AmInt16Buffer>(amMemory->Malloc(MemoryPoolKind::Codec, numSamples * decoder->GetFormat().GetFrameSize()));

        if (decoder->Load(adpcmData) != numSamples || !decoder->Close())
        {
            amMemory->Free(MemoryPoolKind::Codec, adpcmData);
            fprintf(stderr, "Error while decoding ADPCM file \"" AM_OS_CHAR_FMT "\".\n", inFileName);
            return EXIT_FAILURE;
        }

        if (encoder->Write(adpcmData, 0, numSamples) != numSamples || !encoder->Close())
        {
            amMemory->Free(MemoryPoolKind::Codec, adpcmData);
            fprintf(stderr, "Error while encoding PCM file \"" AM_OS_CHAR_FMT "\".\n", outFileName);
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            CallLogFunc("Operation completed successfully.\n");
        }

        amMemory->Free(MemoryPoolKind::Codec, adpcmData);

        res = EXIT_SUCCESS;
    }
    else
    {
        fprintf(stderr, "No encode/decode mode selected. Either add -e (encode) or -d (decode). Use -h for help.\n");
        return EXIT_FAILURE;
    }

    return res;
}

int main(int argc, char* argv[])
{
    MemoryManager::Initialize(MemoryManagerConfig());

    char *inFileName = nullptr, *outFileName = nullptr;
    bool noLogo = false, needHelp = false;
    ProcessingState state;

    while (--argc)
    {
#if defined(_WIN32)
        if ((**++argv == '-' || **argv == '/') && (*argv)[1])
#else
        if ((**++argv == '-') && (*argv)[1])
#endif
        {
            while (*++*argv)
            {
                switch (**argv)
                {
                case 'H':
                case 'h':
                    needHelp = true;
                    state.verbose = true;
                    break;

                case 'O':
                case 'o':
                    noLogo = true;
                    break;

                case 'Q':
                case 'q':
                    state.verbose = false;
                    noLogo = true;
                    break;

                case 'V':
                case 'v':
                    state.verbose = true;
                    break;

                case 'C':
                case 'c':
                    state.mode = ePM_ENCODE;
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    state.lookAhead = **argv - '0';
                    break;

                case 'B':
                case 'b':
                    ++*argv;
                    state.blockSizeShift = strtol(++*argv, argv, 10);

                    if (state.blockSizeShift < 8 || state.blockSizeShift > 15)
                    {
                        fprintf(stderr, "\nblock size power must be 8 to 15!\n");
                        return EXIT_FAILURE;
                    }

                    --*argv;
                    --argc;
                    break;

                case 'F':
                case 'f':
                    state.noiseShaping = false;
                    break;

                case 'D':
                case 'd':
                    state.mode = ePM_DECODE;
                    break;

                default:
                    fprintf(stderr, "\nInvalid option: -%c. Use -h for help.\n", **argv);
                    return EXIT_FAILURE;
                }
            }
        }
        else if (!inFileName)
        {
            inFileName = static_cast<char*>(amMemory->Malloc(MemoryPoolKind::Codec, strlen(*argv) + 10));
            strcpy(inFileName, *argv);
        }
        else if (!outFileName)
        {
            outFileName = static_cast<char*>(amMemory->Malloc(MemoryPoolKind::Codec, strlen(*argv) + 10));
            strcpy(outFileName, *argv);
        }
        else
        {
            fprintf(stderr, "\nUnknown extra argument: %s !\n", *argv);
            return EXIT_FAILURE;
        }
    }

    if (state.verbose || !noLogo)
    {
        RegisterLogFunc(log);
    }

    if (!noLogo)
    {
        CallLogFunc("\n");
        CallLogFunc("Amplitude Audio Compressor\n");
        CallLogFunc("Copyright (c) 2021-present Sparky Studios - Licensed under Apache 2.0\n");
        CallLogFunc("=====================================================================\n");
        CallLogFunc("\n");
    }

    if (needHelp)
    {
        CallLogFunc("Usage: amc [OPTIONS] INPUT_FILE OUTPUT_FILE\n");
        CallLogFunc("\n");
        CallLogFunc("Global options:\n");
        CallLogFunc("    -[hH]:        \tDisplay this help message.\n");
        CallLogFunc("    -[oO]:        \tHide logo and copyright notice.\n");
        CallLogFunc("    -[qQ]:        \tQuiet mode. Shutdown all messages.\n");
        CallLogFunc("    -[vV]:        \tVerbose mode. Display all messages.\n");
        CallLogFunc("\n");
        CallLogFunc("Compression options:\n");
        CallLogFunc("    -[cC]:        \tCompress the input file into the output file.\n");
        CallLogFunc("    -[0-8]:       \tThe look ahead level.\n");
        CallLogFunc("                  \tDefaults to 3.\n");
        CallLogFunc("    -[bB] [8-15]: \tThe block size shift.\n");
        CallLogFunc("                  \tIf not defined, the block size will be calculated based on the number of channels and the sample rate.\n");
        CallLogFunc("    -[fF]:        \tDisable noise shaping. Only used for compression.\n");
        CallLogFunc("\n");
        CallLogFunc("Decompression options:\n");
        CallLogFunc("    -[dD]:        \tDecompress the input file into the output file.\n");
        CallLogFunc("\n");
        CallLogFunc("Example: amc -c -4 -b 12 input_pcm.wav output_adpcm.ams\n");
        CallLogFunc("\n");
        
        return EXIT_SUCCESS;
    }

    return process(AM_STRING_TO_OS_STRING(inFileName), AM_STRING_TO_OS_STRING(outFileName), state);
}
