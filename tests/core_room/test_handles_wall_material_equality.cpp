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
    AM_TEST_CASE(ComponentTestCase, core_room, handles_wall_material_equality)
    {
    public:
        void Run() override
        {
            // Test equality operator with same material types
            {
                RoomWallMaterial material1(eRoomWallMaterialType_Metal);
                RoomWallMaterial material2(eRoomWallMaterialType_Metal);

                AM_EXPECT(material1 == material2);
                AM_EXPECT(!(material1 != material2));
            }

            // Test inequality operator with different material types
            {
                RoomWallMaterial material1(eRoomWallMaterialType_Metal);
                RoomWallMaterial material2(eRoomWallMaterialType_Wood);

                AM_EXPECT(material1 != material2);
                AM_EXPECT(!(material1 == material2));
            }

            // Test equality with multiple different material types
            {
                RoomWallMaterial glass1(eRoomWallMaterialType_Glass);
                RoomWallMaterial glass2(eRoomWallMaterialType_Glass);
                RoomWallMaterial marble(eRoomWallMaterialType_Marble);
                RoomWallMaterial transparent(eRoomWallMaterialType_Transparent);

                AM_EXPECT(glass1 == glass2);
                AM_EXPECT(glass1 != marble);
                AM_EXPECT(glass1 != transparent);
                AM_EXPECT(marble != transparent);
            }

            // Test self-equality
            {
                RoomWallMaterial material(eRoomWallMaterialType_Wood);
                AM_EXPECT(material == material);
                AM_EXPECT(!(material != material));
            }
        }
    };

    AM_REGISTER_TEST(core_room, handles_wall_material_equality);
} // namespace SparkyStudios::Audio::Amplitude::Tests
