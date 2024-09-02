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

#pragma once

#ifndef _AM_MATH_SPHERICAL_POSITION_H
#define _AM_MATH_SPHERICAL_POSITION_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Describes the coordinates of a point on a sphere's surface, relative
     * to the center of the sphere.
     */
    struct AM_API_PUBLIC SphericalPosition
    {
        /**
         * @brief Create a spherical position from a 3D position in world space.
         *
         * @param position The position in world space.
         */
        static SphericalPosition FromWorldSpace(const AmVec3& position);

        /**
         * @brief Create a spherical position from a 3D position in world space. This
         * method is optimized for use in HRTF (Head-Related Transfer Function) space,
         * where the azimuth is rotated 90 degrees around the z-axis
         *
         * @param position The position in world space.
         */
        static SphericalPosition ForHRTF(const AmVec3& position);

        /**
         * @brief Create a spherical position from given azimuth and elevation in degrees.
         *
         * @param azimuthDegrees The azimuth in degrees.
         * @param elevationDegrees The elevation in degrees.
         * @param radius The distance from the center of the sphere to the point.
         *
         * @return A spherical position representing the given azimuth and elevation in degrees.
         */
        static SphericalPosition FromDegrees(AmReal32 azimuthDegrees, AmReal32 elevationDegrees, AmReal32 radius = 1.0f);

        /**
         * @brief Create a spherical position with default values (azimuth = 0, elevation = 0, radius = 1).
         */
        SphericalPosition() = default;

        /**
         * @brief Create a spherical position with given azimuth, elevation, and radius.
         *
         * @param azimuth The rotation around the z-axis in radians.
         * @param elevation The rotation around the x-axis in radians.
         * @param radius The distance from the center of the sphere to the point.
         */
        SphericalPosition(AmReal32 azimuth, AmReal32 elevation, AmReal32 radius = 1.0f);

        /**
         * @brief Flip the azimuth of the spherical position and returns a new instance.
         *
         * @return A new spherical position with the azimuth flipped.
         */
        [[nodiscard]] SphericalPosition FlipAzimuth() const;

        /**
         * @brief Rotates the spherical position with the given rotation and returns a new instance.
         *
         * @param rotation The rotation to apply to the spherical position.
         *
         * @return A rotated spherical position.
         */
        [[nodiscard]] SphericalPosition Rotate(AmQuat rotation) const;

        /**
         * @brief Converts the spherical position to a 3D position in world space.
         *
         * @return A 3D position in world space corresponding to the spherical position.
         */
        [[nodiscard]] AmVec3 ToCartesian() const;

        /**
         * @brief Gets the azimuth in radians of the spherical position.
         *
         * @return The azimuth in radians of the spherical position.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetAzimuth() const
        {
            return _azimuth;
        }

        /**
         * @brief Gets the elevation in radians of the spherical position.
         *
         * @return The elevation in radians of the spherical position.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetElevation() const
        {
            return _elevation;
        }

        /**
         * @brief Gets the distance from the center of the sphere to the point.
         *
         * @return The distance from the center of the sphere to the point.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetRadius() const
        {
            return _radius;
        }

        /**
         * @brief Sets the azimuth in radians of the spherical position.
         *
         * @param azimuth The new azimuth in radians.
         */
        AM_INLINE void SetAzimuth(AmReal32 azimuth)
        {
            _azimuth = azimuth;
        }

        /**
         * @brief Sets the elevation in radians of the spherical position.
         *
         * @param elevation The new elevation in radians.
         */
        AM_INLINE void SetElevation(AmReal32 elevation)
        {
            _elevation = elevation;
        }

        /**
         * @brief Sets the distance from the center of the sphere to the point.
         *
         * @param radius The new distance from the center of the sphere to the point.
         */
        AM_INLINE void SetRadius(AmReal32 radius)
        {
            _radius = radius;
        }

        /**
         * @brief Compares two spherical positions for equality.
         *
         * @param other The other spherical position to compare with.
         *
         * @return @c true if the spherical positions are equal, @c false otherwise.
         */
        bool operator==(const SphericalPosition& other) const;

        /**
         * @brief Compares two spherical positions for inequality.
         *
         * @param other The other spherical position to compare with.
         *
         * @return @c true if the spherical positions are not equal, @c false otherwise.
         */
        bool operator!=(const SphericalPosition& other) const;

    private:
        AmReal32 _azimuth = 0.0f;
        AmReal32 _elevation = 0.0f;
        AmReal32 _radius = 1.0f;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_SPHERICAL_POSITION_H