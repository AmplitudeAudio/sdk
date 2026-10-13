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

#include <Fidelity/Report.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // A damaged baseline must be refused, never read partially: a skipped line turns its metric into "New" and
    // silently removes it from the gate.
    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, baseline_parsing_is_strict)
    {
    public:
        void Run() override
        {
            const std::filesystem::path directory = std::filesystem::temp_directory_path() / "amplitude_fidelity_tests" / "strict_tsv";
            std::filesystem::remove_all(directory);
            std::filesystem::create_directories(directory);

            const auto parse = [&](const std::string& body)
            {
                const std::filesystem::path file = directory / "baseline.tsv";
                std::ofstream(file, std::ios::binary | std::ios::trunc) << "# key\tvalue\tunit\tbetter\ttarget\n" << body;
                bool threw = false;
                std::optional<Baseline> result;
                try
                {
                    result = ReadTsv(file);
                } catch (...)
                {
                    threw = true;
                }

                AM_EXPECT(!threw);
                return result;
            };

            const std::string valid = "P1|v|b1024_f60_j0_r48000|m\t-98.250000\tdB\tlower\t-100.000000\n";
            AM_EXPECT(parse(valid).has_value());
            AM_EXPECT(parse(valid + "P1|v|b1024_f60_j0_r48000|n\tnan\tdB\tlower\t-\n").has_value());

            AM_EXPECT(!parse(valid + "P1|v|b1024_f60_j0_r48000|x\t-130.000000\t-100.000000\n").has_value()); // 3 fields
            AM_EXPECT(!parse(valid + "P1|v|b1024_f60_j0_r48000|x\tabc\tdB\tlower\t-100.000000\n").has_value()); // bad value
            AM_EXPECT(!parse(valid + "P1|v|b1024_f60_j0_r48000|x\t-1.0dB\tdB\tlower\t-100.000000\n").has_value()); // trailing text
            AM_EXPECT(!parse(valid + "P1|v|b1024_f60_j0_r48000|x\t-1.0\tdB\tsideways\t-100.000000\n").has_value()); // bad direction
            AM_EXPECT(!parse(valid + "P1|v|b1024_f60_j0_r48000|x\t-1.0\tdB\tlower\tnope\n").has_value()); // bad target
            AM_EXPECT(!parse(valid + valid).has_value()); // duplicate key

            std::filesystem::remove_all(directory);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, baseline_parsing_is_strict);
} // namespace SparkyStudios::Audio::Amplitude::Tests
