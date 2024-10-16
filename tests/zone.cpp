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

#include <Core/EntityInternalState.h>
#include <Core/ListenerInternalState.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Capsule Zone Tests", "[capsule_zone][math][amplitude]")
{
    CapsuleShape inner(25, 50);
    CapsuleShape outer(50, 100);
    CapsuleZone zone(&inner, &outer);

    WHEN("the location changes")
    {
        const AmVec3 newLocation = AM_V3(10, 20, 30);
        zone.SetLocation(newLocation);

        THEN("it returns the correct location")
        {
            REQUIRE(zone.GetLocation() == newLocation);
        }

        THEN("the inner and outer shapes are moved accordingly")
        {
            REQUIRE(inner.GetLocation() == newLocation);
            REQUIRE(outer.GetLocation() == newLocation);
        }
    }

    WHEN("the orientation changes")
    {
        const auto direction = AM_V3(0, 0, 1);
        const auto up = AM_V3(0, 1, 0);
        const auto newOrientation = Orientation(direction, up);
        zone.SetOrientation(newOrientation);

        THEN("it returns the correct orientation")
        {
            REQUIRE(std::memcmp(&zone.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("the inner and outer shapes are rotated accordingly")
        {
            REQUIRE(std::memcmp(&inner.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
            REQUIRE(std::memcmp(&outer.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("it computes the correct direction and up vectors")
        {
            REQUIRE(AM_EqV3(zone.GetDirection(), newOrientation.GetForward()));
            REQUIRE(AM_EqV3(zone.GetUp(), newOrientation.GetUp()));
        }
    }

    THEN("it computes the correct factors")
    {
        const AmVec3 inner = AM_V3(10, 10, 10);
        const AmVec3 outer = AM_V3(100, 20, 20);
        const AmVec3 middle = AM_V3(37.5f, 0, 0);

        REQUIRE(zone.GetFactor(inner) == 1.0f);
        REQUIRE(zone.GetFactor(outer) == 0.0f);
        REQUIRE(zone.GetFactor(middle) == 0.5f);
    }
}

TEST_CASE("Sphere Zone Tests", "[sphere_zone][math][amplitude]")
{
    SphereShape inner(50);
    SphereShape outer(100);
    SphereZone zone(&inner, &outer);

    WHEN("the location changes")
    {
        const AmVec3 newLocation = AM_V3(10, 20, 30);
        zone.SetLocation(newLocation);

        THEN("it returns the correct location")
        {
            REQUIRE(zone.GetLocation() == newLocation);
        }

        THEN("the inner and outer shapes are moved accordingly")
        {
            REQUIRE(inner.GetLocation() == newLocation);
            REQUIRE(outer.GetLocation() == newLocation);
        }
    }

    WHEN("the orientation changes")
    {
        const auto direction = AM_V3(0, 0, 1);
        const auto up = AM_V3(0, 1, 0);
        const auto newOrientation = Orientation(direction, up);
        zone.SetOrientation(newOrientation);

        THEN("it returns the correct orientation")
        {
            REQUIRE(std::memcmp(&zone.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("the inner and outer shapes are rotated accordingly")
        {
            REQUIRE(std::memcmp(&inner.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
            REQUIRE(std::memcmp(&outer.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("it computes the correct direction and up vectors")
        {
            REQUIRE(AM_EqV3(zone.GetDirection(), direction));
            REQUIRE(AM_EqV3(zone.GetUp(), up));
        }
    }

    THEN("it compute correct factors")
    {
        const AmVec3 inner = AM_V3(25, 25, 25);
        const AmVec3 outer = AM_V3(175, 175, 175);
        const AmVec3 middle = AM_V3(0, 75, 0);

        REQUIRE(zone.GetFactor(inner) == 1);
        REQUIRE(zone.GetFactor(outer) == 0);
        REQUIRE(zone.GetFactor(middle) == 0.5f);
    }
}

TEST_CASE("Box Zone Tests", "[box_zone][math][amplitude]")
{
    BoxShape inner(50, 50, 50);
    BoxShape outer(100, 100, 100);
    BoxZone zone(&inner, &outer);

    WHEN("the location changes")
    {
        const AmVec3 newLocation = AM_V3(10, 20, 30);
        zone.SetLocation(newLocation);

        THEN("it returns the correct location")
        {
            REQUIRE(zone.GetLocation() == newLocation);
        }

        THEN("the inner and outer shapes are moved accordingly")
        {
            REQUIRE(inner.GetLocation() == newLocation);
            REQUIRE(outer.GetLocation() == newLocation);
        }
    }

    WHEN("the orientation changes")
    {
        const auto direction = AM_V3(0, 0, 1);
        const auto up = AM_V3(0, 1, 0);
        const auto newOrientation = Orientation(direction, up);
        zone.SetOrientation(newOrientation);

        THEN("it returns the correct orientation")
        {
            REQUIRE(std::memcmp(&zone.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("the inner and outer shapes are rotated accordingly")
        {
            REQUIRE(std::memcmp(&inner.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
            REQUIRE(std::memcmp(&outer.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("it computes the correct direction and up vectors")
        {
            REQUIRE(AM_EqV3(zone.GetDirection(), direction));
            REQUIRE(AM_EqV3(zone.GetUp(), up));
        }
    }

    THEN("it compute correct factors")
    {
        const AmVec3 inner = AM_V3(25, 25, 25);
        const AmVec3 outer = AM_V3(175, 175, 175);
        const AmVec3 middle = AM_V3(87.5f, 87.5f, 87.5f);

        REQUIRE(zone.GetFactor(inner) == 1);
        REQUIRE(zone.GetFactor(outer) == 0);
        REQUIRE(zone.GetFactor(middle) == 0.25f);
    }
}

TEST_CASE("Cone Zone Tests", "[cone_zone][math][amplitude]")
{
    ConeShape inner(50, 50);
    ConeShape outer(100, 100);
    ConeZone zone(&inner, &outer);

    WHEN("the location changes")
    {
        const AmVec3 newLocation = AM_V3(10, 20, 30);
        zone.SetLocation(newLocation);

        THEN("it returns the correct location")
        {
            REQUIRE(zone.GetLocation() == newLocation);
        }

        THEN("the inner and outer shapes are moved accordingly")
        {
            REQUIRE(inner.GetLocation() == newLocation);
            REQUIRE(outer.GetLocation() == newLocation);
        }
    }

    WHEN("the orientation changes")
    {
        const auto direction = AM_V3(0, 0, 1);
        const auto up = AM_V3(0, 1, 0);
        const auto newOrientation = Orientation(direction, up);
        zone.SetOrientation(newOrientation);

        THEN("it returns the correct orientation")
        {
            REQUIRE(std::memcmp(&zone.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("the inner and outer shapes are rotated accordingly")
        {
            REQUIRE(std::memcmp(&inner.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
            REQUIRE(std::memcmp(&outer.GetOrientation(), &newOrientation, sizeof(Orientation)) == 0);
        }

        THEN("it computes the correct direction and up vectors")
        {
            REQUIRE(AM_EqV3(zone.GetDirection(), direction));
            REQUIRE(AM_EqV3(zone.GetUp(), up));
        }
    }

    THEN("it compute correct factors")
    {
        const AmVec3 inner = AM_V3(25, 25, 0);
        const AmVec3 outer = AM_V3(175, 175, 0);
        const AmVec3 middle1 = AM_V3(0.0f, 87.5f, 0);
        const AmVec3 middle2 = AM_V3(25.0f, 75.0f, 0);

        REQUIRE(zone.GetFactor(inner) == 1);
        REQUIRE(zone.GetFactor(outer) == 0);
        REQUIRE(zone.GetFactor(middle1) == 0.25f);
        REQUIRE(zone.GetFactor(middle2) == 0.5f);
    }
}