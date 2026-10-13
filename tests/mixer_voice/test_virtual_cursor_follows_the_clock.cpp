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

#include <Mixer/Voice/VirtualCursor.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_voice, virtual_cursor_follows_the_clock)
    {
    public:
        void Run() override
        {
            VirtualCursor cursor;
            AM_EXPECT_NOT(cursor.IsAnchored());

            // 44.1 kHz source on a 48 kHz clock, looping over [0, 44100).
            cursor.Anchor(1000, 48000, 44100.0 / 48000.0, 0, 44100, true);
            AM_EXPECT(cursor.IsAnchored());
            AM_EXPECT_EQ(1000ULL, cursor.PositionAt(48000));
            AM_EXPECT_EQ(1000ULL + 44100ULL / 2ULL, cursor.PositionAt(48000 + 24000));
            AM_EXPECT_EQ(1000ULL, cursor.PositionAt(48000 + 48000)); // one full loop later
            AM_EXPECT_NOT(cursor.HasEnded(10'000'000));
            AM_EXPECT_EQ(1000ULL, cursor.PositionAt(0)); // clocks before the anchor do not rewind

            // No loop: clamps at the end and reports it.
            cursor.Anchor(40000, 0, 1.0, 0, 44100, false);
            AM_EXPECT_EQ(44100ULL, cursor.PositionAt(10000));
            AM_EXPECT(cursor.HasEnded(4100));
            AM_EXPECT_NOT(cursor.HasEnded(4099));

            // Double speed.
            cursor.Anchor(0, 0, 2.0, 0, 100000, true);
            AM_EXPECT_EQ(2000ULL, cursor.PositionAt(1000));

            cursor.Clear();
            AM_EXPECT_NOT(cursor.IsAnchored());
        }
    };

    AM_REGISTER_TEST(mixer_voice, virtual_cursor_follows_the_clock);
} // namespace SparkyStudios::Audio::Amplitude::Tests
