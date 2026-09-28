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
#include <fstream>
#include <sstream>

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Stimuli.h>
#include <Fidelity/Wav.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        std::string ReadText(const std::filesystem::path& path)
        {
            std::ifstream file(path, std::ios::binary);
            std::ostringstream text;
            text << file.rdbuf();
            return text.str();
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, asset_generator)
    {
    public:
        void Run() override
        {
            const std::filesystem::path root = std::filesystem::temp_directory_path() / "amplitude_fidelity_tests" / "assets";
            std::filesystem::remove_all(root);
            const AssetPaths paths{ root / "project", root / "assets" };

            AM_EXPECT(ConfigName("isolated", 1024, 48000, ".config.json") == "fidelity.isolated.b1024.config.json");
            AM_EXPECT(ConfigName("isolated", 256, 44100, ".amconfig") == "fidelity.isolated.b256.r44100.amconfig");

            AM_EXPECT(GenerateAssets(paths));

            for (const ConfigVariant& variant : IsolatedConfigVariants())
            {
                const std::filesystem::path config =
                    paths.project / ConfigName("isolated", variant.blockSize, variant.outputRate, ".config.json");
                AM_EXPECT(std::filesystem::exists(config));
                const std::string text = ReadText(config);
                AM_EXPECT(text.find("\"buffer_size\": " + std::to_string(2 * variant.blockSize)) != std::string::npos);
                AM_EXPECT(text.find("\"frequency\": " + std::to_string(variant.outputRate)) != std::string::npos);
                AM_EXPECT(text.find("\"driver\": \"offline\"") != std::string::npos);
            }

            const std::string bank = ReadText(paths.project / "soundbanks" / "fidelity.json");
            for (const StimulusSpec& spec : StimulusCatalog())
            {
                AM_EXPECT(bank.find("\"fidelity/" + spec.name + ".amsound\"") != std::string::npos);
                AM_EXPECT(bank.find("\"fidelity/" + spec.name + "_stream.amsound\"") != std::string::npos);

                const std::string sound = ReadText(paths.project / "sounds" / "fidelity" / (spec.name + ".json"));
                AM_EXPECT(sound.find("\"name\":\"" + SoundName(spec, false) + "\"") != std::string::npos);
                AM_EXPECT(
                    sound.find("\"enabled\":" + std::string(spec.kind == StimulusKind::LoopSine ? "true" : "false")) != std::string::npos);

                std::uint32_t rate = 0;
                std::uint16_t channels = 0;
                std::vector<float> samples;
                AM_EXPECT(ReadWavFloat32(paths.assets / "data" / "fidelity" / (spec.name + ".wav"), rate, channels, samples));
                AM_EXPECT_EQ(rate, spec.sampleRate);
                AM_EXPECT_EQ(channels, spec.channels);
                AM_EXPECT_EQ(samples.size(), StimulusFrameCount(spec) * spec.channels);
            }

            // A second run leaves unchanged files untouched.
            const std::filesystem::path wav = paths.assets / "data" / "fidelity" / "sine_997_48000.wav";
            const auto before = std::filesystem::last_write_time(wav);
            AM_EXPECT(GenerateAssets(paths));
            AM_EXPECT(std::filesystem::last_write_time(wav) == before);

            std::filesystem::remove_all(root);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, asset_generator);
} // namespace SparkyStudios::Audio::Amplitude::Tests
