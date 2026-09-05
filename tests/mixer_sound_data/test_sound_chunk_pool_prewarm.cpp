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

#include <Mixer/SoundData.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_sound_data, sound_chunk_pool_prewarm)
    {
    public:
        void Run() override
        {
            SoundChunkPool pool;

            // Pre-warm mirrors MixLayer's per-batch usage: input, mono transient, stereo output
            pool.PreWarm(1024, 2, 512);

            // All three shapes must be served from the pool
            SoundChunk* in = pool.Acquire(1024, 2);
            SoundChunk* transient = pool.Acquire(512, 1);
            SoundChunk* out = pool.Acquire(512, 2);

            AM_EXPECT_NOT(in == nullptr);
            AM_EXPECT_NOT(transient == nullptr);
            AM_EXPECT_NOT(out == nullptr);

            AM_EXPECT(in->frames >= 1024);
            AM_EXPECT(in->buffer->GetChannelCount() == 2);
            AM_EXPECT(transient->frames >= 512);
            AM_EXPECT(transient->buffer->GetChannelCount() == 1);
            AM_EXPECT(out->frames >= 512);
            AM_EXPECT(out->buffer->GetChannelCount() == 2);

            // All three shapes served from the pre-warmed pool, no new allocation
            AM_EXPECT(pool.allocated == 3);

            pool.Release(out);
            pool.Release(transient);
            pool.Release(in);

            // Re-acquiring a smaller shape must reuse a pooled chunk (no new allocation)
            SoundChunk* again = pool.Acquire(512, 2);
            AM_EXPECT_NOT(again == nullptr);
            AM_EXPECT(again == in || again == out);
            pool.Release(again);

            // Pre-warm is safe to call multiple times: no additional chunks are allocated
            pool.PreWarm(1024, 2, 512);
            AM_EXPECT(pool.allocated == 3);
        }
    };

    AM_REGISTER_TEST(mixer_sound_data, sound_chunk_pool_prewarm);
} // namespace SparkyStudios::Audio::Amplitude::Tests
