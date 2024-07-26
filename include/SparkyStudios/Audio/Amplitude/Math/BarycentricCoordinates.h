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

#ifndef _AM_MATH_BARYCENTRIC_COORDINATES_H
#define _AM_MATH_BARYCENTRIC_COORDINATES_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represents barycentric coordinates between a point and 3 vertices of a triangle.
     */
    struct BarycentricCoordinates
    {
        /**
         * @brief Computes the barycentric coordinates of the intersection of a ray with a triangle.
         *
         * @param rayOrigin The origin of the ray.
         * @param rayDirection The direction of the ray.
         * @param triangle The vertices of the triangle.
         * @param result The result of the intersection.
         *
         * @return @c true if the ray intersects the triangle, @c false otherwise.
         */
        static bool RayTriangleIntersection(
            const AmVec3& rayOrigin, const AmVec3& rayDirection, const std::array<AmVec3, 3>& triangle, BarycentricCoordinates& result);

        /**
         * @brief Default constructor.
         */
        BarycentricCoordinates();

        /**
         * @brief Computes barcentric coordinates from a position and a triangle.
         *
         * @param position The position of the intersection.
         * @param triangle The triangle.
         */
        BarycentricCoordinates(const AmVec3& position, const std::array<AmVec3, 3>& triangle);

        /**
         * @brief Checks whether the coordinates are valid.
         *
         * @return @c true if the coordinates are valid, @c false otherwise.
         */
        [[nodiscard]] bool IsValid() const;

        AmReal32 m_U;
        AmReal32 m_V;
        AmReal32 m_W;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_BARYCENTRIC_COORDINATES_H