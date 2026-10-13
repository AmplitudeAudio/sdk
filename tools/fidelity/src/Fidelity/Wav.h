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

#ifndef _AM_FIDELITY_WAV_H
#define _AM_FIDELITY_WAV_H

#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Bytes of an IEEE-float 32-bit WAV file (44-byte header) holding @p interleaved samples.
     */
    [[nodiscard]] std::vector<char> EncodeWavFloat32(std::uint32_t sampleRate, std::uint16_t channels, std::span<const float> interleaved);

    /**
     * @brief Writes an IEEE-float 32-bit WAV file. Returns false on I/O failure.
     */
    bool WriteWavFloat32(
        const std::filesystem::path& path, std::uint32_t sampleRate, std::uint16_t channels, std::span<const float> interleaved);

    /**
     * @brief Reads an IEEE-float 32-bit WAV file. Returns false when the file is missing or in another format.
     */
    bool ReadWavFloat32(
        const std::filesystem::path& path, std::uint32_t& sampleRate, std::uint16_t& channels, std::vector<float>& interleaved);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_WAV_H
