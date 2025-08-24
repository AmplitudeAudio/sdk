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
#include <lz4.h>

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

    struct
    {
        /**
         * @brief The compression mode to use.
         */
        ePackageFileCompressionMode mode = ePackageFileCompressionMode_Uncompressed;

        /**
         * @brief The compression block size, in KB.
         */
        AmSize blockSize = 64;
    } compression;

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
               "-c,--compression", options.compression.mode,
               "The compression algorithm to use.\n"
               "If not defined, the resulting package will not be compressed. The available values are:\n"
               "0:  No compression.\n"
               "1:  ZLib compression.")
            ->option_text("{0,1}");

        app.add_option(
               "-s,--block-size", options.compression.blockSize, "The size of the blocks to use for compression, in KB. Default is 64KB.")
            ->default_val(64)
            ->default_str("64");

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
static constexpr char kProjectDirPipelines[] = "pipelines";
static constexpr char kProjectDirRTPC[] = "rtpc";
static constexpr char kProjectDirSoundbanks[] = "soundbanks";
static constexpr char kProjectDirSounds[] = "sounds";
static constexpr char kProjectDirSwitchContainers[] = "switch_containers";
static constexpr char kProjectDirSwitches[] = "switches";

static int compressAndWriteAsset(PackageFileItemDescription& item, std::vector<AmUInt8>& output, const DiskFile& input)
{
    LZ4_stream_t lz4Stream;
    LZ4_initStream(&lz4Stream, sizeof(lz4Stream));

    AmSize offset = 0;
    const AmSize inputSize = input.Length();

    while (offset < inputSize)
    {
        const AmSize outputSize = output.size();

        const AmSize chunkSize = std::min(inputSize - offset, item.m_CompressedBlockSize);
        AmInt32 maxDstSize = LZ4_compressBound(chunkSize);
        std::vector<AmUInt8> compressed(maxDstSize);

        std::vector<AmUInt8> data(chunkSize);
        input.Read(data.data(), chunkSize);

        AmInt32 compressedSize =
            LZ4_compress_default(reinterpret_cast<char*>(data.data()), reinterpret_cast<char*>(compressed.data()), chunkSize, maxDstSize);

        if (compressedSize <= 0)
            amLogError("LZ4 compression failed");

        item.m_CompressedChunks.push_back({
            outputSize, // Offset
            chunkSize, // Size
            static_cast<AmSize>(compressedSize) // Compressed size
        });

        output.insert(output.end(), compressed.begin(), compressed.begin() + compressedSize);
        offset += chunkSize;
    }

    return output.size();
}

static int process(const AmOsString& inFileName, const AmOsString& outFileName, const AppOptions& state)
{
    const std::filesystem::path projectPath(inFileName);
    const std::filesystem::path packagePath(outFileName);

    if (!exists(projectPath))
    {
        log(stderr, "The path " AM_OS_CHAR_FMT " does not exist.\n", projectPath.native().c_str());
        return EXIT_FAILURE;
    }

    const auto projectDirectories = { kProjectDirAttenuators, kProjectDirCollections,      kProjectDirData,    kProjectDirEffects,
                                      kProjectDirEvents,      kProjectDirPipelines,        kProjectDirRTPC,    kProjectDirSoundbanks,
                                      kProjectDirSounds,      kProjectDirSwitchContainers, kProjectDirSwitches };

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
    packageFile.Write8(state.compression.mode);

    AmSize lastOffset = 0;
    std::vector<AmUInt8> buffer;
    std::vector<PackageFileItemDescription> items;

    const auto appendItem = [&](const std::filesystem::directory_entry& file)
    {
        if (file.is_directory())
            return;

        if (state.verbose)
            log(stdout, "Adding item: " AM_OS_CHAR_FMT "\n", file.path().c_str());

        DiskFile diskFile(absolute(file));

        PackageFileItemDescription item;
        std::string relativePath = relative(absolute(file), projectPath).string();
        std::ranges::replace(relativePath, '\\', '/'); // Normalize Windows path
        item.m_Name = relativePath;
        item.m_Offset = lastOffset;
        item.m_Size = diskFile.Length();

        if (state.compression.mode == ePackageFileCompressionMode_Uncompressed)
        {
            item.m_CompressedBlockSize = 0;

            buffer.resize(lastOffset + item.m_Size, 0);
            diskFile.Read(buffer.data() + lastOffset, item.m_Size);

            items.push_back(item);
            lastOffset += item.m_Size;
        }
        else if (state.compression.mode == ePackageFileCompressionMode_Compressed)
        {
            item.m_CompressedBlockSize = state.compression.blockSize * 1024;

            std::vector<AmUInt8> compressedData;
            auto compressedSize = compressAndWriteAsset(item, compressedData, diskFile);

            buffer.insert(buffer.end(), compressedData.begin(), compressedData.begin() + compressedSize);

            items.push_back(item);
            lastOffset += compressedSize;
        }
    };

    for (const auto& directory : projectDirectories)
        for (const auto& file : std::filesystem::recursive_directory_iterator(projectPath / directory))
            appendItem(file);

    for (const auto& file : std::filesystem::directory_iterator(projectPath))
        appendItem(file);

    if (state.verbose)
        log(stdout, "Writing package file: " AM_OS_CHAR_FMT "\n", packagePath.c_str());

    packageFile.Write64(items.size());

    for (const auto& item : items)
    {
        packageFile.WriteString(item.m_Name);
        packageFile.Write64(item.m_Offset);
        packageFile.Write64(item.m_Size);
        packageFile.Write64(item.m_CompressedBlockSize);
        packageFile.Write64(item.m_CompressedChunks.size());

        for (const auto& chunk : item.m_CompressedChunks)
        {
            packageFile.Write64(chunk.m_Offset);
            packageFile.Write64(chunk.m_Size);
            packageFile.Write64(chunk.m_CompressedSize);
        }
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
