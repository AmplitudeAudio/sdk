// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    RoomInternalState state;
    state.SetId(1);

    fplutil::intrusive_list room_list(&RoomInternalState::node);
    room_list.push_back(state);

    Room wrapper(&state);
    AM_EXPECT_EQ(wrapper.GetState(), &state);

    // Test SetDimensions through state
    {
        constexpr AmVector3 dimensions = { 100, 100, 100 };
        state.SetDimensions(dimensions);

        AM_EXPECT_EQ(state.GetDimensions(), dimensions);
        AM_EXPECT_EQ(wrapper.GetDimensions(), dimensions);

        const BoxShape boxShape(50, 50, 50);
        AM_EXPECT_EQ(state.GetShape(), boxShape);
        AM_EXPECT_EQ(wrapper.GetShape(), boxShape);
    }

    // Test SetDimensions through wrapper
    {
        const AmVector3 dimensions = { 200.0f, 150.0f, 100.0f };
        wrapper.SetDimensions(dimensions);

        // Verify dimensions are set correctly through wrapper
        AM_EXPECT_EQ(wrapper.GetDimensions(), dimensions);
        AM_EXPECT_EQ(state.GetDimensions(), dimensions);

        // Verify the shape is updated accordingly (half dimensions)
        const BoxShape expectedShape(100.0f, 50.0f, 75.0f);
        AM_EXPECT_EQ(wrapper.GetShape(), expectedShape);
        AM_EXPECT_EQ(state.GetShape(), expectedShape);

        // Verify volume calculation
        const AmReal32 expectedVolume = dimensions.x * dimensions.y * dimensions.z;
        AM_EXPECT_EQ(wrapper.GetVolume(), expectedVolume);
        AM_EXPECT_EQ(state.GetVolume(), expectedVolume);
    }

    // Test SetDimensions with different values
    {
        const AmVector3 newDimensions = { 500.0f, 300.0f, 400.0f };
        wrapper.SetDimensions(newDimensions);

        // Verify new dimensions
        AM_EXPECT_EQ(wrapper.GetDimensions(), newDimensions);
        AM_EXPECT_EQ(state.GetDimensions(), newDimensions);

        // Verify the shape is updated
        const BoxShape newExpectedShape(250.0f, 200.0f, 150.0f);
        AM_EXPECT_EQ(wrapper.GetShape(), newExpectedShape);
        AM_EXPECT_EQ(state.GetShape(), newExpectedShape);

        // Verify new volume
        const AmReal32 newExpectedVolume = newDimensions.x * newDimensions.y * newDimensions.z;
        AM_EXPECT_EQ(wrapper.GetVolume(), newExpectedVolume);
        AM_EXPECT_EQ(state.GetVolume(), newExpectedVolume);
    }

    // Test SetDimensions with zero values (edge case)
    {
        const AmVector3 zeroDimensions = { 0.0f, 0.0f, 0.0f };
        wrapper.SetDimensions(zeroDimensions);

        AM_EXPECT_EQ(wrapper.GetDimensions(), zeroDimensions);
        AM_EXPECT_EQ(state.GetDimensions(), zeroDimensions);
        AM_EXPECT_EQ(wrapper.GetVolume(), 0.0f);
        AM_EXPECT_EQ(state.GetVolume(), 0.0f);
    }

    // Test SetDimensions with very small values
    {
        const AmVector3 smallDimensions = { 1.0f, 1.0f, 1.0f };
        wrapper.SetDimensions(smallDimensions);

        AM_EXPECT_EQ(wrapper.GetDimensions(), smallDimensions);
        AM_EXPECT_EQ(state.GetDimensions(), smallDimensions);
        AM_EXPECT_EQ(wrapper.GetVolume(), 1.0f);
        AM_EXPECT_EQ(state.GetVolume(), 1.0f);

        const BoxShape smallExpectedShape(0.5f, 0.5f, 0.5f);
        AM_EXPECT_EQ(wrapper.GetShape(), smallExpectedShape);
        AM_EXPECT_EQ(state.GetShape(), smallExpectedShape);
    }
}
