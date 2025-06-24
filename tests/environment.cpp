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

#include <Core/EnvironmentInternalState.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Environment Tests", "[environment][core][amplitude]")
{
    EnvironmentInternalState state;
    state.SetId(1);

    fplutil::intrusive_list environment_list(&EnvironmentInternalState::node);
    environment_list.push_back(state);

    auto inner = std::make_shared<SphereShape>(10);
    auto outer = std::make_shared<SphereShape>(20);
    auto zone = std::make_shared<SphereZone>(inner, outer);

    state.SetZone(zone);
    REQUIRE(state.GetZone() == zone);

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

                REQUIRE(zone->GetLocation() == location);
            }

            AND_WHEN("an update occurs")
            {
                state.Update();

                THEN("the location stays the same")
                {
                    REQUIRE(state.GetLocation() == location);

                    REQUIRE(zone->GetLocation() == location);
                }
            }
        }

        WHEN("the orientation changes")
        {
            constexpr AmVector3 direction = kVector3UnitX;
            constexpr AmVector3 up = kVector3UnitZ;

            const auto orientation = Orientation(direction, up);
            state.SetOrientation(orientation);

            THEN("it returns the new orientation")
            {
                REQUIRE(state.GetDirection() == direction);
                REQUIRE(state.GetUp() == up);

                REQUIRE(zone->GetDirection() == direction);
                REQUIRE(zone->GetUp() == up);

                REQUIRE(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                REQUIRE(std::memcmp(&zone->GetOrientation(), &orientation, sizeof(Orientation)) == 0);
            }
        }

        WHEN("the zone changes")
        {
            THEN("it returns the new zone")
            {
                REQUIRE(state.GetZone() == zone);
            }
        }
    }

    SECTION("can be used with a wrapper")
    {
        Environment wrapper(&state);
        REQUIRE(wrapper.GetState() == &state);

        wrapper.SetZone(zone);
        REQUIRE(wrapper.GetZone() == zone);

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

                REQUIRE(state.GetLocation() == zone->GetLocation());
            }

            AND_WHEN("an update occurs")
            {
                wrapper.Update();

                THEN("the location stays the same")
                {
                    REQUIRE(wrapper.GetLocation() == location);

                    REQUIRE(wrapper.GetLocation() == state.GetLocation());

                    REQUIRE(state.GetLocation() == zone->GetLocation());
                }
            }
        }

        WHEN("the orientation changes")
        {
            constexpr AmVector3 direction = { 1, 0, 0 };
            constexpr AmVector3 up = { 0, 0, 1 };
            wrapper.SetOrientation(Orientation(direction, up));

            THEN("it returns the new orientation")
            {
                REQUIRE(wrapper.GetDirection() == direction);
                REQUIRE(wrapper.GetUp() == up);

                REQUIRE(wrapper.GetDirection() == state.GetDirection());
                REQUIRE(wrapper.GetUp() == state.GetUp());

                REQUIRE(state.GetDirection() == zone->GetDirection());
                REQUIRE(state.GetUp() == zone->GetUp());
            }
        }

        WHEN("the zone changes")
        {
            THEN("it returns the new zone")
            {
                REQUIRE(wrapper.GetZone() == zone);

                REQUIRE(wrapper.GetZone() == state.GetZone());
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
        Environment wrapper2(nullptr);
        REQUIRE_FALSE(wrapper2.Valid());

        Environment wrapper3;
        REQUIRE_FALSE(wrapper3.Valid());
    }
}