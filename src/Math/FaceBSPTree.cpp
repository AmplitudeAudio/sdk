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

#include <SparkyStudios/Audio/Amplitude/Math/Geometry.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <Math/FaceBSPTree.h>

namespace SparkyStudios::Audio::Amplitude
{
    void FaceBSPTree::Build(const std::vector<AmVec3>& points, const std::vector<Face>& faces)
    {
        std::vector<Edge> edges;
        GetEdges(faces, edges);

        BuildTree(points, faces, edges, _nodes);
    }

    const Face* FaceBSPTree::Query(const AmVec3& direction) const
    {
        if (_nodes.empty())
            return nullptr;

        AmSize index = 0;
        while (true)
        {
            switch (_nodes[index].m_Type)
            {
            case Node::kNode_Leaf:
                {
                    if (_nodes[index].m_Leaf.m_Face.IsValid())
                        return &_nodes[index].m_Leaf.m_Face;

                    return nullptr;
                }
            case Node::kNode_Split:
                {
                    if (AM_Dot(_nodes[index].m_Split.m_PlaneNormal, direction) > 0.0f)
                        index = _nodes[index].m_Split.m_LeftIndex;
                    else
                        index = _nodes[index].m_Split.m_RightIndex;
                    break;
                }
            }
        }
    }

    void FaceBSPTree::GetEdges(const std::vector<Face>& faces, std::vector<Edge>& edges)
    {
        edges.clear();
        edges.reserve(faces.size() * 3);

        for (const auto& face : faces)
        {
            edges.push_back({ AM_MIN(face.m_A, face.m_B), AM_MAX(face.m_A, face.m_B) });
            edges.push_back({ AM_MIN(face.m_C, face.m_A), AM_MAX(face.m_C, face.m_A) });
            edges.push_back({ AM_MIN(face.m_B, face.m_C), AM_MAX(face.m_B, face.m_C) });
        }

        std::sort(edges.begin(), edges.end());
        edges.erase(std::ranges::unique(edges).begin(), edges.end());

        // We always sort edges and then choose the first one for splitting, but randomly choosing
        // the splitting plane is more optimal. Here is the simplest LCG random generator. The
        // parameters were copied from Numerical Recipes.
        const AmSize first_idx = (edges.size() * 1664525 + 1013904223) % edges.size();
        std::swap(edges[0], edges[first_idx]);
    }

    void FaceBSPTree::BuildTree(
        const std::vector<AmVec3>& vertices, const std::vector<Face>& faces, std::vector<Edge>& edges, std::vector<Node>& nodes)
    {
        constexpr AmReal32 kE = std::numeric_limits<float>::epsilon() * 4.0f;

        AmSize currentEdge = 0;
        while (currentEdge < edges.size())
        {
            const auto& splitBy = edges[currentEdge];
            currentEdge++;

            // The plane passes through by splitBy and (0, 0, 0).
            auto normal = AM_Cross(vertices[splitBy.m_E0], vertices[splitBy.m_E1]);

            // Split faces into subspaces.
            std::vector<Face> facesL;
            std::vector<Face> facesR;
            for (const auto& face : faces)
            {
                if (AM_Dot(normal, vertices[face.m_A]) > kE || AM_Dot(normal, vertices[face.m_B]) > kE ||
                    AM_Dot(normal, vertices[face.m_C]) > kE)
                {
                    facesL.push_back(face);
                }

                if (AM_Dot(normal, vertices[face.m_A]) < -kE || AM_Dot(normal, vertices[face.m_B]) < -kE ||
                    AM_Dot(normal, vertices[face.m_C]) < -kE)
                {
                    facesR.push_back(face);
                }
            }

            if ((facesL.empty() || facesL.size() == faces.size()) || (facesR.empty() || facesR.size() == faces.size()))
            {
                // No reason to add a split, continue to the next edge.
                AMPLITUDE_ASSERT(!edges.empty());
                continue;
            }

            // We need to process only edges from left faces in left subspace.
            std::vector<Edge> edgesL;
            GetEdges(facesL, edgesL);

            std::vector<Edge> edgesR;
            GetEdges(facesR, edgesR);

            // Left node is always the next one, leave the right one for now.
            auto currentIndex = nodes.size();
            auto leftIndex = (nodes.size() + 1);

            Node leftNode{};
            leftNode.m_Type = Node::kNode_Split;
            leftNode.m_Split.m_PlaneNormal = AM_Norm(normal);
            leftNode.m_Split.m_LeftIndex = leftIndex;
            leftNode.m_Split.m_RightIndex = 0;
            nodes.push_back(leftNode);

            // Process left subspace.
            BuildChild(vertices, facesL, edgesL, nodes);

            // Process right subspace and fill in the right node index.
            auto nextIndex = nodes.size();
            if (auto& node = nodes[currentIndex]; node.m_Type == Node::kNode_Split)
                node.m_Split.m_RightIndex = nextIndex;

            BuildChild(vertices, facesR, edgesR, nodes);
            break;
        }
    }

    void FaceBSPTree::BuildChild(
        const std::vector<AmVec3>& vertices, const std::vector<Face>& faces, std::vector<Edge>& edges, std::vector<Node>& nodes)
    {
        // We should have at most one remaining face if there are no remaining edges. This is not
        // true either due to a bug, or when the source data is incorrect (either the sphere is not
        // convex or the (0, 0, 0) is not inside the sphere).
        if (faces.empty())
        {
            Node leaf{};
            leaf.m_Type = Node::kNode_Leaf;
            leaf.m_Leaf.m_Face = { 0, 0, 0 };

            nodes.push_back(leaf);
        }
        else if (faces.size() == 1)
        {
            Node leaf{};
            leaf.m_Type = Node::kNode_Leaf;
            leaf.m_Leaf.m_Face = faces[0];

            nodes.push_back(leaf);
        }
        else
        {
            AMPLITUDE_ASSERT(!edges.empty());
            BuildTree(vertices, faces, edges, nodes);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude