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

#ifndef _AM_MATH_GEOMETRY_H
#define _AM_MATH_GEOMETRY_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represent a triangle geometry.
     *
     * A triangle is a collection of three vertices oriented in a Clock-Wise (CW) manner.
     *
     * @ingroup math
     */
    typedef union Triangle {
        AmVector3 m_Vertices[3];

        struct
        {
            /**
             * @brief Represents the first vertex of a triangle.
             */
            AmVector3 m_A;

            /**
             * @brief Represents the second vertex of a triangle.
             */
            AmVector3 m_B;

            /**
             * @brief Represents the third vertex of a triangle.
             */
            AmVector3 m_C;
        };

#ifdef __cplusplus
        AM_INLINE AmVector3& operator[](const AmUInt32 index)
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 2));
            return m_Vertices[index];
        }

        [[nodiscard]] AM_INLINE const AmVector3& operator[](const AmUInt32 index) const
        {
            AMPLITUDE_ASSERT(AM_BETWEEN(index, 0, 2));
            return m_Vertices[index];
        }
#endif
    } Triangle;

    /**
     * @brief Represents a triangulated face.
     *
     * A face is defined by three vertices. This structure is optimized for use
     * with an existing indexed vertex array, so only the indices of each face's
     * vertex need to be provided.
     *
     * @ingroup math
     */
    struct AM_API_PUBLIC Face
    {
        /**
         * @brief Index of the first vertex.
         */
        AmSize m_A;

        /**
         * @brief Index of the second vertex.
         */
        AmSize m_B;

        /**
         * @brief Index of the third vertex.
         */
        AmSize m_C;

        /**
         * @brief Checks if the face is valid.
         *
         * @return @c true if the face is valid, @c false otherwise.
         */
        [[nodiscard]] bool IsValid() const;
    };

    /**
     * @brief Represents an edge.
     *
     * An edge is defined by two vertices. This structure is optimized for use
     * with an existing indexed vertex array, so only the indices of each edge's
     * vertex need to be provided.
     *
     * @ingroup math
     */
    struct AM_API_PUBLIC Edge
    {
        /**
         * @brief Index of the first vertex.
         */
        AmSize m_E0;

        /**
         * @brief Index of the second vertex.
         */
        AmSize m_E1;

        /**
         * @brief Compares two edges for ordering.
         *
         * This operator determines the ordering of two edges based on their vertex indices.
         * It first compares the first vertex index (m_E0) of the two edges, and if they are equal,
         * it compares the second vertex index (m_E1).
         *
         * @param other The edge to compare against.
         * @return @c true if this edge is considered less than the other, @c false otherwise.
         */
        bool operator<(const Edge& other) const;

        /**
         * @brief Checks if two edges are equal.
         *
         * Compares the vertex indices of two edges to determine if they are equivalent.
         * Two edges are considered equal if both of their vertex indices are equal.
         *
         * @param other The edge to compare against.
         * @return @c true if the edges are equal, @c false otherwise.
         */
        bool operator==(const Edge& other) const;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_GEOMETRY_H
