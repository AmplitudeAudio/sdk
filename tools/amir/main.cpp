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

#include <cstdarg>
#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Codecs/WAV/Codec.h>
#include <Utils/Utils.h>

using namespace SparkyStudios::Audio::Amplitude;

struct ProcessingState
{
    bool verbose = false;
    bool debug = false;
    struct
    {
        bool enabled = false;
        AmUInt32 targetSampleRate = 44100;
    } resampling;
    HRIRSphereDatasetModel datasetModel = eHRIRSphereDatasetModel_IRCAM;
};

static constexpr AmUInt32 kCurrentVersion = 1;

/**
 * @brief The log function, used in verbose mode.
 *
 * @param output The output stream.
 * @param fmt The message format.
 * @param ... The arguments.
 */
static void log(FILE* output, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(output, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(output, fmt, args);
#endif
    va_end(args);
}

void triangulate(const std::vector<HRIRSphereVertex>& vertices, std::vector<AmUInt32>& indices, bool debug = false)
{
    std::vector<ch_vertex> ch_vertices;
    for (const auto& v : vertices)
    {
        ch_vertex ch_v;
        ch_v.x = v.m_Position.X;
        ch_v.y = v.m_Position.Y;
        ch_v.z = v.m_Position.Z;
        ch_vertices.push_back(ch_v);
    }

    int* outIndices = nullptr;
    int faceCount = 0;
    convhull_3d_build(ch_vertices.data(), static_cast<int>(ch_vertices.size()), &outIndices, &faceCount);

    const AmUInt32 indicesCount = faceCount * 3;

    indices.clear();
    indices.resize(indicesCount);

    for (AmUInt32 i = 0; i < indicesCount; ++i)
        indices[i] = outIndices[i];

    if (debug)
    {
        static char debugFileName[] = "debug_hrir_sphere";

        convhull_3d_export_obj(ch_vertices.data(), static_cast<int>(ch_vertices.size()), outIndices, faceCount, false, debugFileName);

        log(stdout, "debug_hrir_sphere.obj written\n");
    }
}

int parseFileName_IRCAM(const AmOsString& fileName, SphericalPosition& position)
{
    const auto azimuth_location = fileName.find(AM_OS_STRING("_T"));
    if (azimuth_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    const auto elevation_location = fileName.find(AM_OS_STRING("_P"));
    if (elevation_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    // azimuth in degrees 3 digits, we need to negate so that the angle is relative to positive z-axis
    // - from 000 to 180 for source on your left
    // - from 180 to 359 for source on your right
    const auto azimuth = -std::strtof(fileName.substr(azimuth_location + 2, 3).c_str(), nullptr);

    // elevation in degrees, modulo 360, 3 digits
    // - from 315 to 345 for source below your head
    // - 0 for source in front of your head
    // - from 015 to 090 for source above your head
    const auto elevation = std::strtof(fileName.substr(elevation_location + 2, 3).c_str(), nullptr);

    position = SphericalPosition::FromDegrees(azimuth, elevation);
    return EXIT_SUCCESS;
}

int parseFileName_MIT(const AmOsString& fileName, SphericalPosition& position)
{
    const auto azimuth_location = fileName.find('e');
    if (azimuth_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    const auto elevation_location = fileName.find('H');
    if (elevation_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    AmOsString azimuthString;
    for (AmSize az = azimuth_location + 1; fileName[az] != 'a'; ++az)
        azimuthString += fileName[az];

    AmOsString elevationString;
    for (AmSize el = elevation_location + 1; el < azimuth_location; ++el)
        elevationString += fileName[el];

    // azimuth in degrees 3 digits, we need to negate so that the angle is relative to positive z-axis
    // - from 000 to 180 for source on your left
    // - from 180 to 359 for source on your right
    const auto azimuth = -std::strtof(azimuthString.c_str(), nullptr);

    // elevation in degrees 2 digits
    // - from -15 to -40 for source below your head
    // - 0 for source in front of your head
    // - from 15 to 90 for source above your head
    const auto elevation = std::strtof(elevationString.c_str(), nullptr);

    position = SphericalPosition(azimuth * AM_DegToRad, elevation * AM_DegToRad, 1.0);
    return EXIT_SUCCESS;
}

int parseFileName_SADIE(const AmOsString& fileName, SphericalPosition& position)
{
    const auto azimuth_location = fileName.find(AM_OS_STRING("azi_"));
    if (azimuth_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    const auto elevation_location = fileName.find(AM_OS_STRING("_ele_"));
    if (elevation_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    // azimuth in degrees, we need to negate so that the angle is relative to positive z-axis
    // - from 000 to 180 for source on your left
    // - from 180 to 359 for source on your right
    const auto azimuth = std::strtof(fileName.substr(azimuth_location + 4, elevation_location - (azimuth_location + 4)).c_str(), nullptr);

    // elevation in degrees
    // - from -15 to -81 for source below your head
    // - 0 for source in front of your head
    // - from 15 to 90 for source above your head
    const auto elevation = std::strtof(fileName.substr(elevation_location + 5).c_str(), nullptr);

    position = SphericalPosition::FromDegrees(azimuth, elevation);
    return EXIT_SUCCESS;
}

static int process(const AmOsString& inFileName, const AmOsString& outFileName, const ProcessingState& state)
{
    const std::filesystem::path datasetPath(inFileName);
    const std::filesystem::path packagePath(outFileName);

    if (!exists(datasetPath))
    {
        log(stderr, "The path " AM_OS_CHAR_FMT " does not exist.\n", datasetPath.native().c_str());
        return EXIT_FAILURE;
    }

    if (!is_directory(datasetPath))
    {
        log(stderr, "The path " AM_OS_CHAR_FMT " is not a directory.\n", datasetPath.native().c_str());
        return EXIT_FAILURE;
    }

    if (state.datasetModel >= eHRIRSphereDatasetModel_Invalid)
    {
        log(stderr, "Unsupported dataset model.\n");
        return EXIT_FAILURE;
    }

    DiskFile packageFile(absolute(packagePath), eFOM_WRITE);

    AmUInt32 sampleRate = 0;
    AmUInt32 irLength = 0;

    std::set<std::filesystem::path> sorted_by_name;

    std::vector<HRIRSphereVertex> vertices;
    std::vector<AmUInt32> indices;

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

    AmUniquePtr<MemoryPoolKind::Default, Codec> wavCodec(amnew(WAVCodec));

    for (const auto& entry : sorted_by_name)
    {
        const auto& path = entry.native();

        if (state.verbose)
            log(stdout, "Processing %s.\n", path.c_str());

        SphericalPosition spherical;

        if (state.datasetModel == eHRIRSphereDatasetModel_IRCAM &&
            parseFileName_IRCAM(entry.filename().native(), spherical) == EXIT_FAILURE)
        {
            log(stderr, "Invalid file name: %s.\n", path.c_str());
            return EXIT_FAILURE;
        }

        if (state.datasetModel == eHRIRSphereDatasetModel_MIT && parseFileName_MIT(entry.filename().native(), spherical) == EXIT_FAILURE)
        {
            log(stderr, "Invalid file name: %s.\n", path.c_str());
            return EXIT_FAILURE;
        }

        if (state.datasetModel == eHRIRSphereDatasetModel_SADIE &&
            parseFileName_SADIE(entry.filename().native(), spherical) == EXIT_FAILURE)
        {
            log(stderr, "Invalid file name: %s.\n", path.c_str());
            return EXIT_FAILURE;
        }

        const AmUInt32 max = state.datasetModel == eHRIRSphereDatasetModel_MIT ? 2 : 1;
        for (AmUInt32 i = 0; i < max; ++i)
        {
            if (i == 1 && (spherical.GetAzimuth() == -AM_PI32 || spherical.GetAzimuth() == AM_PI32))
                continue; // Do not duplicate borders

            spherical.SetAzimuth(spherical.GetAzimuth() - static_cast<AmReal32>(i) * AM_PI32);
            const AmVec3 position = spherical.ToCartesian();

            auto* decoder = wavCodec->CreateDecoder();
            std::shared_ptr<File> file = std::make_shared<DiskFile>(absolute(entry));

            if (!decoder->Open(file))
            {
                log(stderr, "Failed to open file %s.\n", path.c_str());
                return EXIT_FAILURE;
            }

            if (decoder->GetFormat().GetNumChannels() != 2)
            {
                log(stderr, "Unsupported number of channels: %d. Only 2 channels is supported.\n", decoder->GetFormat().GetNumChannels());
                return EXIT_FAILURE;
            }

            AmUInt64 totalFrames = decoder->GetFormat().GetFramesCount();

            if (sampleRate == 0)
                sampleRate = decoder->GetFormat().GetSampleRate();

            if (irLength == 0)
                irLength = totalFrames;

            AudioBuffer buffer(totalFrames, 2);
            decoder->Load(&buffer);

            if (state.resampling.enabled)
            {
                auto* resampler = Resampler::Construct("default");
                resampler->Initialize(2, sampleRate, state.resampling.targetSampleRate);

                auto resampledTotalFrames = resampler->GetExpectedOutputFrames(totalFrames);
                AudioBuffer resampledBuffer(resampledTotalFrames, 2);

                resampler->Process(buffer, totalFrames, resampledBuffer, resampledTotalFrames);

                totalFrames = resampledTotalFrames;

                buffer = resampledBuffer;
                Resampler::Destruct("default", resampler);
            }

            HRIRSphereVertex vertex;
            vertex.m_Position = position;
            vertex.m_LeftIR.resize(totalFrames);
            vertex.m_RightIR.resize(totalFrames);

            const auto& leftChannel = buffer[0];
            const auto& rightChannel = buffer[1];

            for (AmUInt32 j = 0; j < totalFrames; ++j)
            {
                vertex.m_LeftIR[j] = leftChannel[j];
                vertex.m_RightIR[j] = rightChannel[j];
            }

            vertices.push_back(vertex);

            buffer.Clear();
            wavCodec->DestroyDecoder(decoder);
        }
    }

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
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(&vertex.m_Position), sizeof(AmVec3));
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(vertex.m_LeftIR.data()), irLength * sizeof(AmReal32));
        packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(vertex.m_RightIR.data()), irLength * sizeof(AmReal32));
    }

    packageFile.Close();

    if (state.verbose)
        log(stdout, "Done.\n");

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    MemoryManager::Initialize(MemoryManagerConfig());

    char *inFileName = nullptr, *outFileName = nullptr;
    bool noLogo = false, needHelp = false;
    ProcessingState state;

    for (int i = 1; i < argc; i++)
    {
#if defined(AM_WINDOWS_VERSION)
        if (*argv[i] == '-' || *argv[i] == '/')
#else
        if (*argv[i] == '-')
#endif // AM_WINDOWS_VERSION
        {
            switch (argv[i][1])
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

            case 'M':
            case 'm':
                state.datasetModel = static_cast<HRIRSphereDatasetModel>(strtol(argv[++i], argv, 10));

                if (state.datasetModel < eHRIRSphereDatasetModel_MIT || state.datasetModel >= eHRIRSphereDatasetModel_Invalid)
                {
                    log(stderr, "\nInvalid dataset model!\n");
                    return EXIT_FAILURE;
                }
                break;

            case 'D':
            case 'd':
                state.debug = true;
                break;

            case 'R':
            case 'r':
                state.resampling.enabled = true;
                state.resampling.targetSampleRate = strtol(argv[++i], argv, 10);
                break;

            default:
                log(stderr, "\nInvalid option: -%c. Use -h for help.\n", **argv);
                return EXIT_FAILURE;
            }
        }
        else if (!inFileName)
        {
            inFileName = static_cast<char*>(ampoolmalloc(MemoryPoolKind::Default, strlen(argv[i]) + 10));

#if defined(AM_WINDOWS_VERSION)
            strcpy_s(inFileName, strlen(argv[i]) + 10, *argv);
#else
            strcpy(inFileName, argv[i]);
#endif
        }
        else if (!outFileName)
        {
            outFileName = static_cast<char*>(ampoolmalloc(MemoryPoolKind::Default, strlen(argv[i]) + 10));

#if defined(AM_WINDOWS_VERSION)
            strcpy_s(outFileName, strlen(argv[i]) + 10, *argv);
#else
            strcpy(outFileName, argv[i]);
#endif
        }
        else
        {
            log(stderr, "\nUnknown extra argument: %s !\n", *argv);
            return EXIT_FAILURE;
        }
    }

    if (!inFileName || !outFileName)
    {
        needHelp = true;
    }

    if (!noLogo)
    {
        // clang-format off
        log(stdout, "\n");
        log(stdout, "Amplitude HRIR Sphere Builder (amir)\n");
        log(stdout, "Copyright (c) 2024-present Sparky Studios - Licensed under Apache 2.0\n");
        log(stdout, "=====================================================================\n");
        log(stdout, "\n");
        // clang-format on
    }

    if (needHelp)
    {
        // clang-format off
        log(stdout, "Usage: amir [OPTIONS] DATASET_DIR OUTPUT_FILE\n");
        log(stdout, "\n");
        log(stdout, "Options:\n");
        log(stdout, "    -[hH]:        \tDisplay this help message.\n");
        log(stdout, "    -[oO]:        \tHide logo and copyright notice.\n");
        log(stdout, "    -[qQ]:        \tQuiet mode. Shutdown all messages.\n");
        log(stdout, "    -[vV]:        \tVerbose mode. Display all messages.\n");
        log(stdout, "    -[dD]:        \tDebug mode. Will create an obj file with a preview of the sphere shape.\n");
        log(stdout, "    -[rR] freq:   \tResample HRIR data to the target frequency.\n");
        log(stdout, "    -[mM]:        \tThe dataset model to use.\n");
        log(stdout, "                  \tThe default value is 0. The available values are:\n");
        log(stdout, "           0:     \tIRCAN (LISTEN) dataset (http://recherche.ircam.fr/equipes/salles/listen/download.html).\n");
        log(stdout, "           1:     \tMIT (KEMAR) dataset (http://sound.media.mit.edu/resources/KEMAR.html).\n");
        log(stdout, "\n");
        log(stdout, "Example: amir -m 1 /path/to/mit/dataset/ output_package.amir\n");
        log(stdout, "\n");
        // clang-format on

        return EXIT_SUCCESS;
    }

    Engine::RegisterDefaultPlugins();

    const auto res = process(AM_STRING_TO_OS_STRING(inFileName), AM_STRING_TO_OS_STRING(outFileName), state);

    Engine::UnregisterDefaultPlugins();

    return res;
}
