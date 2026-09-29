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

#include <cmath>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/ResampleStream.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        std::vector<AmReal32> PullAll(const AudioBuffer& source, AmUInt32 sourceRate, AmUInt32 outputRate, AmUInt64 block, AmUInt64 total, bool& exact)
        {
            SourceReader reader;
            reader.Initialize(MemorySource(source, sourceRate), 0, 0, true, 0);

            ResampleStream stream;
            AM_UNUSED(stream.Initialize("default", sourceRate, outputRate, 1, 4096));

            std::vector<AmReal32> out;
            AudioBuffer buffer(total, 1);
            exact = true;

            for (AmUInt64 done = 0; done < total; done += block)
            {
                const AmUInt64 n = std::min(block, total - done);
                const auto report = stream.Pull(reader, buffer[0], done, n);
                exact &= report.produced == n && !report.error;
            }

            out.assign(buffer[0].begin(), buffer[0].begin() + total);
            return out;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_voice, resample_stream_produces_exact_frames)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 kRates[][2] = { { 44100, 48000 }, { 48000, 44100 }, { 22050, 48000 }, { 96000, 48000 }, { 48000, 48000 } };
            constexpr AmUInt64 kBlocks[] = { 1, 7, 256, 1024, 4096 };
            constexpr AmUInt64 kTotal = 16384;

            for (const auto& rates : kRates)
            {
                AudioBuffer source(4000, 1);
                for (AmUInt64 i = 0; i < 4000; ++i)
                    source[0][i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * 3.14159265358979323846 * 997.0 * static_cast<AmReal64>(i) / rates[0]));

                bool exact = false;
                const std::vector<AmReal32> expected = PullAll(source, rates[0], rates[1], kTotal, kTotal, exact);
                AM_EXPECT(exact);

                for (const AmUInt64 block : kBlocks)
                {
                    const std::vector<AmReal32> actual = PullAll(source, rates[0], rates[1], block, kTotal, exact);
                    AM_EXPECT(exact);

                    AmReal32 worst = 0.0f;
                    for (AmUInt64 i = 0; i < kTotal; ++i)
                        worst = std::max(worst, std::abs(actual[i] - expected[i]));
                    AM_EXPECT(worst < 1e-6f);
                }
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, resample_stream_produces_exact_frames);
} // namespace SparkyStudios::Audio::Amplitude::Tests
