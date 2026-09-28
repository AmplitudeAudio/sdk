// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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
#include <string>

#include <CLI/CLI.hpp>

#include <Fidelity/AssetGenerator.h>

using namespace SparkyStudios::Audio::Amplitude::Fidelity;

int main(int argc, char** argv)
{
    CLI::App app{ "Amplitude fidelity harness" };

    bool generate = false;
    std::string project;
    std::string assets = "fidelity/assets";

    app.add_flag("--generate-assets", generate, "Generate the fidelity project files and stimuli, then exit.");
    app.add_option("--project", project, "Project directory written by --generate-assets.");
    app.add_option("--assets", assets, "Fidelity asset directory.");

    CLI11_PARSE(app, argc, argv);

    if (!generate)
    {
        std::cerr << "Nothing to do: pass --generate-assets.\n";
        return 1;
    }

    if (project.empty())
    {
        std::cerr << "--generate-assets needs --project.\n";
        return 1;
    }

    return GenerateAssets({ project, assets }) ? 0 : 1;
}
