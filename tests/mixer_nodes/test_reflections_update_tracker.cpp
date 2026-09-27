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

#include <Core/RoomInternalState.h>
#include <Mixer/Nodes/ReflectionsNode.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_nodes, reflections_update_tracker)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 speed = 343.0f;

            RoomInternalState room;
            room.SetId(1);
            room.SetDimensions({ 10.0f, 4.0f, 6.0f });

            ReflectionsUpdateTracker tracker;

            // No room: nothing to compute.
            AM_EXPECT_NOT(tracker.Changed(nullptr, { 0.0f, 0.0f, 0.0f }, speed));

            // First sight of a room: compute.
            AM_EXPECT(tracker.Changed(&room, { 0.0f, 0.0f, 0.0f }, speed));
            // Nothing moved: skip.
            AM_EXPECT_NOT(tracker.Changed(&room, { 0.0f, 0.0f, 0.0f }, speed));

            // Listener moved.
            AM_EXPECT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed));
            AM_EXPECT_NOT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed));

            // Room resized.
            room.SetDimensions({ 12.0f, 4.0f, 6.0f });
            AM_EXPECT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed));

            // Room moved.
            room.SetLocation({ 0.0f, 0.0f, 5.0f });
            AM_EXPECT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed));

            // Cut-off changed.
            room.SetCutOffFrequency(room.GetCutOffFrequency() + 100.0f);
            AM_EXPECT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed));

            // Absorption coefficient changed.
            room.GetCoefficients()[0] += 0.1f;
            AM_EXPECT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed));

            // Speed of sound changed.
            AM_EXPECT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed + 1.0f));
            AM_EXPECT_NOT(tracker.Changed(&room, { 1.0f, 0.0f, 0.0f }, speed + 1.0f));

            // A different room instance with identical values still counts as a change.
            RoomInternalState other;
            other.SetId(2);
            other.SetDimensions(room.GetDimensions());
            AM_EXPECT(tracker.Changed(&other, { 1.0f, 0.0f, 0.0f }, speed + 1.0f));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reflections_update_tracker);
} // namespace SparkyStudios::Audio::Amplitude::Tests
