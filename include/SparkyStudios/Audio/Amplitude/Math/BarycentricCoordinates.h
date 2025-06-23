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

#include <SparkyStudios/Audio/Amplitude/Math/Geometry.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represents barycentric coordinates between a point and 3 vertices of a triangle.
     *
     * @ingroup math
     */
    struct AM_API_PUBLIC BarycentricCoordinates
    {
        /**
         * @brief Computes the barycentric coordinates for the intersection of a ray with a triangle.
         *
         * @param[in] rayOrigin The origin of the ray.
         * @param[in] rayDirection The direction of the ray.
         * @param[in] triangle The vertices of the triangle.
         * @param[out] result The result of the intersection.
         *
         * @return @c true if the ray intersects the triangle, @c false otherwise.
         */
        static bool RayTriangleIntersection(
            const AmVector3& rayOrigin, const AmVector3& rayDirection, const Triangle& triangle, BarycentricCoordinates& result);

        /**
         * @brief Default constructor.
         */
        BarycentricCoordinates();

        /**
         * @brief Computes barycentric coordinates from a position and a triangle.
         *
         * @param[in] position The position of the intersection.
         * @param[in] triangle The triangle.
         */
        BarycentricCoordinates(const AmVector3& position, const Triangle& triangle);

        /**
         * @brief Checks whether the coordinates are valid.
         *
         * @return @c true if the coordinates are valid, @c false otherwise.
         */
        [[nodiscard]] bool IsValid() const;

        /**
         * @brief Represents the barycentric U-coordinate in a triangle.
         *
         * The U-coordinate, along with V and W, defines the barycentric
         * coordinates of a point relative to the vertices of a triangle.
         * It is computed such that the sum of U, V, and W is 1.0, ensuring
         * the point lies within or on the triangle if the values are valid.
         */
        AmReal32 m_U;

        /**
         * @brief Represents the barycentric V-coordinate in a triangle.
         *
         * The V-coordinate, together with U and W, defines the barycentric
         * coordinates of a point relative to the vertices of a triangle.
         * It is computed such that the sum of U, V, and W is 1.0, ensuring
         * the point lies within or on the triangle if the values are valid.
         */
        AmReal32 m_V;

        /**
         * @brief Represents the barycentric W-coordinate in a triangle.
         *
         * The W-coordinate, together with U and V, defines the barycentric
         * coordinates of a point relative to the vertices of a triangle.
         * It is computed such that the sum of U, V, and W is 1.0, ensuring
         * the point lies within or on the triangle if the values are valid.
         */
        AmReal32 m_W;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_BARYCENTRIC_COORDINATES_H