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

#include <filesystem>

#include <Fidelity/Signal.h>
#include <Fidelity/Wav.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, wav_round_trip)
    {
    public:
        void Run() override
        {
            const std::filesystem::path directory = std::filesystem::temp_directory_path() / "amplitude_fidelity_tests" / "wav";
            std::filesystem::remove_all(directory);
            std::filesystem::create_directories(directory);

            std::vector<float> samples = ToFloats(MakeSine(2000, 48000.0, 997.0, 0.5));
            samples[7] = 1.5f; // values above full scale survive
            const std::filesystem::path file = directory / "tone.wav";

            AM_EXPECT_EQ(EncodeWavFloat32(48000, 2, samples).size(), 44 + samples.size() * sizeof(float));
            AM_EXPECT(WriteWavFloat32(file, 48000, 2, samples));

            std::uint32_t rate = 0;
            std::uint16_t channels = 0;
            std::vector<float> read;
            AM_EXPECT(ReadWavFloat32(file, rate, channels, read));
            AM_EXPECT_EQ(rate, 48000);
            AM_EXPECT_EQ(channels, 2);
            AM_EXPECT(read == samples);

            AM_EXPECT(!ReadWavFloat32(directory / "missing.wav", rate, channels, read));
            std::filesystem::remove_all(directory);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, wav_round_trip);
} // namespace SparkyStudios::Audio::Amplitude::Tests
