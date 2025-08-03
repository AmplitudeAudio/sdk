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
#include <iostream>
#include <random>

#include <cli_formatter.h>
#include <utils.h>

#include "gen.resources.h"

using namespace SparkyStudios::Audio::Amplitude;

static constexpr char kProjectPathAttenuators[] = "attenuators";
static constexpr char kProjectPathCollections[] = "collections";
static constexpr char kProjectPathEffects[] = "effects";
static constexpr char kProjectPathEvents[] = "events";
static constexpr char kProjectPathPipelines[] = "pipelines";
static constexpr char kProjectPathRTPC[] = "rtpc";
static constexpr char kProjectPathSoundBanks[] = "soundbanks";
static constexpr char kProjectPathSounds[] = "sounds";
static constexpr char kProjectPathSwitchContainers[] = "switch_containers";
static constexpr char kProjectPathSwitches[] = "switches";

static constexpr uint32_t kProjectVersion = 1;

static std::string SnakeCase(const std::string& str)
{
    std::string result;
    result += static_cast<char>(std::tolower(str[0]));

    for (size_t i = 1, l = str.length(); i < l; i++)
    {
        if (const char ch = str[i]; std::isupper(ch))
        {
            result += '_';
            result += static_cast<char>(std::tolower(ch));
        }
        else if (ch == '-' || ch == ' ')
        {
            result += '_';
        }
        else
        {
            result += ch;
        }
    }

    return result;
}

struct InitProjectOptions
{
    AmString name;
    AmString templateName = "empty";
    AmString directory;
};

struct CreateSourceOptions
{
    AmObjectID id = std::random_device{}();
    AmString name;
    AmReal32 gain = 1.0f;
    AmReal32 pitch = 1.0f;
    AmBusID bus = 1;
    AmReal32 priority = 1.0f;
    bool stream = false;
    AmString scope = "World";
    AmString spatialization = "None";
    AmEffectID effect = 0;
    AmAttenuationID attenuation = 0;
    AmString fader = "Linear";
    AmString path;
};

struct ImportSourceOptions : public CreateSourceOptions
{
    AmString fileName;
};

struct AppContext
{
    std::filesystem::path exeDirectory;

    CLI::App mainApp;

    CLI::App* projectApp{ nullptr };
    CLI::App* projectNewApp{ nullptr };

    CLI::App* assetsApp{ nullptr };
    CLI::App* assetsImportApp{ nullptr };

    InitProjectOptions initProjectOptions;
    CreateSourceOptions createSourceOptions;
    ImportSourceOptions importSourceOptions;

    static void fillCommonSourceOptions(CLI::App* app, CreateSourceOptions& options)
    {
        app->add_option("--id", options.id, "Source ID.");
        app->add_option("--name", options.id, "Source name.")->required();
        app->add_option("--gain", options.gain, "Gain value.")->default_val(1.0f);
        app->add_option("--pitch", options.pitch, "Pitch value.")->default_val(1.0f);
        app->add_option("--bus", options.bus, "Bus ID.")->default_val(1);
        app->add_option("--priority", options.priority, "Source priority.")->default_val(1.0f);
        app->add_flag("--stream", options.stream, "Volume value.")->default_val(false);
        app->add_option("--scope", options.scope, "Source scope.")->default_str("World");
        app->add_option("--spatialization", options.spatialization, "Spatialization mode.")->default_str("None");
        app->add_option("--attenuation", options.attenuation, "Attenuation value.")->default_val(0);
        app->add_option("--fader", options.fader, "Fader name.")->default_str("Linear");
    }

    AppContext()
        : mainApp("Amplitude Project Manager", "ampm")
    {
        mainApp.require_subcommand(1);

        projectApp = mainApp.add_subcommand("project", "Manage Amplitude projects");
        assetsApp = mainApp.add_subcommand("assets", "Manage Amplitude assets");

        initProjectApp();
        initAssetsApp();
    }

    int run(int argc, char** argv)
    {
        const auto formatter = std::make_shared<AmplitudeToolCLIFormatter>();

        mainApp.set_version_flag("--version", "1.0.0");

        mainApp.formatter(formatter)
            ->usage("Usage: ampk [OPTIONS] PROJECT_DIR OUTPUT_FILE")
            ->footer("ampk -c 1 /path/to/project/ output_package.ampk");

        exeDirectory = weakly_canonical(std::filesystem::path(argv[0])).parent_path();

        CLI11_PARSE(mainApp, argc, argv);

        if (projectNewApp->parsed())
            return runProjectNew();

        if (assetsImportApp->parsed())
            return runImportSource();

        return EXIT_SUCCESS;
    }

