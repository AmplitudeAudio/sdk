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
    /// Engine name of the looping sine whose pitch the RTPC drives.
    constexpr const char* kGlideSoundName = "fidelity.glide";

    /// Engine name of the 997 Hz loop played as a positional sound (the pan scenarios place it with a location).
    constexpr const char* kPositionalSineName = "fidelity.positional_sine";

    /// Engine name of the looping white noise (white_48000, not in the stimulus catalog) played with HRTF spatialization: the ambisonic scenarios place it
    /// with a location.
    constexpr const char* kHrtfNoiseName = "fidelity.hrtf_noise";

    /// Engine name of the independent stereo pink noise whose spread follows an RTPC.
    constexpr const char* kSpreadSweepSoundName = "fidelity.spread_sweep";

    /// Engine name of the RTPC the spread sweep's spread curve follows (0 - 360).
    constexpr const char* kSpreadRtpcName = "fidelity_spread";

    /// Engine name of the RTPC the glide sound's pitch curve follows.
    constexpr const char* kGlideRtpcName = "fidelity_pitch";

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
        /// Output channels: 2 (stereo), 6 (5.1) or 8 (7.1).
        std::uint16_t channels = 2;
        /// Renders through the ambisonic pipeline in BinauralHighQuality mode (needs the HRIR sphere).
        bool ambisonic = false;
    };

    /**
     * @brief Block sizes 256/1024/4096 at 48 kHz and 44.1 kHz.
     */
    [[nodiscard]] std::vector<ConfigVariant> IsolatedConfigVariants();

    /**
     * @brief 5.1 and 7.1 at block 1024, 48 kHz.
     */
    [[nodiscard]] std::vector<ConfigVariant> SurroundConfigVariants();

    /**
     * @brief 5.1, block 1024, 48 kHz, ambisonic.
     */
    [[nodiscard]] std::vector<ConfigVariant> AmbisonicConfigVariants();

    /**
     * @brief "isolated" for stereo, "surround51", "surround71", "ambisonic51".
     */
    [[nodiscard]] std::string ConfigKind(const ConfigVariant& variant);

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
