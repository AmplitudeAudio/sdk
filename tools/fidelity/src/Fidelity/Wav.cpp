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

#include <Fidelity/Wav.h>

#include <bit>
#include <cstring>
#include <fstream>
#include <iterator>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    static_assert(std::endian::native == std::endian::little, "The WAV writer assumes a little-endian host.");

    std::vector<char> EncodeWavFloat32(std::uint32_t sampleRate, std::uint16_t channels, std::span<const float> interleaved)
    {
        const auto dataBytes = static_cast<std::uint32_t>(interleaved.size() * sizeof(float));
        std::vector<char> out;
        out.reserve(44 + dataBytes);

        const auto put = [&out](const void* data, std::size_t size)
        {
            const auto* bytes = static_cast<const char*>(data);
            out.insert(out.end(), bytes, bytes + size);
        };
        const auto u16 = [&put](std::uint16_t value)
        {
            put(&value, sizeof(value));
        };
        const auto u32 = [&put](std::uint32_t value)
        {
            put(&value, sizeof(value));
        };

        put("RIFF", 4);
        u32(36 + dataBytes);
        put("WAVE", 4);
        put("fmt ", 4);
        u32(16);
        u16(3); // WAVE_FORMAT_IEEE_FLOAT
        u16(channels);
        u32(sampleRate);
        u32(sampleRate * channels * sizeof(float));
        u16(static_cast<std::uint16_t>(channels * sizeof(float)));
        u16(32);
        put("data", 4);
        u32(dataBytes);
        put(interleaved.data(), dataBytes);
        return out;
    }

    bool WriteWavFloat32(
        const std::filesystem::path& path, std::uint32_t sampleRate, std::uint16_t channels, std::span<const float> interleaved)
    {
        const std::vector<char> bytes = EncodeWavFloat32(sampleRate, channels, interleaved);
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        return file.good();
    }

    bool ReadWavFloat32(
        const std::filesystem::path& path, std::uint32_t& sampleRate, std::uint16_t& channels, std::vector<float>& interleaved)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        const std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (bytes.size() < 12 || std::memcmp(bytes.data(), "RIFF", 4) != 0 || std::memcmp(bytes.data() + 8, "WAVE", 4) != 0)
            return false;

        const auto read16 = [&bytes](std::size_t offset)
        {
            std::uint16_t value = 0;
            std::memcpy(&value, bytes.data() + offset, sizeof(value));
            return value;
        };
        const auto read32 = [&bytes](std::size_t offset)
        {
            std::uint32_t value = 0;
            std::memcpy(&value, bytes.data() + offset, sizeof(value));
            return value;
        };

        bool haveFormat = false;
        std::size_t offset = 12;
        while (offset + 8 <= bytes.size())
        {
            const std::uint32_t size = read32(offset + 4);
            const std::size_t body = offset + 8;
            if (body + size > bytes.size())
                return false;

            if (std::memcmp(bytes.data() + offset, "fmt ", 4) == 0)
            {
                if (size < 16 || read16(body) != 3 || read16(body + 14) != 32)
                    return false;

                channels = read16(body + 2);
                sampleRate = read32(body + 4);
                haveFormat = true;
            }
            else if (std::memcmp(bytes.data() + offset, "data", 4) == 0 && haveFormat)
            {
                interleaved.resize(size / sizeof(float));
                std::memcpy(interleaved.data(), bytes.data() + body, interleaved.size() * sizeof(float));
                return true;
            }

            offset = body + size + (size & 1u);
        }

        return false;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
