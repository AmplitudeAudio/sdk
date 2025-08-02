// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#define CONVHULL_3D_ENABLE
#include "convhull_3d.h"

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <CLI/CLI.hpp>
#include <mysofa.h>

#include <cli_formatter.h>
#include <utils.h>

#include <Core/Codecs/WAV/Codec.h>
#include <DSP/Filters/BiquadResonantFilter.h>
#include <Utils/Utils.h>

using namespace SparkyStudios::Audio::Amplitude;

struct AppOptions;
static int process(const AmOsString& inFileName, const AmOsString& outFileName, const AppOptions& state);

/**
 * @brief Stores the application options passed via CLI.
 */
struct AppOptions
{
    /**
     * @brief Defines if the process is called in verbose mode.
     */
    bool verbose = false;

    /**
     * @brief Defines if the process should not display the logo.
     */
    bool noLogo = false;

    /**
     * @brief Defines if the process is called in debug mode.
     *
     * This will output a 3D visualization mesh of the HRIR sphere, in OBJ format.
     */
    bool debug = false;

    /**
     * @brief Configures the resampler for the IR file.
     */
    struct
    {
        bool enabled = false;
        AmUInt32 targetSampleRate = 44100;
    } resampling;

    /**
     * @brief Defines the model of the HRIR dataset.
     */
    eHRIRSphereDatasetModel datasetModel = eHRIRSphereDatasetModel_IRCAM;

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
    CLI::App app{ "Amplitude HRIR Sphere Builder", "amir" };

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

        app.set_version_flag("--version", "1.0.0");

        const auto formatter = std::make_shared<AmplitudeToolCLIFormatter>();

        app.formatter(formatter)
            ->usage("Usage: amir [OPTIONS] DATASET_DIR OUTPUT_FILE")
            ->footer("amir -m 3 /path/to/mit/file.sofa output_asset.amir");

        app.add_flag("-l,--no-logo", options.noLogo, "Hide logo and copyright notice.")->default_val(false)->default_str("false");

        app.add_flag("-v,--verbose", options.verbose, "Verbose mode. Display all messages")->default_val(false)->default_str("false");

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
            ->capture_default_str();

        app.add_option(
               "-m,--model", options.datasetModel,
               "The dataset model to use.\nThe available values are:\n0:\tIRCAM (LISTEN) dataset "
               "(http://recherche.ircam.fr/equipes/salles/listen/download.html).\n1:\tMIT (KEMAR) dataset "
               "(http://sound.media.mit.edu/resources/KEMAR.html).\n2:\tSADIE II dataset "
               "(https://www.york.ac.uk/sadie-project/database.html).\n3:\tSOFA file (https://www.sofaconventions.org).\n")
            ->option_text("{0,1,2,3}")
            ->required();

        app.add_option_function<AmUInt32>(
               "-r,--resample",
               [this](const AmUInt32& value)
               {
                   options.resampling.enabled = true;
                   options.resampling.targetSampleRate = value;
               },
               "Resamples input data to the target frequency.")
            ->option_text("frequency");

        app.add_flag("-d,--debug", options.debug, "Debug mode. Will create an obj file with a preview of the sphere shape.");

