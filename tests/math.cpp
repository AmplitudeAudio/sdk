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

#include <Math/LinearAlgebra.h>
#include <Utils/Utils.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Barycentric Coordinate Tests", "[barycentric_coordinates][math][amplitude]")
{
    constexpr AmVector3 point1 = { 1, 2, 1 };
    constexpr AmVector3 point2 = { 0, 0, 0 };
    constexpr AmVector3 point3 = { 2, 1, 2 };

    constexpr auto center = kVector3One;
    constexpr auto triangle = Triangle{ point1, point2, point3 };

    const auto i = Sub(point1, center);
    const auto j = Sub(point2, center);
    const auto k = Cross(i, j);

    constexpr auto rayOrigin = center;
    const auto rayDirection = k;

    SECTION("can compute the barycentric coordinates of a point inside the triangle")
    {
        BarycentricCoordinates coordinates(center, triangle);

        REQUIRE(coordinates.IsValid());

        REQUIRE(coordinates.m_U + coordinates.m_V + coordinates.m_W == 1.0f);

        REQUIRE((coordinates.m_U - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_V - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_W - 1.0f / 3.0f) < kEpsilon);
    }

    SECTION("can compute the barycentric coordinates of a ray-triangle intersection")
    {
        BarycentricCoordinates coordinates;
        REQUIRE(BarycentricCoordinates::RayTriangleIntersection(rayOrigin, rayDirection, triangle, coordinates));

        REQUIRE(coordinates.IsValid());

        REQUIRE(coordinates.m_U + coordinates.m_V + coordinates.m_W == 1.0f);

        REQUIRE((coordinates.m_U - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_V - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_W - 1.0f / 3.0f) < kEpsilon);
    }
}

