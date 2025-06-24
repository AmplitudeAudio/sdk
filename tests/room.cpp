// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <catch2/catch_test_macros.hpp>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/RoomInternalState.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Room Tests", "[room][core][amplitude]")
{
    RoomInternalState state;
    state.SetId(1);

    fplutil::intrusive_list room_list(&RoomInternalState::node);
    room_list.push_back(state);

    SECTION("can be used without a wrapper")
    {
        WHEN("the ID changes")
        {
            state.SetId(5);

            THEN("it returns the new ID")
            {
                REQUIRE(state.GetId() == 5);
            }
        }

        WHEN("the location changes")
        {
            constexpr AmVector3 location = { 10, 20, 30 };
            state.SetLocation(location);

            THEN("it returns the new location")
            {
                REQUIRE(state.GetLocation() == location);
            }

            AND_WHEN("an update occurs")
            {
                state.Update();

                THEN("the location stays the same")
                {
                    REQUIRE(state.GetLocation() == location);
                }
            }
        }

        WHEN("the orientation changes")
        {
            const auto direction = kVector3UnitX;
            const auto up = kVector3UnitZ;
            const auto orientation = Orientation(direction, up);
            state.SetOrientation(orientation);

            THEN("it returns the new orientation")
            {
                REQUIRE(state.GetDirection() == direction);
                REQUIRE(state.GetUp() == up);
                REQUIRE(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
            }
        }

        WHEN("the shape changes")
        {
            BoxShape shape(500, 250, 250);
            state.SetShape(shape);

            THEN("it returns the new shape")
            {
                REQUIRE(state.GetShape() == shape);
            }

            THEN("it compute correct dimensions")
            {
                REQUIRE(state.GetDimensions() == AmVector3{ 1000, 500, 500 });
            }

            THEN("it computes the correct volume")
            {
                REQUIRE(state.GetVolume() == 1000 * 500 * 500);
            }

            THEN("it computes the correct surface areas")
            {
                REQUIRE(state.GetSurfaceArea(eRoomWall_Back) == 1000 * 500);
                REQUIRE(state.GetSurfaceArea(eRoomWall_Front) == state.GetSurfaceArea(eRoomWall_Back));

                REQUIRE(state.GetSurfaceArea(eRoomWall_Top) == 1000 * 500);
                REQUIRE(state.GetSurfaceArea(eRoomWall_Bottom) == state.GetSurfaceArea(eRoomWall_Top));

                REQUIRE(state.GetSurfaceArea(eRoomWall_Left) == 500 * 500);
                REQUIRE(state.GetSurfaceArea(eRoomWall_Right) == state.GetSurfaceArea(eRoomWall_Left));
            }

            AND_WHEN("the dimensions change")
            {
                constexpr AmVector3 dimensions = { 100, 100, 100 };
                state.SetDimensions(dimensions);

                THEN("it returns the new dimensions")
                {
                    REQUIRE(state.GetDimensions() == dimensions);
                }

                THEN("it compute the correct shape")
                {
                    BoxShape box_shape(50, 50, 50);
                    REQUIRE(state.GetShape() == box_shape);
                }
            }
        }

        WHEN("the room materials changes")
        {
            SECTION("setting materials per wall")
            {
                state.SetWallMaterial(eRoomWall_Front, RoomWallMaterial(eRoomWallMaterialType_Metal));
                state.SetWallMaterial(eRoomWall_Back, RoomWallMaterial(eRoomWallMaterialType_Marble));

                THEN("it returns the new materials per wall")
                {
                    REQUIRE(state.GetWallMaterial(eRoomWall_Front) == RoomWallMaterial(eRoomWallMaterialType_Metal));
                    REQUIRE(state.GetWallMaterial(eRoomWall_Back) == RoomWallMaterial(eRoomWallMaterialType_Marble));
                }

                THEN("other walls' materials remain unchanged")
                {
                    REQUIRE(state.GetWallMaterial(eRoomWall_Bottom) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                    REQUIRE(state.GetWallMaterial(eRoomWall_Top) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                    REQUIRE(state.GetWallMaterial(eRoomWall_Left) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                    REQUIRE(state.GetWallMaterial(eRoomWall_Right) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                }
            }
        }

        WHEN("the cutoff frequency changes")
        {
            constexpr auto kCutoffFrequency = 1000.0f;
            state.SetCutOffFrequency(kCutoffFrequency);

            THEN("it returns the new cutoff frequency")
            {
                REQUIRE(state.GetCutOffFrequency() == kCutoffFrequency);
            }
        }

        WHEN("the gain changes")
        {
            constexpr auto kGain = 0.5f;
            state.SetGain(kGain);

            THEN("it returns the new gain")
            {
                REQUIRE(state.GetGain() == kGain);
            }
        }
    }

    SECTION("can be used with a wrapper")
    {
        Room wrapper(&state);
        REQUIRE(wrapper.GetState() == &state);

        SECTION("can return the correct ID")
        {
            REQUIRE(wrapper.GetId() == 1);

            REQUIRE(state.GetId() == 1);
        }

        WHEN("the location changes")
        {
            constexpr AmVector3 location = { 10, 20, 30 };
            wrapper.SetLocation(location);

            THEN("it returns the new location")
            {
                REQUIRE(wrapper.GetLocation() == location);

                REQUIRE(wrapper.GetLocation() == state.GetLocation());
            }

            AND_WHEN("an update occurs")
            {
                wrapper.Update();

                THEN("the location stays the same")
                {
                    REQUIRE(wrapper.GetLocation() == location);

                    REQUIRE(wrapper.GetLocation() == state.GetLocation());
                }
            }
        }

        WHEN("the orientation changes")
        {
            const auto direction = kVector3UnitX;
            const auto up = kVector3UnitZ;
            const auto orientation = Orientation(direction, up);
            wrapper.SetOrientation(orientation);

            THEN("it returns the new orientation")
            {
                REQUIRE(wrapper.GetDirection() == direction);
                REQUIRE(wrapper.GetUp() == up);

                REQUIRE(wrapper.GetDirection() == state.GetDirection());
                REQUIRE(wrapper.GetUp() == state.GetUp());

                REQUIRE(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                REQUIRE(std::memcmp(&state.GetOrientation(), &wrapper.GetOrientation(), sizeof(Orientation)) == 0);
            }
        }

        WHEN("the shape changes")
        {
            BoxShape shape(500, 250, 250);
            wrapper.SetShape(shape);

            THEN("it returns the new shape")
            {
                REQUIRE(wrapper.GetShape() == shape);
                REQUIRE(wrapper.GetShape() == state.GetShape());
            }

            THEN("it compute correct dimensions")
            {
                REQUIRE(wrapper.GetDimensions() == AmVector3{ 1000, 500, 500 });
                REQUIRE(wrapper.GetDimensions() == state.GetDimensions());
            }

            THEN("it computes the correct volume")
            {
                REQUIRE(wrapper.GetVolume() == 1000 * 500 * 500);
                REQUIRE(wrapper.GetVolume() == state.GetVolume());
            }

            THEN("it computes the correct surface areas")
            {
                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Back) == 1000 * 500);
                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Front) == wrapper.GetSurfaceArea(eRoomWall_Back));

                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Top) == 1000 * 500);
                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Bottom) == wrapper.GetSurfaceArea(eRoomWall_Top));

                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Left) == 500 * 500);
                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Right) == wrapper.GetSurfaceArea(eRoomWall_Left));

                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Back) == state.GetSurfaceArea(eRoomWall_Back));
                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Front) == state.GetSurfaceArea(eRoomWall_Front));

                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Top) == state.GetSurfaceArea(eRoomWall_Top));
                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Bottom) == state.GetSurfaceArea(eRoomWall_Bottom));

                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Left) == state.GetSurfaceArea(eRoomWall_Left));
                REQUIRE(wrapper.GetSurfaceArea(eRoomWall_Right) == state.GetSurfaceArea(eRoomWall_Right));
            }

            AND_WHEN("the dimensions change")
            {
                constexpr AmVector3 dimensions = { 100, 100, 100 };
                wrapper.SetDimensions(dimensions);

                THEN("it returns the new dimensions")
                {
                    REQUIRE(wrapper.GetDimensions() == dimensions);
                    REQUIRE(wrapper.GetDimensions() == state.GetDimensions());
                }

                THEN("it compute the correct shape")
                {
                    BoxShape box_shape(50, 50, 50);
                    REQUIRE(wrapper.GetShape() == box_shape);
                    REQUIRE(wrapper.GetShape() == state.GetShape());
                }
            }
        }

        WHEN("the room materials changes")
        {
            SECTION("setting materials per wall")
            {
                wrapper.SetWallMaterial(eRoomWall_Front, RoomWallMaterial(eRoomWallMaterialType_Metal));
                wrapper.SetWallMaterial(eRoomWall_Back, RoomWallMaterial(eRoomWallMaterialType_Marble));

                THEN("it returns the new materials per wall")
                {
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Front) == RoomWallMaterial(eRoomWallMaterialType_Metal));
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Back) == RoomWallMaterial(eRoomWallMaterialType_Marble));
                }

                THEN("other walls' materials remain unchanged")
                {
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Bottom) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Top) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Left) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Right) == RoomWallMaterial(eRoomWallMaterialType_Transparent));
                }
            }

            SECTION("setting materials for the entire room")
            {
                const auto material = RoomWallMaterial(eRoomWallMaterialType_Glass);
                wrapper.SetAllWallMaterials(material);

                THEN("it returns the new materials for the entire room")
                {
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Front) == material);
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Back) == material);
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Top) == material);
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Bottom) == material);
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Left) == material);
                    REQUIRE(wrapper.GetWallMaterial(eRoomWall_Right) == material);
                }
            }

            SECTION("setting all materials at once")
            {
                const std::vector<RoomWallMaterial> materials = {
                    RoomWallMaterial(eRoomWallMaterialType_Metal), RoomWallMaterial(eRoomWallMaterialType_Marble),
                    RoomWallMaterial(eRoomWallMaterialType_Glass), RoomWallMaterial(eRoomWallMaterialType_BrickPainted),
                    RoomWallMaterial(eRoomWallMaterialType_Wood),  RoomWallMaterial(eRoomWallMaterialType_FoamPanel)
                };

                wrapper.SetWallMaterials(materials[0], materials[1], materials[2], materials[3], materials[4], materials[5]);

                THEN("it returns the new materials for the entire room")
                {
                    for (size_t i = 0; i < kAmRoomSurfaceCount; ++i)
                    {
                        REQUIRE(wrapper.GetWallMaterial(static_cast<eRoomWall>(i)) == materials[i]);
                    }
                }
            }
        }

        WHEN("the gain changes")
        {
            constexpr auto kGain = 0.5f;
            wrapper.SetGain(kGain);

            THEN("it returns the new gain")
            {
                REQUIRE(wrapper.GetGain() == kGain);
                REQUIRE(wrapper.GetGain() == state.GetGain());
            }
        }

        WHEN("the internal state wrapper is cleared")
        {
            wrapper.Clear();

            THEN("it is no longer valid")
            {
                REQUIRE_FALSE(wrapper.Valid());
            }
        }
    }

    SECTION("cannot create a valid wrapper with a null state")
    {
        Room wrapper2(nullptr);
        REQUIRE_FALSE(wrapper2.Valid());

        Room wrapper3;
        REQUIRE_FALSE(wrapper3.Valid());
    }
}