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
     *
     * @ingroup core
     */
    enum eHRIRSphereDatasetModel : AmUInt8
    {
        /**
         * The HRIR sphere uses data from the IRCAM (LISTEN) dataset.
         * (http://recherche.ircam.fr/equipes/salles/listen/download.html)
         */
        eHRIRSphereDatasetModel_IRCAM = 0,

        /**
         * The HRIR sphere uses data from the MIT (KEMAR) dataset.
         * (http://sound.media.mit.edu/resources/KEMAR.html).
         */
        eHRIRSphereDatasetModel_MIT = 1,

        /**
         * The HRIR sphere uses data from the SADIE II dataset.
         * (https://www.york.ac.uk/sadie-project/database.html).
         */
        eHRIRSphereDatasetModel_SADIE = 2,

        /**
         * The HRIR sphere uses data from a SOFA (Spatially Oriented Format for Acoustics) file.
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
     *
     * @ingroup core
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
     * @brief A vertex in a HRIR sphere.
     *
     * @ingroup core
     */
    struct HRIRSphereVertex
    {
        /**
         * @brief The cartesian position of the vertex.
         */
        AmVec3 m_Position;

        /**
         * @brief The HRIR data for the left ear.
         */
        std::vector<AmReal32> m_LeftIR;

        /**
         * @brief The HRIR data for the right ear.
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
     * @brief Represents a Head-Related Impulse Response (HRIR) sphere.
     *
     * The HRIR sphere is a 3D representation of spatial audio dataset
     * characterized by vertices, faces, and impulse response data that
     * can be used for audio spatialization. It provides methods to query
     * and manipulate the dataset.
     *
     * @ingroup core
     */
    class AM_API_PUBLIC HRIRSphere : public Resource
    {
    public:
        /**
         * @brief Sets the resource file path for the HRIR sphere.
         *
         * @param[in] filePath The file path of the resource to be set.
         */
        virtual void SetResource(const AmOsString& filePath) = 0;

        /**
         * @brief Gets the vertices that make up the HRIR sphere.
         *
         * @return A constant reference to the vector of HRIRSphereVertex instances representing the vertices of the sphere.
         */
        [[nodiscard]] virtual const std::vector<HRIRSphereVertex>& GetVertices() const = 0;

        /**
         * @brief Gets the faces that make up the HRIR sphere.
         *
         * @return A constant reference to the vector of Face instances representing the triangulated faces of the sphere.
         */
        [[nodiscard]] virtual const std::vector<Face>& GetFaces() const = 0;

        /**
         * @brief Retrieves a specific vertex from the HRIR sphere dataset.
         *
         * This method returns a constant reference to the HRIRSphereVertex
         * at the specified index in the dataset.
         *
         * @param[in] index The index of the vertex to retrieve.
         *
         * @return A constant reference to the HRIRSphereVertex at the given index.
         */
        [[nodiscard]] virtual const HRIRSphereVertex& GetVertex(AmUInt32 index) const = 0;

        /**
         * @brief Retrieves the total number of vertices in the HRIR sphere dataset.
         *
         * @return The number of vertices in the HRIR sphere.
         */
        [[nodiscard]] virtual AmUInt32 GetVertexCount() const = 0;

        /**
         * @brief Retrieves the total number of faces in the HRIR sphere dataset.
         *
         * @return The number of faces in the HRIR sphere.
         */
        [[nodiscard]] virtual AmUInt32 GetFaceCount() const = 0;

        /**
         * @brief Retrieves the sample rate of the HRIR sphere dataset.
         *
         * This method returns the sample rate at which the HRIR data is recorded
         * or processed.
         *
         * @return The sample rate of the HRIR sphere dataset.
         */
        [[nodiscard]] virtual AmUInt32 GetSampleRate() const = 0;

        /**
         * @brief Retrieves the impulse response length of the HRIR sphere.
         *
         * This method returns the length of the impulse response (IR) used
         * in HRIR (Head-Related Impulse Response) processing.
         *
         * @return The length of the impulse response in samples.
         */
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
         * @param[in] direction The direction starting from the sound's position to the listener's position.
         * @param[out] leftHRIR The left HRIR data.
         * @param[out] rightHRIR The right HRIR data.
         */
        virtual void Sample(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const = 0;

        /**
         * @brief Applies a transformation matrix to the HRIR sphere.
         *
         * This method applies the specified transformation matrix to modify
         * the spatial properties of the HRIR sphere, such as its orientation
         * or scale in a 3D space.
         *
         * @param[in] matrix The transformation matrix to be applied.
         */
        virtual void Transform(const AmMat4& matrix) = 0;

        /**
         * @brief Checks if the HRIR sphere resource is loaded.
         *
         * This method determines whether the HRIR sphere dataset is loaded
         * and ready for use.
         *
         * @return @c true if the HRIR sphere resource is loaded, @c false otherwise.
         */
        [[nodiscard]] virtual bool IsLoaded() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_HRTF_HRIR_SPHERE_H