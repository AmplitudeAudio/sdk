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

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        // Produces outputs frames from source[from...] in one Process() call, zeros past the end.
        std::vector<AmReal32> Render(ResamplerInstance& r, const std::vector<AmReal32>& source, AmUInt64 from, AmUInt64 outputs)
        {
            AmUInt64 needed = r.GetInputFramesNeeded(outputs);
            AudioBuffer in(needed, 1);
            for (AmUInt64 i = 0; i < needed; ++i)
                in[0][i] = from + i < source.size() ? source[from + i] : 0.0f;

            AudioBuffer out(outputs, 1);
            AmUInt64 produced = outputs;
            r.Process(in, needed, out, produced);
            return { out[0].begin(), out[0].begin() + produced };
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_prime_history_matches_continuous)
    {
    public:
        void Run() override
        {
            std::vector<AmReal32> source(16384);
            for (AmSize i = 0; i < source.size(); ++i)
                source[i] = static_cast<AmReal32>(0.5 * std::sin(0.0731 * static_cast<AmReal64>(i)) + 0.25 * std::sin(0.913 * static_cast<AmReal64>(i)));

            constexpr AmUInt64 kSeek = 4000;
            constexpr AmUInt64 kOutputs = 512;

            // At these ratios an output of the continuous run is centred exactly on kSeek, so a primed instance started
            // there must reproduce the continuous run from that output on. A cold one starts on zeros and rings.
            for (const AmReal64 ratio : { 2.0, 0.5, 1.0 })
            {
                const auto offset = static_cast<AmUInt64>(static_cast<AmReal64>(kSeek) / ratio);

                for (const char* name : kResamplerPresets)
                {
                    auto continuous = Resampler::Construct(name);
                    continuous->Initialize(1, 48000, 48000);
                    continuous->SetRatio(ratio);
                    const auto reference = Render(*continuous, source, 0, offset + kOutputs);

                    auto primed = Resampler::Construct(name);
                    primed->Initialize(1, 48000, 48000);
                    primed->SetRatio(ratio);

                    // More history than any preset keeps: the instance takes the frames nearest the next input.
                    constexpr AmUInt64 kHistory = 1024;
                    AudioBuffer history(kHistory, 1);
                    for (AmUInt64 i = 0; i < kHistory; ++i)
                        history[0][i] = source[kSeek - kHistory + i];

                    primed->PrimeHistory(history, kHistory);
                    const auto seeked = Render(*primed, source, kSeek, kOutputs);

                    AM_EXPECT_EQ(static_cast<AmSize>(kOutputs), seeked.size());
                    AmUInt64 mismatches = 0;
                    for (AmUInt64 i = 0; i < seeked.size(); ++i)
                        mismatches += seeked[i] != reference[offset + i] ? 1 : 0;

                    AM_EXPECT_EQ(0ULL, mismatches);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_prime_history_matches_continuous);
} // namespace SparkyStudios::Audio::Amplitude::Tests
