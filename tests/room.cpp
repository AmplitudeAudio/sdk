// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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
            const auto location = AM_V3(10, 20, 30);
            state.SetLocation(location);

            THEN("it returns the new location")
            {
                REQUIRE(AM_EqV3(state.GetLocation(), location));
            }

            AND_WHEN("an update occurs")
            {
                state.Update();

                THEN("the location stays the same")
                {
                    REQUIRE(AM_EqV3(state.GetLocation(), location));
                }
            }
        }

        WHEN("the orientation changes")
        {
            const auto direction = AM_V3(1, 0, 0);
            const auto up = AM_V3(0, 0, 1);
            const auto orientation = Orientation(direction, up);
            state.SetOrientation(orientation);

            THEN("it returns the new orientation")
            {
                REQUIRE(AM_EqV3(state.GetDirection(), direction));
                REQUIRE(AM_EqV3(state.GetUp(), up));
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
                REQUIRE(AM_EqV3(state.GetDimensions(), AM_V3(1000, 500, 500)));
            }

            THEN("it computes the correct volume")
            {
                REQUIRE(state.GetVolume() == 1000 * 500 * 500);
            }

            THEN("it computes the correct surface areas")
            {
                REQUIRE(state.GetSurfaceArea(RoomWall::Back) == 1000 * 500);
                REQUIRE(state.GetSurfaceArea(RoomWall::Front) == state.GetSurfaceArea(RoomWall::Back));

                REQUIRE(state.GetSurfaceArea(RoomWall::Top) == 1000 * 500);
                REQUIRE(state.GetSurfaceArea(RoomWall::Bottom) == state.GetSurfaceArea(RoomWall::Top));

                REQUIRE(state.GetSurfaceArea(RoomWall::Left) == 500 * 500);
                REQUIRE(state.GetSurfaceArea(RoomWall::Right) == state.GetSurfaceArea(RoomWall::Left));
            }

            AND_WHEN("the dimensions change")
            {
                const auto dimensions = AM_V3(100, 100, 100);
                state.SetDimensions(dimensions);

                THEN("it returns the new dimensions")
                {
                    REQUIRE(AM_EqV3(state.GetDimensions(), dimensions));
                }

                THEN("it compute the correct shape")
                {
                    BoxShape shape(50, 50, 50);
                    REQUIRE(state.GetShape() == shape);
                }
            }
        }

        WHEN("the room materials changes")
        {
            SECTION("setting materials per wall")
            {
                state.SetWallMaterial(RoomWall::Front, RoomMaterial(RoomMaterialType::Metal));
                state.SetWallMaterial(RoomWall::Back, RoomMaterial(RoomMaterialType::Marble));

                THEN("it returns the new materials per wall")
                {
                    REQUIRE(state.GetWallMaterial(RoomWall::Front) == RoomMaterial(RoomMaterialType::Metal));
                    REQUIRE(state.GetWallMaterial(RoomWall::Back) == RoomMaterial(RoomMaterialType::Marble));
                }

                THEN("other walls' materials remain unchanged")
                {
                    REQUIRE(state.GetWallMaterial(RoomWall::Bottom) == RoomMaterial(RoomMaterialType::Transparent));
                    REQUIRE(state.GetWallMaterial(RoomWall::Top) == RoomMaterial(RoomMaterialType::Transparent));
                    REQUIRE(state.GetWallMaterial(RoomWall::Left) == RoomMaterial(RoomMaterialType::Transparent));
                    REQUIRE(state.GetWallMaterial(RoomWall::Right) == RoomMaterial(RoomMaterialType::Transparent));
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
            const auto location = AM_V3(10, 20, 30);
            wrapper.SetLocation(location);

            THEN("it returns the new location")
            {
                REQUIRE(AM_EqV3(wrapper.GetLocation(), location));

                REQUIRE(AM_EqV3(wrapper.GetLocation(), state.GetLocation()));
            }

            AND_WHEN("an update occurs")
            {
                wrapper.Update();

                THEN("the location stays the same")
                {
                    REQUIRE(AM_EqV3(wrapper.GetLocation(), location));

                    REQUIRE(AM_EqV3(wrapper.GetLocation(), state.GetLocation()));
                }
            }
        }

        WHEN("the orientation changes")
        {
            const auto direction = AM_V3(1, 0, 0);
            const auto up = AM_V3(0, 0, 1);
            const auto orientation = Orientation(direction, up);
            wrapper.SetOrientation(orientation);

            THEN("it returns the new orientation")
            {
                REQUIRE(AM_EqV3(wrapper.GetDirection(), direction));
                REQUIRE(AM_EqV3(wrapper.GetUp(), up));

                REQUIRE(AM_EqV3(wrapper.GetDirection(), state.GetDirection()));
                REQUIRE(AM_EqV3(wrapper.GetUp(), state.GetUp()));

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
                REQUIRE(AM_EqV3(wrapper.GetDimensions(), AM_V3(1000, 500, 500)));
                REQUIRE(AM_EqV3(wrapper.GetDimensions(), state.GetDimensions()));
            }

            THEN("it computes the correct volume")
            {
                REQUIRE(wrapper.GetVolume() == 1000 * 500 * 500);
                REQUIRE(wrapper.GetVolume() == state.GetVolume());
            }

            THEN("it computes the correct surface areas")
            {
                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Back) == 1000 * 500);
                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Front) == wrapper.GetSurfaceArea(RoomWall::Back));

                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Top) == 1000 * 500);
                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Bottom) == wrapper.GetSurfaceArea(RoomWall::Top));

                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Left) == 500 * 500);
                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Right) == wrapper.GetSurfaceArea(RoomWall::Left));

                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Back) == state.GetSurfaceArea(RoomWall::Back));
                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Front) == state.GetSurfaceArea(RoomWall::Front));

                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Top) == state.GetSurfaceArea(RoomWall::Top));
                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Bottom) == state.GetSurfaceArea(RoomWall::Bottom));

                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Left) == state.GetSurfaceArea(RoomWall::Left));
                REQUIRE(wrapper.GetSurfaceArea(RoomWall::Right) == state.GetSurfaceArea(RoomWall::Right));
            }

            AND_WHEN("the dimensions change")
            {
                const auto dimensions = AM_V3(100, 100, 100);
                wrapper.SetDimensions(dimensions);

                THEN("it returns the new dimensions")
                {
                    REQUIRE(AM_EqV3(wrapper.GetDimensions(), dimensions));
                    REQUIRE(AM_EqV3(wrapper.GetDimensions(), state.GetDimensions()));
                }

                THEN("it compute the correct shape")
                {
                    BoxShape shape(50, 50, 50);
                    REQUIRE(wrapper.GetShape() == shape);
                    REQUIRE(wrapper.GetShape() == state.GetShape());
                }
            }
        }

        WHEN("the room materials changes")
        {
            SECTION("setting materials per wall")
            {
                wrapper.SetWallMaterial(RoomWall::Front, RoomMaterial(RoomMaterialType::Metal));
                wrapper.SetWallMaterial(RoomWall::Back, RoomMaterial(RoomMaterialType::Marble));

                THEN("it returns the new materials per wall")
                {
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Front) == RoomMaterial(RoomMaterialType::Metal));
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Back) == RoomMaterial(RoomMaterialType::Marble));
                }

                THEN("other walls' materials remain unchanged")
                {
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Bottom) == RoomMaterial(RoomMaterialType::Transparent));
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Top) == RoomMaterial(RoomMaterialType::Transparent));
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Left) == RoomMaterial(RoomMaterialType::Transparent));
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Right) == RoomMaterial(RoomMaterialType::Transparent));
                }
            }

            SECTION("setting materials for the entire room")
            {
                const auto material = RoomMaterial(RoomMaterialType::Glass);
                wrapper.SetAllWallMaterials(material);

                THEN("it returns the new materials for the entire room")
                {
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Front) == material);
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Back) == material);
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Top) == material);
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Bottom) == material);
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Left) == material);
                    REQUIRE(wrapper.GetWallMaterial(RoomWall::Right) == material);
                }
            }

            SECTION("setting all materials at once")
            {
                const std::vector<RoomMaterial> materials = {
                    RoomMaterial(RoomMaterialType::Metal), RoomMaterial(RoomMaterialType::Marble),
                    RoomMaterial(RoomMaterialType::Glass), RoomMaterial(RoomMaterialType::BrickPainted),
                    RoomMaterial(RoomMaterialType::Wood),  RoomMaterial(RoomMaterialType::FoamPanel)
                };

                wrapper.SetWallMaterials(materials[0], materials[1], materials[2], materials[3], materials[4], materials[5]);

                THEN("it returns the new materials for the entire room")
                {
                    for (size_t i = 0; i < kAmRoomSurfaceCount; ++i)
                    {
                        REQUIRE(wrapper.GetWallMaterial(static_cast<RoomWall>(i)) == materials[i]);
                    }
                }
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