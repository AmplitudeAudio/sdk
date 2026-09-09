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

#include <Math/KDTree.h>

#include <Math/LinearAlgebra.h>

#include <algorithm>

namespace SparkyStudios::Audio::Amplitude
{
    void KDTree::Build(const std::vector<AmVector3>& points)
    {
        _points = points;
        _nodes.clear();

        if (_points.empty())
            return;

        std::vector<AmSize> indices(_points.size());
        for (AmSize i = 0; i < indices.size(); ++i)
            indices[i] = i;

        _nodes.reserve(_points.size());
        BuildRecursive(_points, indices, 0, indices.size(), _nodes);
    }

    AmSize KDTree::GetPointCount() const
    {
        return _points.size();
    }

    AmSize KDTree::FindNearest(const AmVector3& query) const
    {
        if (_nodes.empty())
            return kInvalidIndex;

        AmReal32 bestDist = std::numeric_limits<AmReal32>::max();
        AmSize bestIndex = kInvalidIndex;

        FindNearestRecursive(_points, _nodes, 0, query, bestDist, bestIndex);

        return bestIndex;
    }

    AmUInt32 KDTree::BuildRecursive(
        std::vector<AmVector3>& points, std::vector<AmSize>& indices, AmSize lo, AmSize hi, std::vector<Node>& nodes)
    {
        // Partition by index RANGE, not by coordinate value, so duplicate coordinates
        // cannot stall recursion. [lo, hi) is never empty here.
        AmUInt32 nodeIndex = static_cast<AmUInt32>(nodes.size());
        nodes.push_back(Node{});

        AmSize mid = lo + (hi - lo) / 2;

        if (hi - lo == 1)
        {
            nodes[nodeIndex].m_PointIndex = static_cast<AmUInt32>(indices[lo]);
            nodes[nodeIndex].m_Axis = 0;
            nodes[nodeIndex].m_Left = kNoChild;
            nodes[nodeIndex].m_Right = kNoChild;
            return nodeIndex;
        }

        // Split axis: the coordinate with the largest (max - min) spread over [lo, hi).
        AmUInt8 axis = 0;
        {
            AmReal32 minV[3] = { points[indices[lo]].x, points[indices[lo]].y, points[indices[lo]].z };
            AmReal32 maxV[3] = { minV[0], minV[1], minV[2] };

            for (AmSize i = lo + 1; i < hi; ++i)
            {
                const auto& p = points[indices[i]];
                minV[0] = AM_MIN(minV[0], p.x);
                minV[1] = AM_MIN(minV[1], p.y);
                minV[2] = AM_MIN(minV[2], p.z);
                maxV[0] = AM_MAX(maxV[0], p.x);
                maxV[1] = AM_MAX(maxV[1], p.y);
                maxV[2] = AM_MAX(maxV[2], p.z);
            }

            AmReal32 spread[3] = { maxV[0] - minV[0], maxV[1] - minV[1], maxV[2] - minV[2] };
            axis = static_cast<AmUInt8>(std::max_element(spread, spread + 3) - spread);
        }

        // Median by axis coordinate; ties broken by index so ordering is deterministic.
        std::nth_element(
            indices.begin() + static_cast<std::ptrdiff_t>(lo), indices.begin() + static_cast<std::ptrdiff_t>(mid),
            indices.begin() + static_cast<std::ptrdiff_t>(hi),
            [&points, axis](AmSize a, AmSize b)
            {
                const AmReal32 va = points[a].data[axis];
                const AmReal32 vb = points[b].data[axis];
                if (va != vb)
                    return va < vb;
                return a < b;
            });

        nodes[nodeIndex].m_PointIndex = static_cast<AmUInt32>(indices[mid]);
        nodes[nodeIndex].m_Axis = axis;

        if (lo < mid)
            nodes[nodeIndex].m_Left = BuildRecursive(points, indices, lo, mid, nodes);
        else
            nodes[nodeIndex].m_Left = kNoChild;

        if (mid + 1 < hi)
            nodes[nodeIndex].m_Right = BuildRecursive(points, indices, mid + 1, hi, nodes);
        else
            nodes[nodeIndex].m_Right = kNoChild;

        return nodeIndex;
    }

    void KDTree::FindNearestRecursive(
        const std::vector<AmVector3>& points,
        const std::vector<Node>& nodes,
        AmUInt32 nodeIndex,
        const AmVector3& query,
        AmReal32& bestDist,
        AmSize& bestIndex)
    {
        const Node& node = nodes[nodeIndex];

        const AmVector3& pivot = points[node.m_PointIndex];
        const AmReal32 dist = SquaredLength(Sub(query, pivot));

        // Update: strictly closer, or equal distance with a lower index.
        if (dist < bestDist || (dist == bestDist && node.m_PointIndex < bestIndex))
        {
            bestDist = dist;
            bestIndex = node.m_PointIndex;
        }

        const AmReal32 axisDelta = query.data[node.m_Axis] - pivot.data[node.m_Axis];
        const AmReal32 planeDist = axisDelta * axisDelta;

        const AmUInt32 nearChild = (query.data[node.m_Axis] < pivot.data[node.m_Axis]) ? node.m_Left : node.m_Right;
        const AmUInt32 farChild = (nearChild == node.m_Left) ? node.m_Right : node.m_Left;

        if (nearChild != kNoChild)
            FindNearestRecursive(points, nodes, nearChild, query, bestDist, bestIndex);

        // Visit the far side when it can still improve the answer. <= (not <) so a point
        // exactly on the split plane with a lower index is not skipped.
        if (farChild != kNoChild && planeDist <= bestDist)
            FindNearestRecursive(points, nodes, farChild, query, bestDist, bestIndex);
    }
} // namespace SparkyStudios::Audio::Amplitude
