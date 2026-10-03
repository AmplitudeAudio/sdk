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

#pragma once

#ifndef _AM_FIDELITY_ASSET_GENERATOR_H
#define _AM_FIDELITY_ASSET_GENERATOR_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <Fidelity/Stimuli.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Where the generated project (JSON sources) and the compiled assets (with stimulus WAVs) live.
     */
    struct AssetPaths
    {
        std::filesystem::path project;
        std::filesystem::path assets;
    };

    /**
     * @brief A generated engine config: block size in frames and output rate.
     */
    struct ConfigVariant
    {
        std::uint32_t blockSize = 1024;
        std::uint32_t outputRate = 48000;
    };

    /**
     * @brief Block sizes 256/1024/4096 at 48 kHz and 44.1 kHz.
     */
    [[nodiscard]] std::vector<ConfigVariant> IsolatedConfigVariants();

    /**
     * @brief "fidelity.<kind>.b<blockSize>[.r<rate>]<extension>"; the rate is omitted at 48 kHz.
     */
    [[nodiscard]] std::string ConfigName(
        std::string_view kind, std::uint32_t blockSize, std::uint32_t outputRate, std::string_view extension);

    /**
     * @brief Engine sound name of a stimulus: "fidelity.<name>", with "_stream" for the streamed definition.
     */
    [[nodiscard]] std::string SoundName(const StimulusSpec& spec, bool streamed);

    /**
     * @brief Writes the generated configs, sound definitions and soundbank under paths.project, and the stimulus WAVs
     * under paths.assets/data/fidelity. Unchanged files are not rewritten. Returns false (and reports on stderr) on failure.
     */
    bool GenerateAssets(const AssetPaths& paths);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ASSET_GENERATOR_H
