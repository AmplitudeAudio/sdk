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

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Mixer/Amplimix.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, audio_clock_advances_with_mix)
    {
    public:
        void Run() override
        {
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;
            const AmUInt64 before = mixer.GetAudioClock();
            amEngine->WaitUntilFrames(10);
            const AmUInt64 after = mixer.GetAudioClock();

            // The clock counts rendered frames only, in whole blocks.
            AM_EXPECT(after > before);
            const DeviceDescription& device = mixer.GetDeviceDescription();
            const AmUInt64 block = device.mOutputBufferSize / static_cast<AmUInt64>(device.mRequestedOutputChannels);
            AM_EXPECT_EQ(0ULL, (after - before) % block);
        }
    };

    AM_REGISTER_TEST(core_engine, audio_clock_advances_with_mix);
} // namespace SparkyStudios::Audio::Amplitude::Tests
