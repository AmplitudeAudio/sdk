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

#pragma once

#include <CLI/CLI.hpp>

class AmplitudeToolCLIFormatter : public CLI::Formatter
{
public:
    AmplitudeToolCLIFormatter()
    {
        column_width(60);
        description_paragraph_width(120);
    }

    std::string make_description(const CLI::App* app) const override
    {
        std::string description = app->get_description();
        std::string program = "(" + app->get_name() + ")";
        std::string copyright = "Copyright (c) 2024-present Sparky Studios. All rights reserved.";

        const int lineWidth = std::max(description.size() + program.size() + 1, copyright.size());

        std::string result = "\n" + description + " " + program + "\n" + copyright + "\n" + std::string(lineWidth, '-') + "\n";

        return result;
    }

    std::string make_usage(const CLI::App* app, std::string name) const override
    {
        return "\n" + app->get_usage() + "\n\n";
    }

    std::string make_footer(const CLI::App *app) const override
    {
        std::string result = "\nExample:\n";
        result += "  " + app->get_footer() + "\n\n";
        result += "For more information, visit https://docs.amplitudeaudiosdk.com\n";
        return result;
    }
};
