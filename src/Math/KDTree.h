// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_MATH_KD_TREE_H
#define _AM_IMPLEMENTATION_MATH_KD_TREE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>

#include <limits>
#include <vector>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A k-dimensional tree (kd-tree) over 3-D points for exact nearest-neighbor queries.
     *
     * The tree owns a copy of the points passed to @ref Build, stored in the original input
     * order. @ref FindNearest returns the index of the nearest point into that original
     * ordering, so callers can use it to index parallel arrays of their own.
     *
     * Ties (two points equidistant from the query) are broken deterministically: the lowest
     * index wins, matching a first-found brute-force scan.
     */
    class AM_API_PUBLIC KDTree
    {
    public:
        /**
         * @brief Sentinel returned by @ref FindNearest when the tree is empty.
         */
        static constexpr AmSize kInvalidIndex = std::numeric_limits<AmSize>::max();

        KDTree() = default;

        /**
         * @brief Builds the tree over a copy of the given points. O(n log n).
         *
         * @param[in] points The points to index. A copy is retained; the original vector
         * may be destroyed after this call.
         */
        void Build(const std::vector<AmVector3>& points);

        /**
         * @brief Gets the number of indexed points.
         *
         * @return The point count, or 0 if the tree is empty.
         */
        [[nodiscard]] AmSize GetPointCount() const;

        /**
         * @brief Finds the index of the point nearest to the query.
         *
         * @param[in] query The query point.
         *
         * @return The index of the nearest point (into the original Build input), or
         * @ref kInvalidIndex when the tree is empty. Lowest index wins on ties.
         */
        [[nodiscard]] AmSize FindNearest(const AmVector3& query) const;

    private:
        struct Node
        {
            AmUInt32 m_PointIndex; // index into _points (== original input index)
            AmUInt8 m_Axis; // 0, 1, or 2 — split axis
            AmUInt32 m_Left; // index into _nodes, or kNoChild
            AmUInt32 m_Right; // index into _nodes, or kNoChild
        };

        static constexpr AmUInt32 kNoChild = 0xFFFFFFFFu;

        static AmUInt32 BuildRecursive(
            std::vector<AmVector3>& points, std::vector<AmSize>& indices, AmSize lo, AmSize hi, std::vector<Node>& nodes);

        static void FindNearestRecursive(
            const std::vector<AmVector3>& points,
            const std::vector<Node>& nodes,
            AmUInt32 nodeIndex,
            const AmVector3& query,
            AmReal32& bestDist,
            AmSize& bestIndex);

        std::vector<AmVector3> _points; // owned copy, input order
        std::vector<Node> _nodes; // flat tree storage; _nodes[0] is the root when non-empty
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MATH_KD_TREE_H