        app.add_option("DATASET_DIR", options.inputFile, "The path to the dataset file or directory to process.")
            ->required()
            ->transform(CLI::ExistingPath);

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

/**
 * @brief Defines the version of the generated AMIR file.
 */
static constexpr AmUInt32 kCurrentVersion = 1;

void cxcorr(AmReal32* a, AmReal32* b, AmReal32* x_ab, AmSize la, AmSize lb)
{
    AmInt32 m, n, negFLAG, arg, len, lim;

    len = static_cast<AmInt32>(la + lb) - 1;
    std::memset(x_ab, 0, len * sizeof(AmReal32));

    for (m = 1; m <= len; m++)
    {
        arg = m - static_cast<AmInt32>(la);
        if (arg < 0)
        {
            negFLAG = 1;
            lim = static_cast<AmInt32>(la) + arg;
        }
        else
        {
            negFLAG = 0;
            lim = static_cast<AmInt32>(la) - arg;
        }
        for (n = 1; n <= lim; n++)
        {
            if (negFLAG == 0)
                x_ab[m - 1] += (a[arg + n - 1] * b[n - 1]);
            else
                x_ab[m - 1] += (a[n - 1] * b[n - arg - 1]);
        }
    }
}

// Estimates the Inter-aural Time Difference (ITD) between the left and right channels of the HRIR sphere.
// The implementation is inspired by the following code:
// https://github.com/leomccormack/Spatial_Audio_Framework/blob/018e06e86ccdbb37cc527ca511a3a26576126b71/framework/modules/saf_hrir/saf_hrir.c#L40
void estimateITD(HRIRSphereVertex& vertex, AmSize irLength, AmUInt32 sampleRate)
{
    constexpr AmReal32 kFC = 750.0f;
    constexpr AmReal32 kQ = 0.7071f;

    BiquadResonantFilter lpfFilter;
    lpfFilter.InitializeLowPass(kFC, kQ);

    const AmReal32 maxITD = std::sqrt(2.0f) / 2e3f;

    const AmReal32 correlationLength = 2.0f * irLength - 1;

    AmAlignedReal32Buffer correlation;
    AudioBuffer hrirLPF(irLength, kAmStereoChannelCount);

    correlation.Init(correlationLength);

    AudioBuffer hrir(irLength, kAmStereoChannelCount);
    std::memcpy(hrir[0].begin(), vertex.m_LeftIR.data(), irLength * sizeof(AmReal32));
    std::memcpy(hrir[1].begin(), vertex.m_RightIR.data(), irLength * sizeof(AmReal32));

    // Apply LPF
    {
        auto lpfInstance = lpfFilter.CreateInstance();
        lpfInstance->Process(hrir, hrirLPF, irLength, sampleRate);
    }

    // xcorr between L and R
    cxcorr(hrirLPF[0].begin(), hrirLPF[1].begin(), correlation.GetBuffer(), irLength, irLength);

    AmReal32 maxVal = 0.0f;
    AmUInt32 maxIdx = 0;

    for (AmUInt32 j = 0; j < correlationLength; ++j)
    {
        if (correlation[j] > maxVal)
        {
            maxIdx = j;
            maxVal = correlation[j];
        }
    }

    AmReal32 itd = (static_cast<AmReal32>(irLength) - static_cast<AmReal32>(maxIdx) - 1.0f) / static_cast<AmReal32>(sampleRate);
    itd = AM_CLAMP(itd, -maxITD, maxITD);

    vertex.m_LeftDelay = itd < 0.0f ? -itd : 0.0f;
    vertex.m_RightDelay = itd > 0.0f ? itd : 0.0f;
}

void triangulate(const std::vector<HRIRSphereVertex>& vertices, std::vector<AmUInt32>& indices, bool debug = false)
{
    std::vector<ch_vertex> ch_vertices;
    for (const auto& v : vertices)
    {
        ch_vertex ch_v;
        ch_v.x = v.m_Position.x;
        ch_v.y = v.m_Position.y;
        ch_v.z = v.m_Position.z;
        ch_vertices.push_back(ch_v);
    }

    int* outIndices = nullptr;
    int faceCount = 0;
    convhull_3d_build(ch_vertices.data(), static_cast<int>(ch_vertices.size()), &outIndices, &faceCount);

    const AmUInt32 indicesCount = faceCount * 3;

    indices.clear();
    indices.resize(indicesCount);

    std::memcpy(indices.data(), outIndices, indicesCount * sizeof(AmUInt32));

    if (debug)
    {
        static char debugFileName[] = "debug_hrir_sphere";

        convhull_3d_export_obj(ch_vertices.data(), static_cast<int>(ch_vertices.size()), outIndices, faceCount, false, debugFileName);

        log(stdout, "debug_hrir_sphere.obj written\n");
    }

    free(outIndices);
}

int parseFileName_IRCAM(const AmOsString& fileName, SphericalPosition& position)
{
    const auto azimuth_location = fileName.find(AM_OS_STRING("_T"));
    if (azimuth_location == AmOsString::npos)
        return EXIT_FAILURE;

    const auto elevation_location = fileName.find(AM_OS_STRING("_P"));
    if (elevation_location == AmOsString::npos)
        return EXIT_FAILURE;

    // azimuth in degrees 3 digits, we need to negate so that the angle is relative to positive y-axis
    // - from 000 to 180 for source on your left
    // - from 180 to 359 for source on your right
    const auto azimuth = -std::strtof(AM_OS_STRING_TO_STRING(fileName.substr(azimuth_location + 2, 3)), nullptr);

    // elevation in degrees, modulo 360, 3 digits
    // - from 315 to 345 for source below your head
    // - 0 for source in front of your head
    // - from 015 to 090 for source above your head
    const auto elevation = std::strtof(AM_OS_STRING_TO_STRING(fileName.substr(elevation_location + 2, 3)), nullptr);

    position = SphericalPosition::FromDegrees(azimuth, elevation);
    return EXIT_SUCCESS;
}

int parseFileName_MIT(const AmOsString& fileName, SphericalPosition& position)
{
    const auto azimuth_location = fileName.find('e');
    if (azimuth_location == AmOsString::npos)
        return EXIT_FAILURE;

    const auto elevation_location = fileName.find('H');
    if (elevation_location == AmOsString::npos)
        return EXIT_FAILURE;

    AmOsString azimuthString;
    for (AmSize az = azimuth_location + 1; fileName[az] != 'a'; ++az)
        azimuthString += fileName[az];

    AmOsString elevationString;
    for (AmSize el = elevation_location + 1; el < azimuth_location; ++el)
        elevationString += fileName[el];

    // azimuth in degrees 3 digits
    // - from 000 to 180 for source on your right
    // - from 180 to 359 for source on your left
    const auto azimuth = std::strtof(AM_OS_STRING_TO_STRING(azimuthString), nullptr);

    // elevation in degrees 2 digits
    // - from -15 to -40 for source below your head
    // - 0 for source in front of your head
    // - from 15 to 90 for source above your head
    const auto elevation = std::strtof(AM_OS_STRING_TO_STRING(elevationString), nullptr);

    position = SphericalPosition::FromDegrees(azimuth, elevation);
    return EXIT_SUCCESS;
}

int parseFileName_SADIE(const AmOsString& fileName, SphericalPosition& position)
{
    const auto azimuth_location = fileName.find(AM_OS_STRING("azi_"));
    if (azimuth_location == AmOsString::npos)
        return EXIT_FAILURE;

    const auto elevation_location = fileName.find(AM_OS_STRING("_ele_"));
    if (elevation_location == AmOsString::npos)
        return EXIT_FAILURE;

    // azimuth in degrees, we need to negate so that the angle is relative to positive y-axis
    // - from 000 to 180 for source on your left
    // - from 180 to 359 for source on your right
    auto azimuthStr = fileName.substr(azimuth_location + 4, elevation_location - (azimuth_location + 4));
    std::replace(azimuthStr.begin(), azimuthStr.end(), ',', '.');
    const auto azimuth = -std::strtof(AM_OS_STRING_TO_STRING(azimuthStr), nullptr);

    // elevation in degrees
    // - from -15 to -90 for source below your head
    // - 0 for source in front of your head
    // - from 15 to 90 for source above your head
    auto elevationStr = fileName.substr(elevation_location + 5);
    std::replace(elevationStr.begin(), elevationStr.end(), ',', '.');
    const auto elevation = std::strtof(AM_OS_STRING_TO_STRING(elevationStr), nullptr);

    position = SphericalPosition::FromDegrees(azimuth, elevation);
    return EXIT_SUCCESS;
}

void processVertex(
    const AudioBuffer& buffer, const AmVector3& position, AmUInt32 irLength, AmReal32 sampleRate, bool mirror, HRIRSphereVertex& vertex)
{
    vertex.m_Position = position;
    vertex.m_LeftIR.resize(irLength);
    vertex.m_RightIR.resize(irLength);

    const auto& leftChannel = buffer[0];
    const auto& rightChannel = buffer[1];

    std::memcpy(vertex.m_LeftIR.data(), !mirror ? leftChannel.begin() : rightChannel.begin(), irLength * sizeof(AmReal32));
    std::memcpy(vertex.m_RightIR.data(), !mirror ? rightChannel.begin() : leftChannel.begin(), irLength * sizeof(AmReal32));
}

void resampleIR(const AppOptions& state, AudioBuffer& buffer, AmUInt32& sampleRate, AmUInt64& irLength)
{
    if (!state.resampling.enabled)
        return;

    auto resampler = Resampler::Construct("default");
    resampler->Initialize(2, sampleRate, state.resampling.targetSampleRate);

    auto resampledTotalFrames = resampler->GetExpectedOutputFrames(irLength);
    AudioBuffer resampledBuffer(resampledTotalFrames, 2);

    resampler->Process(buffer, irLength, resampledBuffer, resampledTotalFrames);

    irLength = resampledTotalFrames;
    sampleRate = state.resampling.targetSampleRate;

    buffer = resampledBuffer;
}

int process(const AmOsString& inFileName, const AmOsString& outFileName, const AppOptions& state)
{
    const std::filesystem::path datasetPath(inFileName);
    const std::filesystem::path packagePath(outFileName);

    if (!exists(datasetPath))
    {
        log(stderr, "The path " AM_OS_CHAR_FMT " does not exist.\n", datasetPath.native().c_str());
        return EXIT_FAILURE;
    }

    if (state.datasetModel >= eHRIRSphereDatasetModel_Invalid)
    {
        log(stderr, "Unsupported dataset model.\n");
        return EXIT_FAILURE;
    }

    AmUInt32 sampleRate = 0;
    AmUInt64 irLength = 0;

    std::vector<HRIRSphereVertex> vertices;
    std::vector<AmUInt32> indices;

    DiskFile packageFile(absolute(packagePath), eFileOpenMode_Write);

    if (state.datasetModel != eHRIRSphereDatasetModel_SOFA)
    {
        if (!is_directory(datasetPath))
        {
            log(stderr, "The path " AM_OS_CHAR_FMT " is not a directory.\n", datasetPath.native().c_str());
            return EXIT_FAILURE;
        }

        std::set<std::filesystem::path> sorted_by_name;

        for (const auto& file : std::filesystem::recursive_directory_iterator(datasetPath))
        {
            if (file.is_directory())
                continue;

            // Avoid known bad files
            {
                if (file.path().filename() == AM_OS_STRING(".DS_Store"))
                    continue;
            }

            sorted_by_name.insert(file);
        }

        AmUniquePtr<Codec> wavCodec(amnew(WAVCodec));

        std::vector<AmVector3> positions;

        for (const auto& entry : sorted_by_name)
        {
            const auto& path = entry.native();

            if (state.verbose)
                log(stdout, "Processing %s.\n", path.c_str());

            SphericalPosition spherical;

            if (state.datasetModel == eHRIRSphereDatasetModel_IRCAM &&
                parseFileName_IRCAM(entry.filename().native(), spherical) == EXIT_FAILURE)
            {
                log(stderr, "\tInvalid file name: %s.\n", path.c_str());
                return EXIT_FAILURE;
            }

            if (state.datasetModel == eHRIRSphereDatasetModel_MIT &&
                parseFileName_MIT(entry.filename().native(), spherical) == EXIT_FAILURE)
            {
                log(stderr, "\tInvalid file name: %s.\n", path.c_str());
                return EXIT_FAILURE;
            }

            if (state.datasetModel == eHRIRSphereDatasetModel_SADIE &&
                parseFileName_SADIE(entry.filename().native(), spherical) == EXIT_FAILURE)
            {
                log(stderr, "\tInvalid file name: %s.\n", path.c_str());
                return EXIT_FAILURE;
            }

            auto decoder = wavCodec->CreateDecoder();

            if (auto file = AmSharedPtr<DiskFile, eMemoryPoolKind_IO>::Make(absolute(entry)); !decoder->Open(file))
            {
                log(stderr, "\tFailed to open file %s.\n", path.c_str());
                return EXIT_FAILURE;
            }

            if (decoder->GetFormat().GetNumChannels() != 2)
            {
                log(stderr, "\tUnsupported number of channels: %d. Only 2 channels is supported.\n", decoder->GetFormat().GetNumChannels());
                return EXIT_FAILURE;
            }

            AmUInt64 totalFrames = decoder->GetFormat().GetFramesCount();

            if (sampleRate == 0)
                sampleRate = decoder->GetFormat().GetSampleRate();

            if (irLength == 0)
                irLength = totalFrames;

            AudioBuffer buffer(totalFrames, 2);
            decoder->Load(&buffer);

            resampleIR(state, buffer, sampleRate, irLength);

            const AmUInt32 max = state.datasetModel == eHRIRSphereDatasetModel_MIT ? 2 : 1;
            for (AmUInt32 i = 0; i < max; ++i)
            {
                spherical.SetAzimuth(spherical.GetAzimuth() * (i * -2.0f + 1.0f));
                const AmVector3 position = spherical.ToCartesian();

                if (const auto& it = std::find(positions.begin(), positions.end(), position); it != positions.end())
                    continue; // Do not duplicate borders

                positions.push_back(position);

                HRIRSphereVertex vertex;
                processVertex(buffer, position, irLength, sampleRate, i != 0, vertex);
                estimateITD(vertex, irLength, sampleRate);

                vertices.push_back(vertex);

                if (state.verbose)
                    log(stdout, "\tProcessed %s -> {%f, %f, %f}.\n", path.c_str(), vertex.m_Position.x, vertex.m_Position.y,
                        vertex.m_Position.z);
            }

            buffer.Clear();
        }
    }
    else
    {
        AmInt32 err = 0;
        MYSOFA_HRTF* hrtf = nullptr;
        MYSOFA_ATTRIBUTE* tmp_a = nullptr;

        AmString kAttributeType = "Type";

        hrtf = mysofa_load(datasetPath.string().c_str(), &err);

        switch (err)
        {
        case MYSOFA_OK:
            {
                if (state.resampling.enabled)
                    mysofa_resample(hrtf, state.resampling.targetSampleRate);

                vertices.reserve(hrtf->M);

                irLength = hrtf->N;
                sampleRate = hrtf->DataSamplingRate.values[0];

                if (hrtf->R != 2)
                {
                    log(stderr, "Unsupported number of channels: %d. Only 2 channels is supported.\n", hrtf->R);
                    return EXIT_FAILURE;
                }

                const AmUInt32 bufferSize = hrtf->N * hrtf->R;

                const AmVector3 listenerForward = { hrtf->ListenerView.values[0], hrtf->ListenerView.values[1],
                                                    hrtf->ListenerView.values[2] };
                const AmVector3 listenerUp = { hrtf->ListenerUp.values[0], hrtf->ListenerUp.values[1], hrtf->ListenerUp.values[2] };

                AudioBuffer buffer(hrtf->N, hrtf->R);

                for (AmUInt32 i = 0; i < hrtf->M; ++i)
                {
                    std::memcpy(buffer.GetData().GetBuffer(), hrtf->DataIR.values + i * bufferSize, bufferSize * sizeof(AmReal32));

                    const AmString type = mysofa_getAttribute(hrtf->SourcePosition.attributes, kAttributeType.data());

                    AmReal32* rawPosition = hrtf->SourcePosition.values + i * 3;

                    if (type == "spherical")
                        mysofa_s2c(rawPosition);

                    AmVector3 position = { rawPosition[0], rawPosition[1], rawPosition[2] };

                    HRIRSphereVertex vertex;
                    processVertex(buffer, position, irLength, sampleRate, false, vertex);
                    estimateITD(vertex, irLength, sampleRate);

                    vertices.push_back(vertex);

                    buffer.Clear();

                    if (state.verbose)
                        log(stdout, "Processed SOFA measurement %u -> {%f, %f, %f}.\n", i, rawPosition[0], rawPosition[1], rawPosition[2]);
                }
                break;
            }
        }

        mysofa_free(hrtf);
    }

    if (state.verbose)
        log(stdout, "Building mesh...\n");

    triangulate(vertices, indices, state.debug);

    // Header
    packageFile.Write8('A');
    packageFile.Write8('M');
    packageFile.Write8('I');
    packageFile.Write8('R');
    packageFile.Write16(kCurrentVersion);
    packageFile.Write32(sampleRate);
    packageFile.Write32(irLength);
    packageFile.Write32(static_cast<AmUInt32>(vertices.size()));
    packageFile.Write32(static_cast<AmUInt32>(indices.size()));

    // Indices
    packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(indices.data()), indices.size() * sizeof(AmUInt32));

    // Vertices
    for (const auto& vertex : vertices)
    {
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(&vertex.m_Position), sizeof(AmVector3));
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(vertex.m_LeftIR.data()), irLength * sizeof(AmReal32));
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(vertex.m_RightIR.data()), irLength * sizeof(AmReal32));
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(&vertex.m_LeftDelay), sizeof(AmReal32));
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(&vertex.m_RightDelay), sizeof(AmReal32));
    }

    packageFile.Close();

    if (state.verbose)
        log(stdout, "Done.\n");

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    AppContext app;
    return app.run(argc, argv);
}
