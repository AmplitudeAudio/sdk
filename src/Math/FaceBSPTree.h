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

#ifndef _AM_IMPLEMENTATION_MATH_FACE_BSP_H
#define _AM_IMPLEMENTATION_MATH_FACE_BSP_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A Binary Space Partitioning Tree implementation used to partition a convex
     * hull into a set of faces.
     */
    class AM_API_PUBLIC FaceBSPTree
    {
    public:
        struct Node
        {
            enum : AmUInt8
            {
                kNode_Leaf = 0,
                kNode_Split = 1,
            } m_Type;

            union {
                struct
                {
                    AmVec3 m_PlaneNormal;
                    AmUInt32 m_LeftIndex;
                    AmUInt32 m_RightIndex;
                } m_Split;

                struct
                {
                    Face m_Face;
                } m_Leaf;
            };
        };

        FaceBSPTree() = default;

        void Build(const std::vector<AmVec3>& points, const std::vector<Face>& faces);

        [[nodiscard]] const Face* Query(const AmVec3& direction) const;

    private:
        static void GetEdges(const std::vector<Face>& faces, std::vector<Edge>& edges);
        static void BuildTree(
            const std::vector<AmVec3>& vertices, const std::vector<Face>& faces, std::vector<Edge>& edges, std::vector<Node>& nodes);
        static void BuildChild(
            const std::vector<AmVec3>& vertices, const std::vector<Face>& faces, std::vector<Edge>& edges, std::vector<Node>& nodes);

        std::vector<Node> _nodes;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MATH_FACE_BSP_H