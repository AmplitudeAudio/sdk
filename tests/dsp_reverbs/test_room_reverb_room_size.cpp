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
#include <Utils/Audio/Reverb/RoomReverb.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_reverbs, room_reverb_room_size)
    {
    public:
        void Run() override
        {
            RoomReverb smallRoom(48000);
            smallRoom.SetRoomSize(0.1f);

            RoomReverb largeRoom(48000);
            largeRoom.SetRoomSize(0.9f);

            AudioBuffer in(4096, 1);
            AudioBuffer outSmall(4096, 2);
            AudioBuffer outLarge(4096, 2);

            in[0][0] = 1.0f;

            smallRoom.Process(in, outSmall);
            largeRoom.Process(in, outLarge);

            AmReal32 lateEnergySmall = 0.0f;
            AmReal32 lateEnergyLarge = 0.0f;
            for (AmUInt64 f = 2000; f < 4096; ++f)
            {
                lateEnergySmall += outSmall[0][f] * outSmall[0][f];
                lateEnergyLarge += outLarge[0][f] * outLarge[0][f];
            }

            // Larger room must yield significantly more late reverberant energy
            AM_EXPECT(lateEnergyLarge > lateEnergySmall * 1.5f);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, room_reverb_room_size);
} // namespace SparkyStudios::Audio::Amplitude::Tests
