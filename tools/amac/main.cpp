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

#include <cstdlib>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <CLI/CLI.hpp>

#include <cli_formatter.h>
#include <utils.h>

#include <Core/Codecs/AMS/Codec.h>
#include <Core/Codecs/MP3/Codec.h>
#include <Core/Codecs/WAV/Codec.h>

#define AM_FLAG_NOISE_SHAPING 0x1

using namespace SparkyStudios::Audio::Amplitude;

struct AppOptions;
static int process(const AmOsString& inFileName, const AmOsString& outFileName, const AppOptions& state);

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
 * @brief Stores the application options passed via CLI.
 */
struct AppOptions
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
     * @brief Defines if the process should not display the logo.
     */
    bool noLogo = false;

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

    /**
     * @brief Configures the resampler for the encoded ADPCM file.
     */
    struct
    {
        bool enabled = false;
        AmUInt32 targetSampleRate = 44100;
    } resampling;

    /**
     * @brief The path to the input file to process.
     */
    std::string inputFile;

    /**
     * @brief The path to the output file to create.
     */
    std::string outputFile;
};

/**
 * @brief The application context.
 */
struct AppContext
{
    /**
     * @brief The command line interface application.
     */
    CLI::App app{ "Amplitude Audio Compressor", "amac" };

    /**
     * @brief The processing state.
     */
    AppOptions options;

    /**
     * @brief The exit code generated during processing.
     */
    int exitCode = 0;

    int run(int argc, char** argv)
    {
        MemoryManager::Initialize();

        const auto formatter = std::make_shared<AmplitudeToolCLIFormatter>();

        app.set_version_flag("--version", "1.0.0");

        app.formatter(formatter)
            ->usage("Usage: amac [OPTIONS] INPUT_FILE OUTPUT_FILE")
            ->footer("amac -e -4 -b 12 input_pcm.wav output_adpcm.ams");

        app.add_flag("-l,--no-logo", options.noLogo, "Hide logo and copyright notice.")
            ->default_val(false)
            ->default_str("false")
            ->group("Global");

        app.add_flag("-v,--verbose", options.verbose, "Verbose mode. Display all messages")
            ->default_val(false)
            ->default_str("false")
            ->group("Global");

        app.add_flag_function(
               "-q,--quiet",
               [this](bool value)
               {
                   if (!value)
                       return;

                   options.verbose = false;
                   options.noLogo = true;
               },
               "Quiet mode. Shutdown all messages.")
            ->default_val(false)
            ->default_str("false")
            ->group("Global");

        auto encodeFlag = app.add_flag_function(
                                 "-e,--encode",
                                 [this](bool value)
                                 {
                                     if (!value)
                                         return;

                                     options.mode = ePM_ENCODE;
                                 },
                                 "Compress the input file into the output file.")
                              ->default_val(false)
                              ->group("Encode");

        app.add_flag_function(
               "-d,--decode",
               [this](bool value)
               {
                   if (!value)
                       return;

                   options.mode = ePM_DECODE;
               },
               "Decompress the input file into the output file.")
            ->default_val(false)
            ->excludes(encodeFlag)
            ->group("Decode");

        app.add_flag("-0{0},-1{1},-2{2},-3{3},-4{4},-5{5},-6{6},-7{7},-8{8}", options.lookAhead, "The look ahead level.")
            ->default_val(3)
            ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast)
            ->needs(encodeFlag)
            ->group("Encode");

        app.add_option(
               "-b,--block-size-shift", options.blockSizeShift,
               "The block size shift. If not defined, the block size will be calculated based on the number of channels and the sample "
               "rate.")
            ->default_val(0)
            ->transform(CLI::Range(8, 15))
            ->needs(encodeFlag)
            ->option_text("frequency in [8 - 15]")
            ->group("Encode");

        app.add_flag("!-f", options.noiseShaping, "Disable noise shaping. Only used for compression.")
            ->default_val(true)
            ->needs(encodeFlag)
            ->group("Encode");

        app.add_option_function<AmUInt32>(
               "-r,--resample",
               [this](const AmUInt32& value)
               {
                   options.resampling.enabled = true;
                   options.resampling.targetSampleRate = value;
               },
               "Resamples input data to the target frequency.")
            ->transform(CLI::Range(8000, 384000))
            ->needs(encodeFlag)
            ->option_text("frequency in [8000 - 384000]")
            ->group("Encode");

        app.add_option("INPUT_FILE", options.inputFile, "The path to the input file to process.")->required()->transform(CLI::ExistingFile);

        app.add_option("OUTPUT_FILE", options.outputFile, "The path to the output file to create.")->required();

        CLI11_PARSE(app, argc, argv);

        if (!options.noLogo)
        {
            log(stdout, formatter->make_description(&app).c_str());
            log(stdout, "\n");
        }

        Engine::RegisterDefaultExtensions();

        exitCode = process(AM_STRING_TO_OS_STRING(options.inputFile), AM_STRING_TO_OS_STRING(options.outputFile), options);

        Engine::UnregisterDefaultExtensions();

        MemoryManager::Deinitialize();

        return exitCode;
    }
};

