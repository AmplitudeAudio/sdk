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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_room, handles_default_constructor)
    {
    public:
        void Run() override
        {
            // Test default constructor creates invalid Room
            {
                Room defaultRoom;

                AM_EXPECT(!defaultRoom.Valid());
                AM_EXPECT_EQ(defaultRoom.GetId(), kAmInvalidObjectId);
                AM_EXPECT_EQ(defaultRoom.GetState(), nullptr);
            }

            // Test multiple default-constructed rooms
            {
                Room room1;
                Room room2;
                Room room3;

                AM_EXPECT(!room1.Valid());
                AM_EXPECT(!room2.Valid());
                AM_EXPECT(!room3.Valid());

                AM_EXPECT_EQ(room1.GetId(), kAmInvalidObjectId);
                AM_EXPECT_EQ(room2.GetId(), kAmInvalidObjectId);
                AM_EXPECT_EQ(room3.GetId(), kAmInvalidObjectId);

                AM_EXPECT_EQ(room1.GetState(), nullptr);
                AM_EXPECT_EQ(room2.GetState(), nullptr);
                AM_EXPECT_EQ(room3.GetState(), nullptr);
            }

            // Test that Clear() works on default-constructed Room
            {
                Room room;
                AM_EXPECT(!room.Valid());

                room.Clear(); // Should not crash
                AM_EXPECT(!room.Valid());
                AM_EXPECT_EQ(room.GetId(), kAmInvalidObjectId);
                AM_EXPECT_EQ(room.GetState(), nullptr);
            }

            // Test copy assignment from default-constructed Room
            {
                Room defaultRoom;
                Room anotherRoom;

                anotherRoom = defaultRoom;

                AM_EXPECT(!anotherRoom.Valid());
                AM_EXPECT_EQ(anotherRoom.GetId(), kAmInvalidObjectId);
                AM_EXPECT_EQ(anotherRoom.GetState(), nullptr);
            }
        }
    };

    AM_REGISTER_TEST(core_room, handles_default_constructor);
} // namespace SparkyStudios::Audio::Amplitude::Tests
