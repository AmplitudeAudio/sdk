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

#ifndef _AM_IMPLEMENTATION_HRTF_HRIR_SPHERE_H
#define _AM_IMPLEMENTATION_HRTF_HRIR_SPHERE_H

#include <SparkyStudios/Audio/Amplitude/HRTF/HRIRSphere.h>

#include <IO/Resource.h>
#include <Math/FaceBSPTree.h>

namespace SparkyStudios::Audio::Amplitude
{
    class HRIRSphereImpl final
        : public HRIRSphere
        , public ResourceImpl
    {
    public:
        /**
         * @brief Constructs a new @c HRIRSphere instance.
         */
        HRIRSphereImpl();

        ~HRIRSphereImpl() override;

        void SetResource(const AmOsString& filePath) override;

        [[nodiscard]] AM_INLINE const AmOsString& GetPath() const override
        {
            return ResourceImpl::GetPath();
        }

        void Load(const FileSystem* loader) override;

        [[nodiscard]] const std::vector<HRIRSphereVertex>& GetVertices() const override;
        [[nodiscard]] const std::vector<Face>& GetFaces() const override;
        [[nodiscard]] const HRIRSphereVertex& GetVertex(AmUInt32 index) const override;
        [[nodiscard]] AmUInt32 GetVertexCount() const override;
        [[nodiscard]] AmUInt32 GetFaceCount() const override;
        [[nodiscard]] AmUInt32 GetSampleRate() const override;
        [[nodiscard]] AmUInt32 GetIRLength() const override;
        void SetSamplingMode(eHRIRSphereSamplingMode mode) override;
        [[nodiscard]] eHRIRSphereSamplingMode GetSamplingMode() const override;
        void Sample(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const override;
        void Transform(const AmMat4& matrix) override;
        [[nodiscard]] bool IsLoaded() const override;

    private:
        void SampleBilinear(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const;
        void SampleNearestNeighbor(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const;
        const HRIRSphereVertex* GetClosestVertex(const AmVec3& position, const Face* face) const;

        eHRIRSphereSamplingMode _samplingMode;
        HRIRSphereFileHeaderDescription _header;
        std::vector<HRIRSphereVertex> _vertices;
        std::vector<Face> _faces;
        FaceBSPTree _tree;

        bool _loaded;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_HRTF_HRIR_SPHERE_H
