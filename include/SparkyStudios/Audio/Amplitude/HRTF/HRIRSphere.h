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

#ifndef _AM_HRTF_HRIR_SPHERE_H
#define _AM_HRTF_HRIR_SPHERE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/IO/File.h>
#include <SparkyStudios/Audio/Amplitude/IO/Resource.h>
#include <SparkyStudios/Audio/Amplitude/Math/Geometry.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The model of the HRIR sphere dataset.
     */
    enum HRIRSphereDatasetModel : AmUInt8
    {
        /**
         * @brief The HRIR sphere uses data from the IRCAM (LISTEN) dataset.
         * (http://recherche.ircam.fr/equipes/salles/listen/download.html)
         */
        eHRIRSphereDatasetModel_IRCAM = 0,

        /**
         * @brief The HRIR sphere uses data from the MIT (KEMAR) dataset.
         * (http://sound.media.mit.edu/resources/KEMAR.html).
         */
        eHRIRSphereDatasetModel_MIT = 1,

        /**
         * @brief The HRIR sphere uses data from the SADIE II dataset.
         * (https://www.york.ac.uk/sadie-project/database.html).
         */
        eHRIRSphereDatasetModel_SADIE = 2,

        /**
         * @brief The HRIR sphere uses data from a SOFA (Spatially Oriented Format for Acoustics) file.
         * (https://www.sofaconventions.org/).
         */
        eHRIRSphereDatasetModel_SOFA = 3,

        /**
         * @brief Invalid HRIR sphere dataset model.
         */
        eHRIRSphereDatasetModel_Invalid
    };

    /**
     * @brief Provides metadata about an HRIR sphere file.
     */
    struct HRIRSphereFileHeaderDescription
    {
        /**
         * @brief HRIR sphere file header tag. Should be always equal to "AMIR".
         */
        AmUInt8 m_Header[4] = { 'A', 'M', 'I', 'R' };

        /**
         * @brief HRIR sphere file version. Allows backward compatibility.
         */
        AmUInt16 m_Version = 0;

        /**
         * @brief Sample rate used to encode HRIR data.
         */
        AmUInt32 m_SampleRate = 0;

        /**
         * @brief The length of the HRIR data in number of samples.
         */
        AmUInt32 m_IRLength = 0;

        /**
         * @brief The number of vertices in the HRIR sphere.
         */
        AmUInt32 m_VertexCount = 0;

        /**
         * @brief The number of indices in the HRIR sphere.
         */
        AmUInt32 m_IndexCount = 0;
    };

    /**
     * @brief A vertex of the HRIR sphere.
     */
    struct HRIRSphereVertex
    {
        /**
         * @brief The cartesian position of the vertex.
         */
        AmVec3 m_Position;

        /**
         * @brief The left HRIR data.
         */
        std::vector<AmReal32> m_LeftIR;

        /**
         * @brief The right HRIR data.
         */
        std::vector<AmReal32> m_RightIR;

        /**
         * @brief The delay for the left ear.
         */
        AmReal32 m_LeftDelay = 0.0f;

        /**
         * @brief The delay for the right ear.
         */
        AmReal32 m_RightDelay = 0.0f;
    };

    /**
     * @brief A 3D sphere of HRIR data.
     */
    class AM_API_PUBLIC HRIRSphere : public Resource
    {
    public:
        virtual void SetResource(const AmOsString& filePath) = 0;

        [[nodiscard]] virtual const std::vector<HRIRSphereVertex>& GetVertices() const = 0;

        [[nodiscard]] virtual const std::vector<Face>& GetFaces() const = 0;

        [[nodiscard]] virtual const HRIRSphereVertex& GetVertex(AmUInt32 index) const = 0;

        [[nodiscard]] virtual AmUInt32 GetVertexCount() const = 0;

        [[nodiscard]] virtual AmUInt32 GetFaceCount() const = 0;

        [[nodiscard]] virtual AmUInt32 GetSampleRate() const = 0;

        [[nodiscard]] virtual AmUInt32 GetIRLength() const = 0;

        /**
         * @brief Sets the sampling mode for the HRIR sphere.
         *
         * @param[in] mode The sampling mode to use.
         */
        virtual void SetSamplingMode(eHRIRSphereSamplingMode mode) = 0;

        /**
         * @brief Gets the sampling mode for the HRIR sphere.
         */
        [[nodiscard]] virtual eHRIRSphereSamplingMode GetSamplingMode() const = 0;

        /**
         * @brief Samples the HRIR sphere for the given direction.
         *
         * @param[in] direction The sound to listener direction.
         * @param[out] leftHRIR The left HRIR data.
         * @param[out] rightHRIR The right HRIR data.
         */
        virtual void Sample(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const = 0;

        virtual void Transform(const AmMat4& matrix) = 0;

        [[nodiscard]] virtual bool IsLoaded() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_HRTF_HRIR_SPHERE_H