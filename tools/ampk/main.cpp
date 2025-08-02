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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <CLI/CLI.hpp>

#include <cli_formatter.h>
#include <utils.h>

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
     * @brief The compression algorithm to use.
     */
    ePackageFileCompressionAlgorithm compression = ePackageFileCompressionAlgorithm_None;

    /**
     * @brief The path to the input project directory to process.
     */
    std::string inputFile;

    /**
     * @brief The path to the output package file to create.
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
    CLI::App app{ "Amplitude Packager", "ampk" };

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
            ->usage("Usage: ampk [OPTIONS] PROJECT_DIR OUTPUT_FILE")
            ->footer("ampk -c 1 /path/to/project/ output_package.ampk");

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
            ->default_str("false");

        app.add_option(
               "-c,--compression", options.compression,
               "The compression algorithm to use.\nIf not defined, the resulting package will not be compressed. The available values "
               "are:\n0:\tNo compression.\n1:\tZLib compression.")
            ->option_text("{0,1}");

        app.add_option("PROJECT_DIR", options.inputFile, "The path to the project directory to process.")
            ->required()
            ->transform(CLI::ExistingPath);

        app.add_option("OUTPUT_FILE", options.outputFile, "The path to the output package file to create.")->required();

        CLI11_PARSE(app, argc, argv);

        if (!options.noLogo)
        {
            log(stdout, formatter->make_description(&app).c_str());
            log(stdout, "\n");
        }

        exitCode = process(AM_STRING_TO_OS_STRING(options.inputFile), AM_STRING_TO_OS_STRING(options.outputFile), options);

        MemoryManager::Deinitialize();

        return exitCode;
    }
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

static int process(const AmOsString& inFileName, const AmOsString& outFileName, const AppOptions& state)
{
    const std::filesystem::path projectPath(inFileName);
    const std::filesystem::path packagePath(outFileName);

    if (!exists(projectPath))
    {
        log(stderr, "The path " AM_OS_CHAR_FMT " does not exist.\n", projectPath.native().c_str());
        return EXIT_FAILURE;
    }

    const auto projectDirectories = { kProjectDirAttenuators,      kProjectDirCollections, kProjectDirData,       kProjectDirEffects,
                                      kProjectDirEvents,           kProjectDirRTPC,        kProjectDirSoundbanks, kProjectDirSounds,
                                      kProjectDirSwitchContainers, kProjectDirSwitches };

    for (const auto& directory : projectDirectories)
    {
        if (!exists(projectPath / directory) || !is_directory(projectPath / directory))
        {
            log(stderr, "Invalid project path. The \"%s\" directory is missing.\n", directory);
            return EXIT_FAILURE;
        }
    }

    if (state.verbose)
        log(stdout, "Processing project directory: " AM_OS_CHAR_FMT "\n", projectPath.c_str());

    DiskFile packageFile(absolute(packagePath), eFileOpenMode_Write);

    packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>("AMPK"), 4);
    packageFile.Write16(kCurrentVersion);
    packageFile.Write8(ePackageFileCompressionAlgorithm_None); // TODO: state.compression

    AmSize lastOffset = 0;
    std::vector<AmUInt8> buffer;
    std::vector<PackageFileItemDescription> items;

    const auto appendItem = [&](const std::filesystem::path& file)
    {
        if (state.verbose)
            log(stdout, "Adding item: " AM_OS_CHAR_FMT "\n", file.c_str());

        DiskFile diskFile(absolute(file));

        PackageFileItemDescription item;
        std::string relativePath = relative(absolute(file), projectPath).string();
        std::ranges::replace(relativePath, '\\', '/');
        item.m_Name = relativePath;
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

    if (state.verbose)
        log(stdout, "Writing package file: " AM_OS_CHAR_FMT "\n", packagePath.c_str());

    packageFile.Write64(items.size());

    for (const auto& item : items)
    {
        packageFile.WriteString(item.m_Name);
        packageFile.Write64(item.m_Offset);
        packageFile.Write64(item.m_Size);
    }

    packageFile.Write(buffer.data(), buffer.size());

    if (state.verbose)
        log(stdout, "Package file created successfully.\n");

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    AppContext app;
    return app.run(argc, argv);
}
