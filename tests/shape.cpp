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

TEST_CASE("Sphere Shape Tests", "[sphere_shape][math][amplitude]")
{
    SphereShape shape(50);

    const AmVec3 inner = AM_V3(25, 25, 25);
    const AmVec3 outer = AM_V3(100, 100, 100);
    const AmVec3 center = AM_V3(0, 0, 0);
    const AmVec3 surfacePoint = AM_V3(0, 0, 50);

    THEN("it returns the correct radius")
    {
        REQUIRE(shape.GetRadius() == 50);
    }

    THEN("it computes the correct diameter")
    {
        REQUIRE(shape.GetDiameter() == 100);
    }

    WHEN("the radius changes")
    {
        shape.SetRadius(100);

        THEN("it returns the new radius")
        {
            REQUIRE(shape.GetRadius() == 100);
        }

        THEN("it computes the correct diameter")
        {
            REQUIRE(shape.GetDiameter() == 200);
        }
    }

    WHEN("the location changes")
    {
        const AmVec3 location = AM_V3(100, 100, 100);
        shape.SetLocation(location);

        THEN("it returns the new location")
        {
            REQUIRE(shape.GetLocation() == location);
        }
    }

    WHEN("the orientation changes")
    {
        const Orientation orientation = Orientation(AM_QFromAxisAngle_RH(AM_V3(0.5, 0.5, 0.5), AM_PI32));
        shape.SetOrientation(orientation);

        THEN("it returns the new orientation")
        {
            REQUIRE(std::memcmp(&shape.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
        }
    }

    THEN("it returns the shortest distance between a point and the shape")
    {
        REQUIRE(shape.GetShortestDistanceToEdge(inner) == shape.GetRadius() - AM_Len(inner));
        REQUIRE(shape.GetShortestDistanceToEdge(outer) == shape.GetRadius() - AM_Len(outer));
        REQUIRE(shape.GetShortestDistanceToEdge(surfacePoint) == 0.0f);
    }

    THEN("it detects inner and outer containment")
    {
        REQUIRE(shape.Contains(inner));
        REQUIRE_FALSE(shape.Contains(outer));
        REQUIRE(shape.Contains(surfacePoint));
    }

    GIVEN("another sphere shape")
    {
        SphereShape other(100);
        SphereShape some(50);
        SphereShape clone(50);
        clone.SetLocation(AM_V3(100, 100, 100));

        THEN("it can check for equality")
        {
            REQUIRE(shape == some);
            REQUIRE(shape != other);
            REQUIRE_FALSE(shape == clone);
        }
    }
}

TEST_CASE("Box Shape Tests", "[box_shape][math][amplitude]")
{
    BoxShape shape(50, 50, 50);

    const AmVec3 inner = AM_V3(-25, -25, -25);
    const AmVec3 outer = AM_V3(100, 100, 100);
    const AmVec3 center = AM_V3(0, 0, 0);
    const AmVec3 surfacePoint = AM_V3(0, 0, 50);

    THEN("it returns the correct dimensions")
    {
        REQUIRE(shape.GetHalfWidth() == 50);
        REQUIRE(shape.GetHalfHeight() == 50);
        REQUIRE(shape.GetHalfDepth() == 50);

        REQUIRE(shape.GetWidth() == 100);
        REQUIRE(shape.GetHeight() == 100);
        REQUIRE(shape.GetDepth() == 100);
    }

    WHEN("the dimensions change")
    {
        shape.SetHalfWidth(10);
        shape.SetHalfHeight(20);
        shape.SetHalfDepth(30);

        THEN("it returns the new dimensions")
        {
            REQUIRE(shape.GetHalfWidth() == 10);
            REQUIRE(shape.GetHalfHeight() == 20);
            REQUIRE(shape.GetHalfDepth() == 30);

            REQUIRE(shape.GetWidth() == 20);
            REQUIRE(shape.GetHeight() == 40);
            REQUIRE(shape.GetDepth() == 60);
        }
    }

    THEN("it returns the shortest distance between a point and the shape")
    {
        const auto corners = shape.GetCorners();

        REQUIRE(shape.GetShortestDistanceToEdge(inner) == 25.0f);
        REQUIRE(shape.GetShortestDistanceToEdge(outer) == -50.0f);
        REQUIRE(shape.GetShortestDistanceToEdge(center) == 50.0f);
        REQUIRE(shape.GetShortestDistanceToEdge(surfacePoint) == 0.0f);
    }

    THEN("it detects inner and outer containment")
    {
        REQUIRE(shape.Contains(inner));
        REQUIRE_FALSE(shape.Contains(outer));
        REQUIRE(shape.Contains(surfacePoint));
    }

    THEN("it detects the correct closest point to a shape")
    {
        const auto corners = shape.GetCorners();

        REQUIRE(shape.GetClosestPoint(inner) == inner);
        REQUIRE(shape.GetClosestPoint(outer) == corners[4]);
        REQUIRE(shape.GetClosestPoint(center) == center);
        REQUIRE(shape.GetClosestPoint(surfacePoint) == AM_V3(0, 0, 50));
    }

    THEN("it can check for equality")
    {
        BoxShape other(250, 250, 250);
        BoxShape some(50, 50, 50);
        BoxShape clone(AM_V3(100, 100, 100), AM_V3(100, 100, 100));

        REQUIRE(shape == some);
        REQUIRE(shape != other);
        REQUIRE_FALSE(shape == clone);
    }
}

TEST_CASE("Capsule Shape Tests", "[capsule_shape][math][amplitude]")
{
    CapsuleShape shape(50, 100);

    const AmVec3 inner = AM_V3(0, 0, 75);
    const AmVec3 outer = AM_V3(0, 150, 0);
    const AmVec3 center = AM_V3(0, 0, 0);
    const AmVec3 surfacePoint = AM_V3(50, 0, 50);

    THEN("it returns the correct radius")
    {
        REQUIRE(shape.GetRadius() == 50);
    }

    THEN("it returns the correct diameter")
    {
        REQUIRE(shape.GetDiameter() == 100);
    }

    THEN("it returns the correct half height")
    {
        REQUIRE(shape.GetHalfHeight() == 100);
    }

    THEN("it returns the correct height")
    {
        REQUIRE(shape.GetHeight() == 200);
    }

    WHEN("the radius changes")
    {
        shape.SetRadius(100);

        THEN("it returns the new radius")
        {
            REQUIRE(shape.GetRadius() == 100);
        }

        THEN("it returns the correct diameter")
        {
            REQUIRE(shape.GetDiameter() == 200);
        }
    }

    WHEN("the half height changes")
    {
        shape.SetHalfHeight(50);

        THEN("it returns the new half height")
        {
            REQUIRE(shape.GetHalfHeight() == 50);
        }

        THEN("it returns the correct height")
        {
            REQUIRE(shape.GetHeight() == 100);
        }
    }

    THEN("it returns the shortest distance between a point and the shape")
    {
        REQUIRE(shape.GetShortestDistanceToEdge(inner) == 25.0f);
        REQUIRE(shape.GetShortestDistanceToEdge(outer) == -100.0f);
        REQUIRE(shape.GetShortestDistanceToEdge(surfacePoint) == 0.0f);
    }

    THEN("it detects inner and outer containment")
    {
        REQUIRE(shape.Contains(inner));
        REQUIRE_FALSE(shape.Contains(outer));
        REQUIRE(shape.Contains(surfacePoint));
    }

    THEN("it can check for equality")
    {
        CapsuleShape other(50, 200);
        CapsuleShape some(50, 100);
        CapsuleShape clone(50, 100);
        clone.SetLocation(AM_V3(100, 100, 100));

        REQUIRE(shape == some);
        REQUIRE(shape != other);
        REQUIRE_FALSE(shape == clone);
    }
}

TEST_CASE("Cone Shape Tests", "[cone_shape][math][amplitude]")
{
    ConeShape shape(50, 100);

    const AmVec3 inner = AM_V3(0, 50, 0);
    const AmVec3 outer = AM_V3(0, 0, 150);
    const AmVec3 center = AM_V3(0, 0, 0);
    const AmVec3 surfacePoint = AM_V3(0, 100, 25);

    THEN("it returns the correct radius")
    {
        REQUIRE(shape.GetRadius() == 50);
    }

    THEN("it returns the correct diameter")
    {
        REQUIRE(shape.GetDiameter() == 100);
    }

    WHEN("the radius changes")
    {
        shape.SetRadius(100);

        THEN("it returns the new radius")
        {
            REQUIRE(shape.GetRadius() == 100);
        }

        THEN("it returns the correct diameter")
        {
            REQUIRE(shape.GetDiameter() == 200);
        }
    }

    WHEN("the height changes")
    {
        shape.SetHeight(200);

        THEN("it returns the new height")
        {
            REQUIRE(shape.GetHeight() == 200);
        }
    }

    THEN("it returns the shortest distance between a point and the shape")
    {
        REQUIRE(shape.GetShortestDistanceToEdge(inner) == 25.0f);
        REQUIRE(shape.GetShortestDistanceToEdge(outer) == -150.0f);
        REQUIRE(shape.GetShortestDistanceToEdge(surfacePoint) == 0.0f);
    }

    THEN("it detects inner and outer containment")
    {
        REQUIRE(shape.Contains(inner));
        REQUIRE_FALSE(shape.Contains(outer));
        REQUIRE(shape.Contains(surfacePoint));
    }

    THEN("it can check for equality")
    {
        ConeShape other(50, 200);
        ConeShape some(50, 100);
        ConeShape clone(50, 100);
        clone.SetLocation(AM_V3(100, 100, 100));

        REQUIRE(shape == some);
        REQUIRE(shape != other);
        REQUIRE_FALSE(shape == clone);
    }
}
