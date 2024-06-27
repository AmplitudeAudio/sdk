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
    indices.reserve(indicesCount);

    for (AmUInt32 i = 0; i < indicesCount; ++i)
        indices[i] = outIndices[i];

    if (debug)
    {
        convhull_3d_export_obj(
            ch_vertices.data(), static_cast<int>(ch_vertices.size()), outIndices, faceCount, false, "debug_hrir_sphere");

        log(stdout, "debug_hrir_sphere.obj written\n");
    }
}

int parseFileName_IRCAM(const AmOsString& fileName, AmVec3& position)
{
    const auto azimuth_location = fileName.find("_T");
    if (azimuth_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    const auto elevation_location = fileName.find("_P");
    if (elevation_location == AmOsString::npos)
    {
        return EXIT_FAILURE;
    }

    // azimuth in degrees 3 digits, we need 90 deg offset so that the angle is relative to z-axis,
    // - from 000 to 180 for source on your left,
    // - from 180 to 359 for source on your right
    const auto azimuth = 90.0f + static_cast<AmReal32>(std::atof(fileName.substr(azimuth_location + 2, 3).c_str()));

    // elevation in degrees, modulo 360, 3 digits,
    // - from 315 to 345 for source below your head,
    // - 0 for source in front of your head,
    // - from 015 to 090 for source above your head
    const auto elevation = static_cast<AmReal32>(std::atof(fileName.substr(elevation_location + 2, 3).c_str()));

    position = SphericalToCartesian(azimuth * AM_DegToRad, elevation * AM_DegToRad, 1.0);
    return EXIT_SUCCESS;
}

static int process_IRCAM(const std::filesystem::path& datasetPath, const std::filesystem::path& packagePath, const ProcessingState& state)
{
    DiskFile packageFile(absolute(packagePath), eFOM_WRITE);

    AmUInt32 sampleRate = 0;
    AmUInt32 irLength = 0;

    std::set<std::filesystem::path> sorted_by_name;

    std::vector<HRIRSphereVertex> vertices;
    std::vector<AmUInt32> indices;

    for (const auto& file : std::filesystem::directory_iterator(datasetPath))
    {
        if (file.is_directory())
            continue;

        sorted_by_name.insert(file);
    }

    AmUniquePtr<MemoryPoolKind::Default, Codec> wavCodec(amnew(WAVCodec));

    for (const auto& entry : sorted_by_name)
    {
        const auto& path = entry.native();

        if (state.verbose)
            log(stdout, "Processing %s.\n", path.c_str());

        AmVec3 position;
        if (parseFileName_IRCAM(path, position) == EXIT_FAILURE)
        {
            log(stderr, "Invalid file name: %s.\n", path.c_str());
            return EXIT_FAILURE;
        };

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

        if (sampleRate == 0)
            sampleRate = decoder->GetFormat().GetSampleRate();

        if (irLength == 0)
            irLength = decoder->GetFormat().GetFramesCount();

        AmAlignedReal32Buffer buffer;
        buffer.Init(decoder->GetFormat().GetFramesCount() * decoder->GetFormat().GetFrameSize());

        decoder->Load(buffer.GetBuffer());

        HRIRSphereVertex vertex;
        vertex.m_Position = position;
        vertex.m_LeftIR.resize(decoder->GetFormat().GetFramesCount());
        vertex.m_RightIR.resize(decoder->GetFormat().GetFramesCount());

        for (AmUInt32 i = 0, l = decoder->GetFormat().GetFramesCount(); i < l; ++i)
        {
            vertex.m_LeftIR[i] = buffer[i * 2 + 0];
            vertex.m_RightIR[i] = buffer[i * 2 + 1];
        }

        vertices.push_back(vertex);

        buffer.Release();
        wavCodec->DestroyDecoder(decoder);
    }

    triangulate(vertices, indices, state.debug);

    // Header
    packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>("AMIR"), 4);
    packageFile.Write16(kCurrentVersion);
    packageFile.Write32(sampleRate);
    packageFile.Write32(irLength);
    packageFile.Write32(static_cast<AmUInt32>(vertices.size()));
    packageFile.Write32(static_cast<AmUInt32>(indices.size()));

    // Indices
    packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>(indices.data()), static_cast<AmUInt32>(indices.size()) * sizeof(AmUInt32));

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

    if (state.datasetModel == eHRIRSphereDatasetModel_IRCAM)
    {
        return process_IRCAM(datasetPath, packagePath, state);
    }

    log(stderr, "Unsupported dataset model.\n");
    return EXIT_FAILURE;
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
        log(stdout, "    -[mM]:        \tThe dataset model to use.\n");
        log(stdout, "                  \tThe default value is 1. The available values are:\n");
        log(stdout, "           0:     \tIRCAN (LISTEN) dataset (http://recherche.ircam.fr/equipes/salles/listen/download.html).\n");
        log(stdout, "           1:     \tMIT (KEMAR) dataset (http://sound.media.mit.edu/resources/KEMAR.html).\n");
        log(stdout, "\n");
        log(stdout, "Example: amir -c 1 /path/to/project/ output_package.amir\n");
        log(stdout, "\n");
        // clang-format on

        return EXIT_SUCCESS;
    }

    return process(AM_STRING_TO_OS_STRING(inFileName), AM_STRING_TO_OS_STRING(outFileName), state);
}
