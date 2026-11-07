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

#include "RecordingCodec.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Once a streamed voice is playing, mixing it (and the worker keeping its ring filled) goes through no pool.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_stream_mix_never_allocates)
    {
    public:
        void Run() override
        {
#if !defined(AM_NO_MEMORY_STATS)
            const auto allocations = []()
            {
                AmUInt64 total = 0;
                for (AmUInt32 k = 0; k < eMemoryPoolKind_COUNT; ++k)
                    total += amMemory->GetStats(static_cast<eMemoryPoolKind>(k)).allocCount.load();
                return total;
            };

            EventCount began;
            Channel channel = amEngine->Play(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &began);

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return began.count.load() > 0 && channel.Playing();
                }));

            // Let the first blocks and the decoder's scratch settle.
            const AmUInt32 settled = MixedBlocks().load() + 20;
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return MixedBlocks().load() >= settled;
                }));

            // The game thread is idle while the audio thread mixes 200 blocks: only the mixer and the worker allocate.
            const AmUInt32 target = MixedBlocks().load() + 200;
            const AmUInt64 before = allocations();
            for (AmUInt32 i = 0; i < 2000 && MixedBlocks().load() < target; ++i)
                Thread::Sleep(10);
            const AmUInt64 after = allocations();

            AM_EXPECT(MixedBlocks().load() >= target);
            AM_EXPECT_EQ(before, after);

            channel.Stop(0);
#else
            AM_EXPECT(false);
#endif
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_stream_mix_never_allocates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