TEST_CASE("Cartesian Coordinate System Tests", "[cartesian_coordinate_system][math][amplitude]")
{
    GIVEN("a right-handed z-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::RightHandedZUp();

        THEN("it have the correct axes")
        {
            REQUIRE(coordinateSystem.GetRightVector() == kVector3UnitX);
            REQUIRE(coordinateSystem.GetUpVector() == kVector3UnitZ);
            REQUIRE(coordinateSystem.GetForwardVector() == kVector3UnitY);
        }
    }

    GIVEN("a right-handed y-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::RightHandedYUp();

        THEN("it have the correct axes")
        {
            REQUIRE(coordinateSystem.GetRightVector() == kVector3UnitX);
            REQUIRE(coordinateSystem.GetUpVector() == kVector3UnitY);
            REQUIRE(coordinateSystem.GetForwardVector() == AmVector3{ 0, 0, -1 });
        }
    }

    GIVEN("a left-handed z-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::LeftHandedZUp();

        THEN("it have the correct axes")
        {
            REQUIRE(coordinateSystem.GetRightVector() == kVector3UnitX);
            REQUIRE(coordinateSystem.GetUpVector() == kVector3UnitZ);
            REQUIRE(coordinateSystem.GetForwardVector() == AmVector3{ 0, -1, 0 });
        }
    }

    GIVEN("a left-handed y-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::LeftHandedYUp();

        THEN("it have the correct axes")
        {
            REQUIRE(coordinateSystem.GetRightVector() == kVector3UnitX);
            REQUIRE(coordinateSystem.GetUpVector() == kVector3UnitY);
            REQUIRE(coordinateSystem.GetForwardVector() == kVector3UnitZ);
        }
    }

    GIVEN("a Cartesian coordinate system with arbitrary axes")
    {
        constexpr auto rightVector = CartesianCoordinateSystem::Axis::NegativeX;
        constexpr auto upVector = CartesianCoordinateSystem::Axis::PositiveY;
        constexpr auto forwardVector = CartesianCoordinateSystem::Axis::NegativeZ;

        const CartesianCoordinateSystem coordinateSystem(rightVector, forwardVector, upVector);

        THEN("it have the correct axes")
        {
            REQUIRE(coordinateSystem.GetRightVector() == CartesianCoordinateSystem::GetVector(rightVector));
            REQUIRE(coordinateSystem.GetUpVector() == CartesianCoordinateSystem::GetVector(upVector));
            REQUIRE(coordinateSystem.GetForwardVector() == CartesianCoordinateSystem::GetVector(forwardVector));
        }
    }

    SECTION("coordinate system conversion")
    {
        const auto from = CartesianCoordinateSystem::Default();
        const auto to = CartesianCoordinateSystem::AmbiX();

        GIVEN("a point in the default coordinate system")
        {
            constexpr auto point = AmVector3{ 1, 2, 3 };

            WHEN("converted to AmbiX coordinate system")
            {
                const auto convertedPoint = CartesianCoordinateSystem::Convert(point, from, to);

                THEN("it should have the same position in the AmbiX coordinate system")
                {
                    REQUIRE(convertedPoint == AmVector3{ 2, -1, 3 });
                }

                THEN("it should convert back to the original coordinate system")
                {
                    const auto convertedBackPoint = CartesianCoordinateSystem::Convert(convertedPoint, to, from);

                    REQUIRE(convertedBackPoint == point);
                }

                THEN("it should match the point using the Converter API")
                {
                    const auto converter = CartesianCoordinateSystem::Converter(from, to);
                    const auto convertedPoint2 = converter.Forward(point);

                    REQUIRE(convertedPoint2 == convertedPoint);

                    AND_THEN("it should convert back to the original coordinate system using the Converter API")
                    {
                        const auto convertedBackPoint2 = converter.Backward(convertedPoint);

                        REQUIRE(convertedBackPoint2 == point);
                    }
                }
            }
        }

        GIVEN("a quaternion rotation in the default coordinate system")
        {
            const auto rotation = FromAxisAngle(kVector3UnitY, AM_DegToRad * 45.0f);

            WHEN("converted to AmbiX coordinate system")
            {
                const auto convertedRotation = CartesianCoordinateSystem::Convert(rotation, from, to);

                THEN("it should have the same rotation in the AmbiX coordinate system")
                {
                    REQUIRE(convertedRotation.xyz == AmVector3{ rotation.y, -rotation.x, rotation.z });
                    REQUIRE(convertedRotation.w == rotation.w);
                }

                THEN("it should convert back to the original coordinate system")
                {
                    const auto convertedBackRotation = CartesianCoordinateSystem::Convert(convertedRotation, to, from);

                    REQUIRE(convertedBackRotation.xyz == rotation.xyz);
                    REQUIRE(convertedBackRotation.w == rotation.w);
                }

                THEN("it should match the rotation using the Converter API")
                {
                    const auto converter = CartesianCoordinateSystem::Converter(from, to);
                    const auto convertedRotation2 = converter.Forward(rotation);

                    REQUIRE(convertedRotation2.xyz == convertedRotation.xyz);
                    REQUIRE(convertedRotation2.w == convertedRotation.w);

                    AND_THEN("it should convert back to the original coordinate system using the Converter API")
                    {
                        const auto convertedBackRotation2 = converter.Backward(convertedRotation);

                        REQUIRE(convertedBackRotation2.xyz == rotation.xyz);
                        REQUIRE(convertedBackRotation2.w == rotation.w);
                    }
                }
            }
        }

        GIVEN("a scalar in the default coordinate system")
        {
            constexpr auto scalar = 5.0f;

            WHEN("converted to AmbiX coordinate system")
            {
                const auto convertedScalar = CartesianCoordinateSystem::Convert(scalar, from, to);

                THEN("it should have the same scalar in the AmbiX coordinate system")
                {
                    REQUIRE(convertedScalar == scalar);
                }

                THEN("it should convert back to the original coordinate system")
                {
                    const auto convertedBackScalar = CartesianCoordinateSystem::Convert(convertedScalar, to, from);

                    REQUIRE(convertedBackScalar == scalar);
                }

                THEN("it should match the scalar using the Converter API")
                {
                    const auto converter = CartesianCoordinateSystem::Converter(from, to);
                    const auto convertedScalar2 = converter.Forward(scalar);

                    REQUIRE(convertedScalar2 == convertedScalar);

                    AND_THEN("it should convert back to the original coordinate system using the Converter API")
                    {
                        const auto convertedBackScalar2 = converter.Backward(convertedScalar);

                        REQUIRE(convertedBackScalar2 == scalar);
                    }
                }
            }
        }
    }
}