    [[nodiscard]] int runProjectNew() const
    {
        // Create project directory
        const auto projectDir = std::filesystem::path(initProjectOptions.directory) / initProjectOptions.name;
        if (exists(projectDir))
        {
            std::cerr << "Error: Project directory already exists: " << projectDir.string() << std::endl;
            return EXIT_FAILURE;
        }

        std::filesystem::create_directories(projectDir);

        std::cout << "Initializing project '" << initProjectOptions.name << "' using template '" << initProjectOptions.templateName
                  << "' in directory '" << projectDir.string() << "'." << std::endl;

        // Copy template files to project directory
        if (initProjectOptions.templateName != "empty")
        {
            const auto templateDir = exeDirectory / "templates" / initProjectOptions.templateName;
            if (!exists(templateDir))
            {
                std::cerr << "Error: Template directory not found: " << templateDir.string() << std::endl;
                return EXIT_FAILURE;
            }

            // Copy template files to project directory
            copy(
                templateDir, projectDir / "sources",
                std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
        }
        else
        {
            // Create project "sources" directories
            create_directories(projectDir / "sources" / kProjectPathAttenuators);
            create_directories(projectDir / "sources" / kProjectPathCollections);
            create_directories(projectDir / "sources" / kProjectPathEffects);
            create_directories(projectDir / "sources" / kProjectPathEvents);
            create_directories(projectDir / "sources" / kProjectPathPipelines);
            create_directories(projectDir / "sources" / kProjectPathRTPC);
            create_directories(projectDir / "sources" / kProjectPathSoundBanks);
            create_directories(projectDir / "sources" / kProjectPathSounds);
            create_directories(projectDir / "sources" / kProjectPathSwitchContainers);
            create_directories(projectDir / "sources" / kProjectPathSwitches);

            // Create default config file
            {
                std::ofstream projectFile(projectDir / "sources" / "pc.config.json");
                projectFile << resource_default_config_json_data;
                projectFile.flush();
            }

            // Create default buses file
            {
                std::ofstream projectFile(projectDir / "sources" / "pc.buses.json");
                projectFile << resource_default_buses_json_data;
                projectFile.flush();
            }

            // Create default pipeline file
            {
                std::ofstream projectFile(projectDir / "sources" / kProjectPathPipelines / "default.json");
                projectFile << resource_default_pipeline_json_data;
                projectFile.flush();
            }
        }

        // Create project "build" directory
        create_directories(projectDir / "build");

        // Create project "data" directory
        create_directories(projectDir / "data");

        // Create project "plugins" directory
        create_directories(projectDir / "plugins");

        // Create project file
        std::ofstream projectFile(projectDir / ".amproject");
        if (!projectFile.is_open())
        {
            std::cerr << "Error: Failed to create project file in directory: " << projectDir.string() << std::endl;
            return EXIT_FAILURE;
        }

        projectFile
            << R"({"name": ")" << initProjectOptions.name
            << R"(", "default_configuration": "pc.config.amconfig", "sources_dir": "sources", "data_dir": "data", "build_dir": "build", "version": )"
            << kProjectVersion << R"( })";
        projectFile.flush();

        std::cout << "Project '" << initProjectOptions.name << "' initialized successfully" << std::endl;

        return EXIT_SUCCESS;
    }

    [[nodiscard]] int runImportSource() const
    {
        // TODO: Implement import source logic here
        std::cout << "Importing source..." << std::endl;

        // Return success or failure status
        return EXIT_SUCCESS;
    }

    void initProjectApp()
    {
        const auto templateNameValidator = CLI::Validator(
            [this](const std::string& input) -> std::string
            {
                std::vector<std::string> validTemplates = { "empty" };
                if (const auto templatesDir = exeDirectory / "templates"; exists(templatesDir))
                    for (const std::filesystem::directory_iterator it(templatesDir); const auto& entry : it)
                        if (entry.is_directory())
                            validTemplates.push_back(entry.path().filename().string());

                if (std::ranges::find(validTemplates, input) == validTemplates.end())
                    return "The provided template name is not valid. Valid options are: " + CLI::detail::join(validTemplates, ", ") + ".";

                return "";
            },
            "TEMPLATE_NAME", "TEMPLATE_NAME");

        const auto projectNameTransformer = CLI::Validator(
            [](std::string& input) -> std::string
            {
                input = SnakeCase(input);
                return {};
            },
            "PROJECT_NAME", "PROJECT_NAME");

        projectNewApp = projectApp->add_subcommand("new", "Initialize a new project");

        // project new
        {
            projectNewApp->add_option("-n,--name", initProjectOptions.name, "Project name.")->required()->transform(projectNameTransformer);

            projectNewApp
                ->add_option(
                    "-t,--template", initProjectOptions.templateName, "Project template. If not set, defaults to the 'empty' template.")
                ->default_val("empty")
                ->check(templateNameValidator, "TEMPLATE_NAME");

            projectNewApp
                ->add_option(
                    "OUTPUT", initProjectOptions.directory, "Project destination directory. If not set, defaults to current directory.")
                ->default_val(std::filesystem::current_path().string());
        }
    }

    void initAssetsApp()
    {
        assetsImportApp = assetsApp->add_subcommand("import", "Import new source asset from external sound files");

        // assets import
        {
            assetsImportApp->add_option("-i,--input", importSourceOptions.fileName, "Input file path.")->required();
            fillCommonSourceOptions(assetsImportApp, importSourceOptions);
        }
    }
};

int main(int argc, char** argv)
{
    AppContext app;
    return app.run(argc, argv);
}
