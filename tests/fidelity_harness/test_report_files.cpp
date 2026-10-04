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
#include <limits>
#include <sstream>

#include <Fidelity/Report.h>
#include <Fidelity/Signal.h>
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

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, report_files)
    {
    public:
        void Run() override
        {
            Measurement m;
            m.variant = "sine_997_44100";
            m.Add("spectrum.thdnDb", -98.25, "dB", Better::Lower, -100.0);
            m.Add("broken", std::numeric_limits<double>::quiet_NaN(), "dB", Better::Lower, -90.0);
            m.events.push_back({ "click", 0, 4800, -70.0, 30.0, "stop", 12, 5 });
            m.capture.sampleRate = 48000;
            m.capture.channels = { ToFloats(MakeSine(4800, 48000.0, 997.0, 0.5)), ToFloats(MakeSine(4800, 48000.0, 997.0, 0.5)) };
            m.residual = { std::vector<float>(4800, 0.0f) };

            Measurement failed;
            failed.variant = "broken_variant";
            failed.error = "engine initialization failed";

            const std::vector<ScenarioResult> results = { ScenarioResult{ "P1", "Resampling quality", { m, failed } } };

            const std::filesystem::path out = std::filesystem::temp_directory_path() / "amplitude_fidelity_tests" / "report";
            std::filesystem::remove_all(out);

            ReportOptions options;
            options.outDir = out;
            options.gitSha = "abc1234";
            options.buildMode = "debug";
            AM_EXPECT(WriteReport(results, options, nullptr));

            const std::string json = ReadText(out / "report.json");
            AM_EXPECT(json.find("\"harness\": \"amplitude_fidelity\"") != std::string::npos);
            AM_EXPECT(json.find("\"git\": \"abc1234\"") != std::string::npos);
            AM_EXPECT(json.find("\"value\": null") != std::string::npos); // NaN
            AM_EXPECT(json.find("engine initialization failed") != std::string::npos);

            const std::string summary = ReadText(out / "summary.md");
            AM_EXPECT(summary.find("## P1") != std::string::npos);
            AM_EXPECT(summary.find("spectrum.thdnDb") != std::string::npos);
            AM_EXPECT(summary.find("misses") != std::string::npos);
            AM_EXPECT(summary.find("engine initialization failed") != std::string::npos);

            AM_EXPECT(std::filesystem::exists(out / "metrics.tsv"));

            std::uint32_t rate = 0;
            std::uint16_t channels = 0;
            std::vector<float> samples;
            AM_EXPECT(ReadWavFloat32(out / "captures" / "P1" / "sine_997_44100__b1024_f60_j0_r48000.wav", rate, channels, samples));
            AM_EXPECT(rate == 48000 && channels == 2 && samples.size() == 9600);
            AM_EXPECT(std::filesystem::exists(out / "captures" / "P1" / "sine_997_44100__b1024_f60_j0_r48000.residual.wav"));

            std::filesystem::remove_all(out);
            options.writeWavs = false;
            AM_EXPECT(WriteReport(results, options, nullptr));
            AM_EXPECT(!std::filesystem::exists(out / "captures"));
            std::filesystem::remove_all(out);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, report_files);
} // namespace SparkyStudios::Audio::Amplitude::Tests
