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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamRing.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, stream_ring_reads_what_was_written)
    {
    public:
        void Run() override
        {
            StreamRing ring(8, 2);
            AM_EXPECT_EQ(8u, ring.GetCapacity());
            AM_EXPECT_EQ(8u, ring.GetWritableFrames());

            AudioBuffer source(12, 2);
            for (AmUInt64 i = 0; i < 12; ++i)
            {
                source[0][i] = static_cast<AmReal32>(i);
                source[1][i] = -static_cast<AmReal32>(i);
            }

            // Fill to capacity: a ninth frame does not fit.
            AM_EXPECT_EQ(8u, ring.Write(source, 0, 12));
            AM_EXPECT_EQ(0u, ring.GetWritableFrames());
            AM_EXPECT_EQ(8u, ring.GetWriteIndex());

            // Reads do not consume: the same index reads twice.
            AudioBuffer out(4, 2);
            AM_EXPECT_EQ(4u, ring.Read(2, out, 0, 4));
            AM_EXPECT_EQ(2.0f, out[0][0]);
            AM_EXPECT_EQ(-5.0f, out[1][3]);
            AM_EXPECT_EQ(4u, ring.Read(2, out, 0, 4));

            // Reading past the write index returns what exists.
            AM_EXPECT_EQ(2u, ring.Read(6, out, 0, 4));

            // Releasing frees space; the next write wraps around the storage.
            ring.Release(5);
            AM_EXPECT_EQ(5u, ring.GetWritableFrames());
            AM_EXPECT_EQ(4u, ring.Write(source, 8, 4));
            AM_EXPECT_EQ(4u, ring.Read(8, out, 0, 4));
            AM_EXPECT_EQ(8.0f, out[0][0]);
            AM_EXPECT_EQ(-11.0f, out[1][3]);

            // Released frames can no longer be read.
            AM_EXPECT_EQ(0u, ring.Read(4, out, 0, 1));

            // Release never moves backwards.
            ring.Release(3);
            AM_EXPECT_EQ(5u, ring.GetReleaseIndex());

            // End and failure markers.
            AM_EXPECT_EQ(StreamRing::kNoEnd, ring.GetEndIndex());
            ring.MarkEnd();
            AM_EXPECT_EQ(12u, ring.GetEndIndex());
            AM_EXPECT(!ring.IsFailed());
            ring.MarkFailed();
            AM_EXPECT(ring.IsFailed());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, stream_ring_reads_what_was_written);
} // namespace SparkyStudios::Audio::Amplitude::Tests
