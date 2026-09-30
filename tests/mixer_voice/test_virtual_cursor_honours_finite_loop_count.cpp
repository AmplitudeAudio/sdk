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
    AM_TEST_CASE(ComponentTestCase, mixer_voice, virtual_cursor_honours_finite_loop_count)
    {
    public:
        void Run() override
        {
            VirtualCursor cursor;

            // A 1000-frame region, 3 total passes, anchored right at the start of the first pass.
            cursor.Anchor(0, 0, 1.0, 0, 1000, true, 3);
            AM_EXPECT_EQ(3U, cursor.LoopsRemainingAt(0));
            AM_EXPECT_NOT(cursor.HasEnded(0));

            // Still inside the first pass.
            AM_EXPECT_EQ(500ULL, cursor.PositionAt(500));
            AM_EXPECT_EQ(3U, cursor.LoopsRemainingAt(500));
            AM_EXPECT_NOT(cursor.HasEnded(500));

            // Into the second pass (wrapped position), one pass done.
            AM_EXPECT_EQ(200ULL, cursor.PositionAt(1200));
            AM_EXPECT_EQ(2U, cursor.LoopsRemainingAt(1200));
            AM_EXPECT_NOT(cursor.HasEnded(1200));

            // Into the third (last) pass, two passes done.
            AM_EXPECT_EQ(100ULL, cursor.PositionAt(2100));
            AM_EXPECT_EQ(1U, cursor.LoopsRemainingAt(2100));
            AM_EXPECT_NOT(cursor.HasEnded(2100));

            // Exactly at the end of the third pass: ended, clamped at the region end.
            AM_EXPECT(cursor.HasEnded(3000));
            AM_EXPECT_EQ(1000ULL, cursor.PositionAt(3000));
            AM_EXPECT_EQ(0U, cursor.LoopsRemainingAt(3000));

            // Well past the end: still clamped, still ended (does not keep wrapping).
            AM_EXPECT(cursor.HasEnded(10000));
            AM_EXPECT_EQ(1000ULL, cursor.PositionAt(10000));

            // Anchored mid-way through the last pass: 100 frames left.
            cursor.Anchor(900, 0, 1.0, 0, 1000, true, 1);
            AM_EXPECT_EQ(1U, cursor.LoopsRemainingAt(0));
            AM_EXPECT_NOT(cursor.HasEnded(99));
            AM_EXPECT(cursor.HasEnded(100));

            // loopsRemaining of 0 (the engine's "loops forever" convention) never ends.
            cursor.Anchor(0, 0, 1.0, 0, 1000, true, 0);
            AM_EXPECT_EQ(0U, cursor.LoopsRemainingAt(0));
            AM_EXPECT_NOT(cursor.HasEnded(1'000'000));
        }
    };

    AM_REGISTER_TEST(mixer_voice, virtual_cursor_honours_finite_loop_count);
} // namespace SparkyStudios::Audio::Amplitude::Tests