static int process(const AmOsString& inFileName, const AmOsString& outFileName, const AppOptions& state)
{
    AmInt32 res;
    DiskFileSystem fs;

    const auto inputFile = fs.OpenFile(inFileName, eFileOpenMode_Read);

    auto ams_codec = Codec::Find("ams");
    auto wav_codec = Codec::Find("wav");

    if (state.mode == ePM_ENCODE)
    {
        const auto outputFile = fs.OpenFile(outFileName, eFileOpenMode_Write);

        auto codec = Codec::FindForFile(inputFile);
        if (!codec)
        {
            log(stderr, "Unable to load the input file: " AM_OS_CHAR_FMT ". File not found or codec unavailable.\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        auto decoder = codec->CreateDecoder();
        if (!decoder->Open(inputFile))
        {
            log(stderr, "Unable to load the input file: " AM_OS_CHAR_FMT ". The found codec (%s) was not able to open the input file.\n",
                inFileName.c_str(), codec->GetName().c_str());
            return EXIT_FAILURE;
        }

        const SoundFormat format = decoder->GetFormat();

        AmUInt16 numChannels = format.GetNumChannels();
        AmUInt32 sampleRate = format.GetSampleRate(), blockSize;
        AmUInt64 numSamples = format.GetFramesCount();
        AmUInt64 framesSize = format.GetFrameSize();

        auto encoder = std::dynamic_pointer_cast<AMSCodec::AMSEncoder>(ams_codec->CreateEncoder());

        if (state.blockSizeShift > 0)
            blockSize = 1 << state.blockSizeShift;
        else
            blockSize = 256 * numChannels * (sampleRate < 11000 ? 1 : sampleRate / 11000);

        const AmUInt32 samplesPerBlock = (blockSize - numChannels * 4) * (numChannels ^ 3) + 1;

        if (state.verbose)
        {
            log(stdout, "Each %d byte ADPCM block will contain %d samples * %d channels.\n", blockSize, samplesPerBlock, numChannels);
            log(stdout, "Encoding PCM file \"" AM_OS_CHAR_FMT "\" to ADPCM file \"" AM_OS_CHAR_FMT "\"...\n", inFileName.c_str(),
                outFileName.c_str());
        }

        AudioBuffer pcmData(numSamples, numChannels);
        if (decoder->Load(&pcmData) != numSamples || !decoder->Close())
        {
            log(stderr, "Error while decoding PCM file \"" AM_OS_CHAR_FMT "\".\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        encoder->SetEncodingParams(
            samplesPerBlock, state.lookAhead,
            state.noiseShaping ? (sampleRate > 64000 ? Compression::ADPCM::eNSM_STATIC : Compression::ADPCM::eNSM_DYNAMIC)
                               : Compression::ADPCM::eNSM_OFF);

        if (state.resampling.enabled)
        {
            if (state.verbose)
                log(stdout, "Resampling input data from %d Hz to %d Hz...\n", sampleRate, state.resampling.targetSampleRate);

            auto resampler = Resampler::Construct("default");
            resampler->Initialize(numChannels, sampleRate, state.resampling.targetSampleRate);

            AmUInt64 f = resampler->GetExpectedOutputFrames(numSamples);
            AudioBuffer output(f, numChannels);

            resampler->Process(pcmData, numSamples, output, f);

            sampleRate = state.resampling.targetSampleRate;
            numSamples = f;

            pcmData = output;

            if (state.verbose)
                log(stdout, "Resampling completed.\n");
        }

        SoundFormat encodeFormat{};
        encodeFormat.SetAll(sampleRate, numChannels, format.GetBitsPerSample(), numSamples, framesSize, eAudioSampleFormat_Int16);

        encoder->SetFormat(encodeFormat);
        if (!encoder->Open(outputFile))
        {
            log(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for writing.\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (encoder->Write(&pcmData, 0, numSamples) != numSamples || !encoder->Close())
        {
            log(stderr, "Error while encoding ADPCM file \"" AM_OS_CHAR_FMT "\".\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            log(stdout, "Operation completed successfully.\n");
        }

        res = EXIT_SUCCESS;
    }
    else if (state.mode == ePM_DECODE)
    {
        const auto outputFile = fs.OpenFile(outFileName, eFileOpenMode_Write);

        auto decoder = ams_codec->CreateDecoder();
        auto encoder = wav_codec->CreateEncoder();

        if (!decoder->Open(inputFile))
        {
            log(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for decoding.\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        const SoundFormat& amsFormat = decoder->GetFormat();

        SoundFormat wavFormat{};
        wavFormat.SetAll(
            amsFormat.GetSampleRate(), amsFormat.GetNumChannels(),
            16, // always decode in 16 bits per sample
            amsFormat.GetFramesCount(),
            amsFormat.GetNumChannels() * sizeof(AmInt16), // Always decode in 16 bits signed integers
            eAudioSampleFormat_Int16);

        encoder->SetFormat(wavFormat);
        if (!encoder->Open(outputFile))
        {
            log(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for encoding.\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            log(stdout, "Decoding ADPCM file \"" AM_OS_CHAR_FMT "\" to PCM file \"" AM_OS_CHAR_FMT "\"...\n", inFileName.c_str(),
                outFileName.c_str());
        }

        AmUInt64 numSamples = amsFormat.GetFramesCount();
        AmUInt64 numChannels = amsFormat.GetNumChannels();

        AudioBuffer adpcmData(numSamples, numChannels);

        if (decoder->Load(&adpcmData) != numSamples || !decoder->Close())
        {
            log(stderr, "Error while decoding ADPCM file \"" AM_OS_CHAR_FMT "\".\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        if (encoder->Write(&adpcmData, 0, numSamples) != numSamples || !encoder->Close())
        {
            log(stderr, "Error while encoding PCM file \"" AM_OS_CHAR_FMT "\".\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            log(stdout, "Operation completed successfully.\n");
        }

        res = EXIT_SUCCESS;
    }
    else
    {
        log(stderr, "No encode/decode mode selected. Either add -e (encode) or -d (decode). Use -h for help.\n");
        return EXIT_FAILURE;
    }

    return res;
}

int main(int argc, char* argv[])
{
    AppContext app;
    return app.run(argc, argv);
}
