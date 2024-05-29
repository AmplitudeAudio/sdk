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

#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

struct ProcessingState
{
    bool verbose = false;

    PackageFileCompressionAlgorithm compression = ePCA_None;
};

static constexpr AmUInt32 kCurrentVersion = 1;

static constexpr char kProjectDirAttenuators[] = "attenuators";
static constexpr char kProjectDirCollections[] = "collections";
static constexpr char kProjectDirData[] = "data";
static constexpr char kProjectDirEffects[] = "effects";
static constexpr char kProjectDirEvents[] = "events";
static constexpr char kProjectDirRTPC[] = "rtpc";
static constexpr char kProjectDirSoundbanks[] = "soundbanks";
static constexpr char kProjectDirSounds[] = "sounds";
static constexpr char kProjectDirSwitchContainers[] = "switch_containers";
static constexpr char kProjectDirSwitches[] = "switches";

/**
 * @brief The log function, used in verbose mode.
 *
 * @param fmt The message format.
 * @param args The arguments.
 */
static void log(const char* fmt, va_list args)
{
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(stdout, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(stdout, fmt, args);
#endif
}

static int process(const AmOsString& inFileName, const AmOsString& outFileName, const ProcessingState& state)
{
    const std::filesystem::path projectPath(inFileName);
    const std::filesystem::path packagePath(outFileName);

    if (!exists(projectPath))
    {
        fprintf(stderr, "The path " AM_OS_CHAR_FMT " does not exist.\n", projectPath.string().c_str());
        return EXIT_FAILURE;
    }

    const auto projectDirectories = { kProjectDirAttenuators,      kProjectDirCollections, kProjectDirData,       kProjectDirEffects,
                                      kProjectDirEvents,           kProjectDirRTPC,        kProjectDirSoundbanks, kProjectDirSounds,
                                      kProjectDirSwitchContainers, kProjectDirSwitches };

    for (const auto& directory : projectDirectories)
    {
        if (!exists(projectPath / directory) || !is_directory(projectPath / directory))
        {
            fprintf(stderr, "Invalid project path. The \"attenuators\" directory is missing.\n");
            return EXIT_FAILURE;
        }
    }

    DiskFile packageFile(absolute(packagePath), eFOM_WRITE);

    packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>("AMPK"), 4);
    packageFile.Write16(kCurrentVersion);
    packageFile.Write8(state.compression);

    AmSize lastOffset = 0;
    std::vector<AmUInt8> buffer;
    std::vector<PackageFileItemDescription> items;

    const auto appendItem = [&](const std::filesystem::path& file)
    {
        DiskFile diskFile(absolute(file));

        PackageFileItemDescription item;
        item.m_Name = relative(absolute(file), projectPath);
        item.m_Offset = lastOffset;
        item.m_Size = diskFile.Length();

        buffer.resize(lastOffset + item.m_Size, 0);
        diskFile.Read(buffer.data() + lastOffset, item.m_Size);

        items.push_back(item);
        lastOffset += item.m_Size;
    };

    for (const auto& directory : projectDirectories)
    {
        for (const auto& file : std::filesystem::recursive_directory_iterator(projectPath / directory))
        {
            if (file.is_directory())
                continue;

            appendItem(file);
        }
    }

    for (const auto& file : std::filesystem::directory_iterator(projectPath))
    {
        if (file.is_directory())
            continue;

        appendItem(file);
    }

    packageFile.Write64(items.size());

    for (const auto& item : items)
    {
        packageFile.WriteString(item.m_Name);
        packageFile.Write64(item.m_Offset);
        packageFile.Write64(item.m_Size);
    }

    packageFile.Write(buffer.data(), buffer.size());

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

            case 'C':
            case 'c':
                state.compression = static_cast<PackageFileCompressionAlgorithm>(strtol(argv[++i], argv, 10));

                if (state.compression < ePCA_None || state.compression >= ePCA_Invalid)
                {
                    fprintf(stderr, "\nInvalid compression algorithm!\n");
                    return EXIT_FAILURE;
                }
                break;

            default:
                fprintf(stderr, "\nInvalid option: -%c. Use -h for help.\n", **argv);
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
            fprintf(stderr, "\nUnknown extra argument: %s !\n", *argv);
            return EXIT_FAILURE;
        }
    }

    if (!inFileName || !outFileName)
    {
        needHelp = true;
    }

    if (state.verbose || !noLogo || needHelp)
    {
        RegisterLogFunc(log);
    }

    if (!noLogo)
    {
        // clang-format off
        CallLogFunc("\n");
        CallLogFunc("Amplitude Packager (ampk)\n");
        CallLogFunc("Copyright (c) 2024-present Sparky Studios - Licensed under Apache 2.0\n");
        CallLogFunc("=====================================================================\n");
        CallLogFunc("\n");
        // clang-format on
    }

    if (needHelp)
    {
        // clang-format off
        CallLogFunc("Usage: ampk [OPTIONS] PROJECT_DIR OUTPUT_FILE\n");
        CallLogFunc("\n");
        CallLogFunc("Options:\n");
        CallLogFunc("    -[hH]:        \tDisplay this help message.\n");
        CallLogFunc("    -[oO]:        \tHide logo and copyright notice.\n");
        CallLogFunc("    -[qQ]:        \tQuiet mode. Shutdown all messages.\n");
        CallLogFunc("    -[vV]:        \tVerbose mode. Display all messages.\n");
        CallLogFunc("    -[cC]:        \tThe compression algorithm to use.\n");
        CallLogFunc("                  \tIf not defined, the resulting package will not be compressed. The available values are:\n");
        CallLogFunc("           0:     \tNo compression.\n");
        CallLogFunc("           1:     \tZLib compression.\n");
        CallLogFunc("\n");
        CallLogFunc("Example: ampk -c 1 /path/to/project/ output_package.ampk\n");
        CallLogFunc("\n");
        // clang-format on

        return EXIT_SUCCESS;
    }

    return process(AM_STRING_TO_OS_STRING(inFileName), AM_STRING_TO_OS_STRING(outFileName), state);
}
