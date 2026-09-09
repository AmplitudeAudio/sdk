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

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>
#include <SparkyStudios/Audio/Amplitude/Math/BarycentricCoordinates.h>

#include <HRTF/HRIRSphere.h>
#include <Math/FaceBSPTree.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    HRIRSphereImpl::HRIRSphereImpl()
        : ResourceImpl()
        , _samplingMode(eHRIRSphereSamplingMode_NearestNeighbor)
        , _loaded(false)
    {}

    HRIRSphereImpl::~HRIRSphereImpl()
    {
        _vertices.clear();
        _faces.clear();
    }

    void HRIRSphereImpl::SetResource(const AmOsString& filePath)
    {
        m_filename = filePath;
    }

    void HRIRSphereImpl::Load(std::shared_ptr<const FileSystem> loader)
    {
        if (_loaded)
            return;

        if (!loader->Exists(m_filename))
        {
            amLogError("Failed to load HRIRSphere: " AM_OS_CHAR_FMT " does not exist.", m_filename.c_str());
            return;
        }

        const auto file = loader->OpenFile(m_filename);

        // Read the header magic
        file->Read(_header.m_Header, 4);

        if (std::strncmp(reinterpret_cast<char*>(_header.m_Header), "AMIR", 4) != 0)
        {
            amLogError("Failed to load HRIRSphere: " AM_OS_CHAR_FMT " is not a valid HRIRSphere file.", m_filename.c_str());
            return;
        }

        _header.m_Version = file->Read16();
        _header.m_SampleRate = file->Read32();
        _header.m_IRLength = file->Read32();
        _header.m_VertexCount = file->Read32();
        _header.m_IndexCount = file->Read32();

        std::vector<AmUInt32> indices(_header.m_IndexCount);
        file->Read(reinterpret_cast<AmUInt8Buffer>(indices.data()), _header.m_IndexCount * sizeof(AmUInt32));

        _vertices.resize(_header.m_VertexCount);

        for (auto& vertex : _vertices)
        {
            file->Read(reinterpret_cast<AmUInt8Buffer>(&vertex.m_Position), sizeof(AmVector3));

            vertex.m_LeftIR.resize(_header.m_IRLength);
            file->Read(reinterpret_cast<AmUInt8Buffer>(vertex.m_LeftIR.data()), _header.m_IRLength * sizeof(AmReal32));

            vertex.m_RightIR.resize(_header.m_IRLength);
            file->Read(reinterpret_cast<AmUInt8Buffer>(vertex.m_RightIR.data()), _header.m_IRLength * sizeof(AmReal32));

            file->Read(reinterpret_cast<AmUInt8Buffer>(&vertex.m_LeftDelay), sizeof(AmReal32));

            file->Read(reinterpret_cast<AmUInt8Buffer>(&vertex.m_RightDelay), sizeof(AmReal32));
        }

        const AmUInt32 faceCount = indices.size() / 3;
        _faces.resize(faceCount);

        for (AmUInt32 i = 0; i < faceCount; ++i)
        {
            _faces[i].m_A = indices[i * 3 + 0];
            _faces[i].m_B = indices[i * 3 + 1];
            _faces[i].m_C = indices[i * 3 + 2];
        }

        RebuildIndexes();

        _loaded = true;
    }

    const std::vector<HRIRSphereVertex>& HRIRSphereImpl::GetVertices() const
    {
        return _vertices;
    }

    const std::vector<Face>& HRIRSphereImpl::GetFaces() const
    {
        return _faces;
    }

    const HRIRSphereVertex& HRIRSphereImpl::GetVertex(AmUInt32 index) const
    {
        return _vertices[index];
    }

    AmUInt32 HRIRSphereImpl::GetVertexCount() const
    {
        return _vertices.size();
    }

    AmUInt32 HRIRSphereImpl::GetFaceCount() const
    {
        return _faces.size();
    }

    AmUInt32 HRIRSphereImpl::GetSampleRate() const
    {
        return _header.m_SampleRate;
    }

    AmUInt32 HRIRSphereImpl::GetIRLength() const
    {
        return _header.m_IRLength;
    }

    eHRIRSphereSamplingMode HRIRSphereImpl::GetSamplingMode() const
    {
        return _samplingMode;
    }

    void HRIRSphereImpl::SetSamplingMode(eHRIRSphereSamplingMode mode)
    {
        _samplingMode = mode;
    }

    void HRIRSphereImpl::Sample(const AmVector3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const
    {
        switch (_samplingMode)
        {
        case eHRIRSphereSamplingMode_Bilinear:
            return SampleBilinear(direction, leftHRIR, rightHRIR);
        case eHRIRSphereSamplingMode_NearestNeighbor:
            return SampleNearestNeighbor(direction, leftHRIR, rightHRIR);
        }
    }

    void HRIRSphereImpl::Transform(const AmMatrix4& matrix)
    {
        for (auto& vertex : _vertices)
            vertex.m_Position = Amplitude::Transform(matrix, AmVector4{ .xyz = vertex.m_Position, ._pad2 = 1.0f }).xyz;

        // Vertex positions changed: both geometry-derived indexes must follow.
        RebuildIndexes();
    }

    void HRIRSphereImpl::RebuildIndexes()
    {
        // The face BSP is built over raw positions; the nearest-neighbor KD-tree over
        // normalized positions (over unit vectors, max-Dot == min-Euclidean).
        std::vector<AmVector3> positions;
        positions.reserve(_vertices.size());

        std::vector<AmVector3> normals;
        normals.reserve(_vertices.size());

        for (const auto& vertex : _vertices)
        {
            positions.push_back(vertex.m_Position);
            normals.push_back(Normalize(vertex.m_Position));
        }

        _tree.Build(positions, _faces);
        _nearestIndex.Build(normals);
    }

    bool HRIRSphereImpl::IsLoaded() const
    {
        return _loaded;
    }

    void HRIRSphereImpl::SampleBilinear(const AmVector3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const
    {
        const auto& dir = Scale(direction, 10.0f);
        const auto* face = _tree.Query(dir);

        if (face == nullptr)
        {
            std::memset(leftHRIR, 0, GetIRLength() * sizeof(AmReal32));
            std::memset(rightHRIR, 0, GetIRLength() * sizeof(AmReal32));
            return;
        }

        // If we are very close to any vertex, just return the HRIR of that vertex
        {
            const auto* vertex = GetClosestVertex(direction, face);

            if (vertex != nullptr)
            {
                const AmSize length = vertex->m_LeftIR.size();
                std::memcpy(leftHRIR, vertex->m_LeftIR.data(), length * sizeof(AmReal32));
                std::memcpy(rightHRIR, vertex->m_RightIR.data(), length * sizeof(AmReal32));
                return;
            }
        }

        const auto& vertexA = _vertices[face->m_A];
        const auto& vertexB = _vertices[face->m_B];
        const auto& vertexC = _vertices[face->m_C];

        // Otherwise, perform bilinear interpolation
        {
            BarycentricCoordinates barycenter;
            if (!BarycentricCoordinates::RayTriangleIntersection(
                    kVector3Zero, dir, { vertexA.m_Position, vertexB.m_Position, vertexC.m_Position }, barycenter))
            {
                std::memset(leftHRIR, 0, GetIRLength() * sizeof(AmReal32));
                std::memset(rightHRIR, 0, GetIRLength() * sizeof(AmReal32));
                return;
            }

            const AmSize length = vertexA.m_LeftIR.size();

            for (AmSize i = 0; i < length; ++i)
            {
                leftHRIR[i] =
                    vertexA.m_LeftIR[i] * barycenter.m_U + vertexB.m_LeftIR[i] * barycenter.m_V + vertexC.m_LeftIR[i] * barycenter.m_W;

                rightHRIR[i] =
                    vertexA.m_RightIR[i] * barycenter.m_U + vertexB.m_RightIR[i] * barycenter.m_V + vertexC.m_RightIR[i] * barycenter.m_W;
            }
        }
    }

    void HRIRSphereImpl::SampleNearestNeighbor(const AmVector3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const
    {
        const AmVector3 dir = Normalize(direction);
        const AmSize length = GetIRLength();

        const AmSize index = _nearestIndex.FindNearest(dir);

        if (index == KDTree::kInvalidIndex)
        {
            std::memset(leftHRIR, 0, length * sizeof(AmReal32));
            std::memset(rightHRIR, 0, length * sizeof(AmReal32));
            return;
        }

        const auto& vertex = _vertices[index];
        std::memcpy(leftHRIR, vertex.m_LeftIR.data(), length * sizeof(AmReal32));
        std::memcpy(rightHRIR, vertex.m_RightIR.data(), length * sizeof(AmReal32));
    }

    const HRIRSphereVertex* HRIRSphereImpl::GetClosestVertex(const AmVector3& position, const Face* face) const
    {
        const auto& vertexA = _vertices[face->m_A];
        const auto& vertexB = _vertices[face->m_B];
        const auto& vertexC = _vertices[face->m_C];

        constexpr AmReal32 k2 = kEpsilon * kEpsilon;

        if (SquaredLength(Sub(vertexA.m_Position, position)) < k2)
            return &vertexA;

        if (SquaredLength(Sub(vertexB.m_Position, position)) < k2)
            return &vertexB;

        if (SquaredLength(Sub(vertexC.m_Position, position)) < k2)
            return &vertexC;

        return nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
