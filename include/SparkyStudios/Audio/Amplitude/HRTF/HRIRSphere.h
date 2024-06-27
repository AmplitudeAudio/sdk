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

#ifndef SS_AMPLITUDE_AUDIO_HRTF_HRIRSPHERE_H
#define SS_AMPLITUDE_AUDIO_HRTF_HRIRSPHERE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/IO/File.h>
#include <SparkyStudios/Audio/Amplitude/IO/Resource.h>

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
         *(http://sound.media.mit.edu/resources/KEMAR.html).
         */
        eHRIRSphereDatasetModel_MIT = 1,

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
    };

    /**
     * @brief A 3D sphere of HRTF data.
     */
    class AM_API_PUBLIC HRIRSphere : public Resource
    {
    public:
        /**
         * @brief Constructs a new @c HRIRSphere instance.
         *
         * @param[in] filePath The path to the HRIR sphere file.
         */
        explicit HRIRSphere(const AmOsString& filePath);

        ~HRIRSphere() override;

        void Load(const FileSystem* loader) override;

        [[nodiscard]] const std::vector<HRIRSphereVertex>& GetVertices() const;

        [[nodiscard]] const std::vector<AmUInt32>& GetIndices() const;

        [[nodiscard]] const HRIRSphereVertex& GetVertex(AmUInt32 index) const;

        [[nodiscard]] AmUInt32 GetVertexCount() const;

        [[nodiscard]] AmUInt32 GetIndexCount() const;

        [[nodiscard]] AmUInt32 GetSampleRate() const;

        [[nodiscard]] AmUInt32 GetIRLength() const;

    private:
        HRIRSphereFileHeaderDescription _header;
        std::vector<HRIRSphereVertex> _vertices;
        std::vector<AmUInt32> _indices;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_HRTF_HRIRSPHERE_H