TEST_CASE("Spherical Position Tests", "[spherical_position][math][amplitude]")
{
    GIVEN("a spherical position")
    {
        SphericalPosition position(AM_DegToRad * 45.0f, AM_DegToRad * 30.0f, 5.0f);

        THEN("it should store the correct spherical coordinates")
        {
            REQUIRE(position.GetAzimuth() == AM_DegToRad * 45.0f);
            REQUIRE(position.GetElevation() == AM_DegToRad * 30.0f);
            REQUIRE(position.GetRadius() == 5.0f);
        }

        THEN("it can convert to Cartesian coordinates")
        {
            const auto cartesianPosition = position.ToCartesian();

            REQUIRE(cartesianPosition.x == +5.0f * std::cos(position.GetElevation()) * std::cos(position.GetAzimuth()));
            REQUIRE(cartesianPosition.y == -5.0f * std::cos(position.GetElevation()) * std::sin(position.GetAzimuth()));
            REQUIRE(cartesianPosition.z == +5.0f * std::sin(position.GetElevation()));
        }

        THEN("it can flip azimuth")
        {
            const auto flippedPosition = position.FlipAzimuth();

            REQUIRE(flippedPosition.GetAzimuth() == -45.0f * AM_DegToRad);
            REQUIRE(flippedPosition.GetElevation() == position.GetElevation());
            REQUIRE(flippedPosition.GetRadius() == position.GetRadius());
        }

        THEN("it can change azimuth, elevation and radius")
        {
            position.SetAzimuth(AM_DegToRad * 60.0f);
            position.SetElevation(AM_DegToRad * 45.0f);
            position.SetRadius(10.0f);

            REQUIRE(position.GetAzimuth() == AM_DegToRad * 60.0f);
            REQUIRE(position.GetElevation() == AM_DegToRad * 45.0f);
            REQUIRE(position.GetRadius() == 10.0f);
        }

        THEN("it can be rotated")
        {
            const auto rotation = FromAxisAngle(kVector3UnitZ, AM_DegToRad * 90.0f);
            const auto rotatedPosition = position.Rotate(rotation);

            const auto rotatedPosition2 = SphericalPosition::FromWorldSpace(RotateVector(position.ToCartesian(), rotation));

            REQUIRE(rotatedPosition.GetAzimuth() == rotatedPosition2.GetAzimuth());
            REQUIRE(rotatedPosition.GetElevation() == rotatedPosition2.GetElevation());
            REQUIRE(rotatedPosition.GetRadius() == rotatedPosition2.GetRadius());
        }

        SECTION("equality comparison")
        {
            const auto otherPosition = SphericalPosition(AM_DegToRad * 45.0f, AM_DegToRad * 30.0f, 5.0f);

            THEN("it should be equal to itself")
            {
                REQUIRE(position == position);
            }

            THEN("it should be equal to another spherical position with the same coordinates")
            {
                REQUIRE(position == otherPosition);
            }

            THEN("it should not be equal to another spherical position with different coordinates")
            {
                const auto differentPosition = SphericalPosition(AM_DegToRad * 60.0f, AM_DegToRad * 45.0f, 5.0f);

                REQUIRE(position != differentPosition);
            }
        }
    }

    GIVEN("a cartesian position")
    {
        constexpr AmVector3 cartesianPosition = { 5.0f, 3.0f, 4.0f };

        THEN("it can convert to spherical coordinates in world space")
        {
            const auto sphericalPosition = SphericalPosition::FromWorldSpace(cartesianPosition);

            REQUIRE(sphericalPosition.GetAzimuth() == -std::atan2(cartesianPosition.y, cartesianPosition.x));
            REQUIRE(sphericalPosition.GetElevation() == std::atan2(cartesianPosition.z, Length(cartesianPosition.xy)));
            REQUIRE(sphericalPosition.GetRadius() == Length(cartesianPosition));
        }

        THEN("it can convert to spherical coordinates in AmbiX space")
        {
            const auto sphericalPosition = SphericalPosition::ForHRTF(cartesianPosition);

            REQUIRE(sphericalPosition.GetAzimuth() == 90.0f * AM_DegToRad - std::atan2(cartesianPosition.y, cartesianPosition.z));
            REQUIRE(sphericalPosition.GetElevation() == std::atan2(cartesianPosition.z, Length(cartesianPosition.xy)));
            REQUIRE(sphericalPosition.GetRadius() == Length(cartesianPosition));
        }
    }

    GIVEN("values in degrees")
    {
        constexpr auto azimuth = 45.0f;
        constexpr auto elevation = 30.0f;
        constexpr auto radius = 5.0f;

        THEN("it should store the correct spherical coordinates")
        {
            const auto sphericalPosition = SphericalPosition::FromDegrees(azimuth, elevation, radius);

            REQUIRE(sphericalPosition.GetAzimuth() == azimuth * AM_DegToRad);
            REQUIRE(sphericalPosition.GetElevation() == elevation * AM_DegToRad);
            REQUIRE(sphericalPosition.GetRadius() == radius);
        }
    }
}

