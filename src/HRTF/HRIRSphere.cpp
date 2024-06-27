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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/HRTF/HRIRSphere.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    HRIRSphere::HRIRSphere(const AmOsString& filePath)
        : Resource()
    {
        _filename = filePath;
    }

    HRIRSphere::~HRIRSphere()
    {
        _vertices.clear();
        _indices.clear();
    }

    void HRIRSphere::Load(const FileSystem* loader)
    {
        if (!loader->Exists(_filename))
        {
            amLogError("Failed to load HRIRSphere: " AM_OS_CHAR_FMT " does not exist", _filename.c_str());
            return;
        }

        const auto file = loader->OpenFile(_filename);

        // Read the header magic
        file->Read(_header.m_Header, 4);

        if (std::strncmp(reinterpret_cast<const char*>(_header.m_Header), "AMIR", 4) != 0)
        {
            amLogError("Failed to load HRIRSphere: " AM_OS_CHAR_FMT " is not a valid HRIRSphere file", _filename.c_str());
            return;
        }

        _header.m_Version = file->Read16();
        _header.m_SampleRate = file->Read32();
        _header.m_IRLength = file->Read32();
        _header.m_VertexCount = file->Read32();
        _header.m_IndexCount = file->Read32();

        _indices.resize(_header.m_IndexCount);
        file->Read(reinterpret_cast<AmUInt8Buffer>(_indices.data()), _header.m_IndexCount * sizeof(AmUInt32));

        _vertices.resize(_header.m_VertexCount);
        for (auto& vertex : _vertices)
        {
            file->Read(reinterpret_cast<AmUInt8Buffer>(&vertex.m_Position), sizeof(AmVec3));

            vertex.m_LeftIR.resize(_header.m_IRLength);
            file->Read(reinterpret_cast<AmUInt8Buffer>(vertex.m_LeftIR.data()), _header.m_IRLength * sizeof(AmReal32));

            vertex.m_RightIR.resize(_header.m_IRLength);
            file->Read(reinterpret_cast<AmUInt8Buffer>(vertex.m_RightIR.data()), _header.m_IRLength * sizeof(AmReal32));
        }
    }

    const std::vector<HRIRSphereVertex>& HRIRSphere::GetVertices() const
    {
        return _vertices;
    }

    const std::vector<AmUInt32>& HRIRSphere::GetIndices() const
    {
        return _indices;
    }

    const HRIRSphereVertex& HRIRSphere::GetVertex(AmUInt32 index) const
    {
        return _vertices[index];
    }

    AmUInt32 HRIRSphere::GetVertexCount() const
    {
        return _vertices.size();
    }

    AmUInt32 HRIRSphere::GetIndexCount() const
    {
        return _indices.size();
    }

    AmUInt32 HRIRSphere::GetSampleRate() const
    {
        return _header.m_SampleRate;
    }

    AmUInt32 HRIRSphere::GetIRLength() const
    {
        return _header.m_IRLength;
    }
} // namespace SparkyStudios::Audio::Amplitude
