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
        constexpr AmUInt64 kBlock = 256;

        std::vector<AmReal32> PullAll(ResampleStream& stream, SourceReader& reader, AmUInt64 frames)
        {
            std::vector<AmReal32> result;
            AudioBuffer out(kBlock, 1);
            while (result.size() < frames)
            {
                const AmUInt64 n = AM_MIN(kBlock, frames - result.size());
                AM_UNUSED(stream.Pull(reader, out[0], 0, n));
                result.insert(result.end(), out[0].begin(), out[0].begin() + n);
            }

            return result;
        }

        AmUInt64 Mismatches(const std::vector<AmReal32>& seeked, const std::vector<AmReal32>& reference, AmUInt64 offset)
        {
            AmUInt64 count = 0;
            for (AmUInt64 i = 0; i < seeked.size(); ++i)
                count += seeked[i] != reference[offset + i] ? 1 : 0;
            return count;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_voice, resample_stream_seek_primes_history)
    {
    public:
        void Run() override
        {
            // 96 kHz into 48 kHz: every second source frame is an output centre, so a seek to an even frame lands on an
            // output of the continuous run, and must reproduce it from there on.
            constexpr AmUInt64 kLength = 6000;
            AudioBuffer buffer(kLength, 1);
            for (AmUInt64 i = 0; i < kLength; ++i)
                buffer[0][i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * AM_PI * 997.0 * static_cast<AmReal64>(i) / 96000.0));

            constexpr AmUInt64 kOutputs = 1024;

            struct Case
            {
                bool loop;
                AmUInt32 loopCount; ///< Total plays; 0 loops forever.
                AmUInt64 seek;
            };

            // Looping, a seek near the region start is compared against the second pass, which reads the loop's tail on
            // its left: the pre-roll must take its history from there too, even on a last pass with no seam left to cross.
            for (const Case& test : { Case{ false, 0, 2000 }, Case{ true, 0, 10 }, Case{ true, 1, 10 } })
            {
                SourceReader continuousReader;
                continuousReader.Initialize(MemorySource(buffer, 96000), 0, 0, test.loop, 0);
                ResampleStream continuous;
                AM_EXPECT(continuous.Initialize("default", 96000, 48000, 1, kBlock));

                const AmUInt64 seek = test.seek;
                const AmUInt64 offset = ((test.loop ? kLength : 0) + seek) / 2;
                const auto reference = PullAll(continuous, continuousReader, offset + kOutputs);

                SourceReader reader;
                reader.Initialize(MemorySource(buffer, 96000), 0, 0, test.loop, test.loopCount);

                ResampleStream stream;
                AM_EXPECT(stream.Initialize("default", 96000, 48000, 1, kBlock));
                AM_UNUSED(PullAll(stream, reader, 300));

                stream.Seek(reader, seek);
                AM_EXPECT_EQ(seek, stream.GetSourcePosition(reader));
                AM_EXPECT_EQ(seek, reader.GetCursor());

                const auto seeked = PullAll(stream, reader, kOutputs);
                AM_EXPECT_EQ(0ULL, Mismatches(seeked, reference, offset));
            }

            // Near the start of a sound that does not loop, there is nothing before it: the pre-roll stops at the region
            // start and the rest of the history stays silent, as a stream played from the start had it.
            {
                SourceReader continuousReader;
                continuousReader.Initialize(MemorySource(buffer, 96000), 0, 0, false, 0);
                ResampleStream continuous;
                AM_EXPECT(continuous.Initialize("default", 96000, 48000, 1, kBlock));
                const auto reference = PullAll(continuous, continuousReader, 5 + kOutputs);

                SourceReader reader;
                reader.Initialize(MemorySource(buffer, 96000), 0, 0, false, 0);
                ResampleStream stream;
                AM_EXPECT(stream.Initialize("default", 96000, 48000, 1, kBlock));
                AM_UNUSED(PullAll(stream, reader, 300));

                stream.Seek(reader, 10);
                AM_EXPECT_EQ(0ULL, Mismatches(PullAll(stream, reader, kOutputs), reference, 5));
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, resample_stream_seek_primes_history);
} // namespace SparkyStudios::Audio::Amplitude::Tests