TEST_CASE("Curve Tests", "[curve][math][amplitude]")
{
    SECTION("curve points")
    {
        CurvePoint a{ 0.5, 1.0f };
        CurvePoint b{ 1.0, 2.0f };
        CurvePoint c{ 0.5, 1.0f };

        THEN("it should be equal to itself")
        {
            REQUIRE(a == a);
        }

        THEN("it should be equal to another curve point with the same coordinates")
        {
            REQUIRE(a == c);
        }

        THEN("it should not be equal to another curve point with different coordinates")
        {
            REQUIRE(a != b);
            REQUIRE(b != c);
        }
    }

    SECTION("curve parts")
    {
        CurvePart part1;

        CurvePoint zero{ 0.0, 0.0f };
        CurvePoint one{ 1.0, 1.0f };

        THEN("it cannot be initialized with a null definition")
        {
            part1.Initialize(nullptr);
            REQUIRE(part1.GetStart() == zero);
            REQUIRE(part1.GetEnd() == zero);
        }

        THEN("it can receive a start point")
        {
            part1.SetStart(one);
            REQUIRE(part1.GetStart() == one);
        }

        THEN("it can receive an end point")
        {
            part1.SetEnd(zero);
            REQUIRE(part1.GetEnd() == zero);
        }

        THEN("it can receive a fader")
        {
            part1.SetFader("Linear");
            REQUIRE(part1.GetFader() != nullptr);

            AND_THEN("it cannot use an invalid fader")
            {
                part1.SetFader("Invalid");
                REQUIRE(part1.GetFader() == nullptr);
            }
        }

        THEN("it can get the right values")
        {
            part1.SetFader("Linear");
            part1.SetStart(zero);
            part1.SetEnd(one);

            for (AmReal64 t = 0.0; t <= 1.0; t += 0.1)
            {
                const AmReal32 value = part1.Get(t);
                REQUIRE(value - t < kEpsilon);
            }
        }
    }

    SECTION("curve")
    {
        CurvePoint zero{ 0.0, 0.0f };
        CurvePoint middle{ 0.5, 1.0f };
        CurvePoint one{ 1.0, 0.0f };

        CurvePart part1;
        part1.SetStart(zero);
        part1.SetEnd(middle);
        part1.SetFader("Linear");

        CurvePart part2;
        part2.SetStart(middle);
        part2.SetEnd(one);
        part2.SetFader("Linear");

        Curve curve;

        THEN("cannot be initialized with a null definition")
        {
            curve.Initialize(nullptr);

            REQUIRE(curve.Get(0.0) == 0.0f);
        }

        THEN("can be initialized with curve parts")
        {
            const std::vector<CurvePart> parts = { part1, part2 };
            curve.Initialize(parts);

            REQUIRE(curve.Get(0.0) - 0.0f < kEpsilon);
            REQUIRE(curve.Get(0.5) - 1.0f < kEpsilon);
            REQUIRE(curve.Get(1.0) - 0.0f < kEpsilon);
        }

        THEN("can get the right values")
        {
            curve.Initialize({ part1, part2 });

            for (AmReal64 t = 0.0; t <= 0.5; t += 0.1)
            {
                const AmReal32 value = curve.Get(t);
                REQUIRE(value - (t * 2) < kEpsilon);
            }

            for (AmReal64 t = 0.5; t <= 1.0; t += 0.1)
            {
                const AmReal32 value = curve.Get(t);
                REQUIRE(value - 2 - 2 * t < kEpsilon);
            }
        }

        THEN("cannot get values outside the range")
        {
            curve.Initialize({ part1, part2 });

            REQUIRE(curve.Get(-1.0) == 0.0f);
            REQUIRE(curve.Get(2.0) == 0.0f);
        }
    }
}

TEST_CASE("Utilities Tests", "[utilities][math][amplitude]")
{
    THEN("dithering always works")
    {
        // This is only to increase coverage, as the dithering function is a simple function that always returns a value
        AmDitherReal32(1.0f / INT16_MIN, 1.0f / INT16_MAX);
    }

    SECTION("floating-point to fixed-point conversion")
    {
        THEN("it can convert floating-point audio sample to fixed-point")
        {
            constexpr AmReal32 value = 0.5f;
            const AmInt32 fixedPointValue = AmFloatToFixedPoint(value);
            REQUIRE(std::abs(fixedPointValue - 16384) < kEpsilon);
        }

        THEN("it can convert 16-bit integer to 32-bit floating-point")
        {
            constexpr AmInt16 fxp = 16384;
            const AmReal32 value = AmInt16ToReal32(fxp);
            REQUIRE(std::abs(value - 0.5f) < kEpsilon);
        }

        THEN("it can convert 32-bit integer to 32-bit floating-point")
        {
            constexpr AmInt32 value = 16384;
            const AmReal32 fxp = AmInt32ToReal32(value);
            REQUIRE(std::abs(fxp - 0.5f) < kEpsilon);
        }

        THEN("it can convert 32-bit floating-point to 16-bit integer")
        {
            constexpr AmReal32 value = 0.5f;

            const AmInt16 fxp1 = AmReal32ToInt16(value, false);
            REQUIRE((fxp1 - 16384) < kEpsilon);

            const AmInt16 fxp2 = AmReal32ToInt16(value, true);
            REQUIRE((fxp2 - 16384) < kEpsilon);

            REQUIRE(std::abs(fxp1 - fxp2) < kEpsilon);
        }
    }

    SECTION("catmull_rom")
    {
        constexpr AmReal32 p1 = 0.0f;
        constexpr AmReal32 p2 = 1.0f;
        constexpr AmReal32 p3 = 2.0f;
        constexpr AmReal32 p4 = 3.0f;

        REQUIRE(CatmullRom(0.0f, p1, p2, p3, p4) == 1.0f);
        REQUIRE(CatmullRom(1.0f, p1, p2, p3, p4) == 2.0f);
        REQUIRE(CatmullRom(0.5f, p1, p2, p3, p4) == 1.5f);
    }

    SECTION("doppler factor")
    {
        constexpr AmReal32 soundSpeed = 343.0f;
        constexpr AmVector3 source = { 10.0f, 25.0f, 1.0f };
        constexpr AmVector3 listener = { 0.0f, 0.0f, 0.0f };

        const AmReal32 dopplerFactor = ComputeDopplerFactor(Sub(source, listener), source, listener, soundSpeed, 1.0f);

        REQUIRE(std::abs(dopplerFactor - 0.927166343f) < kEpsilon);
    }

    SECTION("next power of two")
    {
        REQUIRE(NextPowerOf2(1) == 1);
        REQUIRE(NextPowerOf2(2) == 2);
        REQUIRE(NextPowerOf2(3) == 4);
        REQUIRE(NextPowerOf2(16) == 16);
        REQUIRE(NextPowerOf2(250) == 256);
        REQUIRE(NextPowerOf2(4000) == 4096);
        REQUIRE(NextPowerOf2(1024) == 1024);
        REQUIRE(NextPowerOf2(2024) == 2048);
        REQUIRE(NextPowerOf2(4096) == 4096);
    }

    SECTION("integer pow")
    {
        REQUIRE(IntegerPow(2, 0) == 1);
        REQUIRE(IntegerPow(2.5, 1) == 2.5);
        REQUIRE(IntegerPow(3.1f, 2) == 9.61f);
        REQUIRE(IntegerPow(2, 10) == 1024);
    }

    SECTION("gcd")
    {
        REQUIRE(FindGCD(12, 18) == 6);
        REQUIRE(FindGCD(20, 30) == 10);
        REQUIRE(FindGCD(48, 144) == 48);
        REQUIRE(FindGCD(-100, 200) == 100);
        REQUIRE(FindGCD(0, 200) == 200);
    }
}

TEST_CASE("Orientation Tests", "[orientation][math][amplitude]")
{
    SECTION("Zero orientation")
    {
        const auto zero = Orientation::Zero();

        // Test that Zero() creates an orientation with zero angles
        REQUIRE(zero.GetYaw() == 0.0f);
        REQUIRE(zero.GetPitch() == 0.0f);
        REQUIRE(zero.GetRoll() == 0.0f);

        // Test forward and up vectors for zero orientations
        REQUIRE(zero.GetForward() == kVector3UnitY);
        REQUIRE(zero.GetUp() == kVector3UnitZ);
    }

    SECTION("Construction from Euler angles")
    {
        constexpr AmReal32 yaw = AM_DegToRad * 45.0f;
        constexpr AmReal32 pitch = AM_DegToRad * 30.0f;
        constexpr AmReal32 roll = AM_DegToRad * 15.0f;

        const Orientation orientation(yaw, pitch, roll);

        THEN("it should store the correct Euler angles")
        {
            REQUIRE(std::abs(orientation.GetYaw() - yaw) < kEpsilon);
            REQUIRE(std::abs(orientation.GetPitch() - pitch) < kEpsilon);
            REQUIRE(std::abs(orientation.GetRoll() - roll) < kEpsilon);
        }

        THEN("it should compute forward and up vectors")
        {
            const auto forward = orientation.GetForward();
            const auto up = orientation.GetUp();

            // Forward and up vectors should be normalized
            REQUIRE(std::abs(Length(forward) - 1.0f) < kEpsilon);
            REQUIRE(std::abs(Length(up) - 1.0f) < kEpsilon);

            // Forward and up vectors should be perpendicular
            REQUIRE(std::abs(Dot(forward, up)) < kEpsilon);

            // Expected values for yaw=45°, pitch=30°, roll=15°
            // Computed from rotation matrix R = Rz(45°) * Ry(30°) * Rx(15°)
            constexpr AmVector3 expectedForward = {
                0.3535534f, // Forward X component
                0.6123724f, // Forward Y component
                -0.5f // Forward Z component
            };

            constexpr AmVector3 expectedUp = {
                -0.1830127f, // Up X component
                0.1830127f, // Up Y component
                0.9659258f // Up Z component
            };

            // Verify forward vector components with appropriate tolerance
            REQUIRE(std::abs(forward.x - expectedForward.x) < kEpsilon);
            REQUIRE(std::abs(forward.y - expectedForward.y) < kEpsilon);
            REQUIRE(std::abs(forward.z - expectedForward.z) < kEpsilon);

            // Verify up vector components with appropriate tolerance
            REQUIRE(std::abs(up.x - expectedUp.x) < kEpsilon);
            REQUIRE(std::abs(up.y - expectedUp.y) < kEpsilon);
            REQUIRE(std::abs(up.z - expectedUp.z) < kEpsilon);
        }

        THEN("it should compute ZYZ Euler angles")
        {
            // Alpha, beta, gamma should be computed
            const auto alpha = orientation.GetAlpha();
            const auto beta = orientation.GetBeta();
            const auto gamma = orientation.GetGamma();

            // These values should be within valid ranges
            REQUIRE(alpha >= -AM_PI32);
            REQUIRE(alpha <= AM_PI32);
            REQUIRE(beta >= 0.0f);
            REQUIRE(beta <= AM_PI32);
            REQUIRE(gamma >= -AM_PI32);
            REQUIRE(gamma <= AM_PI32);
        }

        THEN("it should compute a valid quaternion")
        {
            const auto quaternion = orientation.GetQuaternion();

            // Quaternion should be normalized
            const AmReal32 length =
                std::sqrt(AM_SQUARE(quaternion.w) + AM_SQUARE(quaternion.z) + AM_SQUARE(quaternion.y) + AM_SQUARE(quaternion.z));
            REQUIRE(std::abs(length - 1.0f) < kEpsilon);
        }
    }

    SECTION("Construction from forward and up vectors")
    {
        constexpr AmVector3 forward = kVector3UnitX;
        constexpr AmVector3 up = kVector3UnitZ;

        const Orientation orientation(forward, up);

        THEN("it should normalize and store the vectors")
        {
            const auto storedForward = orientation.GetForward();
            const auto storedUp = orientation.GetUp();

            // Vectors should be normalized
            REQUIRE(std::abs(Length(storedForward) - 1.0f) < kEpsilon);
            REQUIRE(std::abs(Length(storedUp) - 1.0f) < kEpsilon);

            // Should match normalized input vectors
            REQUIRE(storedForward == Normalize(forward));
            REQUIRE(storedUp == Normalize(up));
        }

        THEN("it should compute corresponding Euler angles")
        {
            // ZYX angles should be computed from the vectors
            const auto yaw = orientation.GetYaw();
            const auto pitch = orientation.GetPitch();
            const auto roll = orientation.GetRoll();

            // Reconstructed vectors from angles should match the original
            const Orientation reconstructed(yaw, pitch, roll);
            const auto reconstructedForward = reconstructed.GetForward();
            const auto reconstructedUp = reconstructed.GetUp();

            REQUIRE(reconstructedForward == orientation.GetForward());
            REQUIRE(reconstructedUp == orientation.GetUp());
        }
    }

    SECTION("Construction from quaternion")
    {
        // Create a quaternion representing a 90-degree rotation around Z-axis
        const AmQuaternion quaternion = FromAxisAngle(kVector3UnitZ, AM_DegToRad * 90.0f);

        const Orientation orientation(quaternion);

        THEN("it should store the quaternion")
        {
            const AmQuaternion storedQuaternion = orientation.GetQuaternion();

            REQUIRE(std::abs(storedQuaternion.w - quaternion.w) < kEpsilon);
            REQUIRE(std::abs(storedQuaternion.x - quaternion.x) < kEpsilon);
            REQUIRE(std::abs(storedQuaternion.y - quaternion.y) < kEpsilon);
            REQUIRE(std::abs(storedQuaternion.z - quaternion.z) < kEpsilon);
        }

        THEN("it should compute corresponding forward and up vectors")
        {
            const auto forward = orientation.GetForward();
            const auto up = orientation.GetUp();

            // Vectors should be normalized
            REQUIRE(std::abs(Length(forward) - 1.0f) < kEpsilon);
            REQUIRE(std::abs(Length(up) - 1.0f) < kEpsilon);

            // For a 90-degree Z rotation, forward should be rotated from unit Y
            const AmVector3 expectedForward = RotateVector(kVector3UnitY, quaternion);
            const AmVector3 expectedUp = RotateVector(kVector3UnitZ, quaternion);

            REQUIRE(forward == expectedForward);
            REQUIRE(up == expectedUp);
        }
    }

    SECTION("Rotation matrix generation")
    {
        constexpr AmReal32 yaw = AM_DegToRad * 45.0f;
        constexpr AmReal32 pitch = AM_DegToRad * 30.0f;
        constexpr AmReal32 roll = AM_DegToRad * 15.0f;

        const Orientation orientation(yaw, pitch, roll);
        const AmMatrix3 rotationMatrix = orientation.GetRotationMatrix();

        THEN("it should produce a valid rotation matrix")
        {
            // Check if it's orthogonal (R * R^T = I)
            const AmMatrix3 identity = Identity3();
            const AmMatrix3 product = Mul(rotationMatrix, Transpose(rotationMatrix));

            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    REQUIRE(std::abs(product[i][j] - identity[i][j]) < kEpsilon);

            // Check determinant is 1 (proper rotation)
            const AmReal32 det = Determinant(rotationMatrix);
            REQUIRE(std::abs(det - 1.0f) < kEpsilon);
        }

        THEN("it should correctly transform the unit vectors")
        {
            const AmVector3 transformedY = Transform(rotationMatrix, kVector3UnitY);
            const AmVector3 transformedZ = Transform(rotationMatrix, kVector3UnitZ);

            REQUIRE(transformedY == orientation.GetForward());
            REQUIRE(transformedZ == orientation.GetUp());
        }
    }

    SECTION("Look-at matrix generation")
    {
        const Orientation orientation(0.0f, 0.0f, 0.0f); // Zero orientation
        constexpr AmVector3 eye = { 5.0f, 5.0f, 5.0f };

        const AmMatrix4 lookAtMatrix = orientation.GetLookAtMatrix(eye);

        THEN("it should produce a valid transformation matrix")
        {
            // The matrix should be invertible
            const AmReal32 det = Determinant(lookAtMatrix);
            REQUIRE(std::abs(det) > kEpsilon);
        }

        THEN("it should incorporate the eye position")
        {
            // The translation part should be related to the eye position
            REQUIRE(lookAtMatrix[3][0] != 0.0f || lookAtMatrix[3][1] != 0.0f || lookAtMatrix[3][2] != 0.0f);
        }
    }

    SECTION("Consistency between different representations")
    {
        GIVEN("an orientation created from Euler angles")
        {
            constexpr AmReal32 yaw = AM_DegToRad * 60.0f;
            constexpr AmReal32 pitch = AM_DegToRad * 20.0f;
            constexpr AmReal32 roll = AM_DegToRad * 10.0f;

            const Orientation originalOrientation(yaw, pitch, roll);

            WHEN("reconstructed from its forward and up vectors")
            {
                const Orientation reconstructedFromVectors(originalOrientation.GetForward(), originalOrientation.GetUp());

                THEN("it should produce equivalent orientations")
                {
                    REQUIRE(originalOrientation.GetForward() == reconstructedFromVectors.GetForward());
                    REQUIRE(originalOrientation.GetUp() == reconstructedFromVectors.GetUp());
                }
            }

            WHEN("reconstructed from its quaternion")
            {
                const Orientation reconstructedFromQuaternion(originalOrientation.GetQuaternion());

                THEN("it should produce equivalent orientations")
                {
                    REQUIRE(originalOrientation.GetForward() == reconstructedFromQuaternion.GetForward());
                    REQUIRE(originalOrientation.GetUp() == reconstructedFromQuaternion.GetUp());
                }
            }
        }
    }

    SECTION("Edge cases")
    {
        THEN("it should handle extreme angle values")
        {
            // Test with large angles
            const Orientation orientation1(AM_PI32 * 2.5f, AM_PI32 * 1.5f, AM_PI32 * 3.0f);

            // Vectors should still be normalized
            REQUIRE(std::abs(Length(orientation1.GetForward()) - 1.0f) < kEpsilon);
            REQUIRE(std::abs(Length(orientation1.GetUp()) - 1.0f) < kEpsilon);
        }

        THEN("it should handle nearly parallel vectors")
        {
            constexpr AmVector3 forward = { 1.0f, 0.0f, 0.0f };
            constexpr AmVector3 up = { 1.0f, 0.01f, 0.0f }; // Nearly parallel

            const Orientation orientation(forward, up);

            // Should still produce normalized, orthogonal vectors
            const auto resultForward = orientation.GetForward();
            const auto resultUp = orientation.GetUp();

            REQUIRE(std::abs(Length(resultForward) - 1.0f) < kEpsilon);
            REQUIRE(std::abs(Length(resultUp) - 1.0f) < kEpsilon);
            REQUIRE(std::abs(Dot(resultForward, resultUp)) < kEpsilon);
        }
    }
}