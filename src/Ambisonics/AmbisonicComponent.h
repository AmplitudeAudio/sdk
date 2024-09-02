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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_COMPONENT_H
#define _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_COMPONENT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    enum BFormatChannel
    {
        eBFormatChannel_W,
        eBFormatChannel_Y,
        eBFormatChannel_Z,
        eBFormatChannel_X,
        eBFormatChannel_V,
        eBFormatChannel_T,
        eBFormatChannel_R,
        eBFormatChannel_S,
        eBFormatChannel_U,
        eBFormatChannel_Q,
        eBFormatChannel_O,
        eBFormatChannel_M,
        eBFormatChannel_K,
        eBFormatChannel_L,
        eBFormatChannel_N,
        eBFormatChannel_P,
        eBFormatChannel_COUNT
    };

    enum SpeakersPreset
    {
        // Used internally
        eSpeakersPreset_Custom = -1,
        // 2D Speaker Setup
        eSpeakersPreset_Mono,
        eSpeakersPreset_Stereo,
        eSpeakersPreset_Surround_5_1,
        eSpeakersPreset_Surround_7_1,
        // 3D Speaker Setup
        eSpeakersPreset_CubePoints,
        eSpeakersPreset_DodecahedronFaces,
        eSpeakersPreset_LebedevGridOrder26,
        eSpeakerSetUp_COUNT
    };

    /**
     * @brief Base class for ambisonic audio components.
     */
    class AmbisonicComponent
    {
    public:
        AmbisonicComponent();
        virtual ~AmbisonicComponent() = default;

        /**
         * @brief Gets the order of the ambisonic component.
         *
         * @return The order of the ambisonic component.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetOrder() const
        {
            return m_order;
        }

        /**
         * @brief Gets whether the ambisonic component is 3D or not (has height).
         *
         * @return Whether the ambisonic component is 3D or not (has height).
         */
        [[nodiscard]] AM_INLINE bool Is3D() const
        {
            return m_is3D;
        }

        /**
         * @brief Gets the number of B-Format channels in the ambisonic component.
         *
         * @return The number of B-format channels in the ambisonic component.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetChannelCount() const
        {
            return m_channelCount;
        }

        /**
         * @brief Configures the ambisonic component with the given order, 3D status.
         *
         * @param order The order of the ambisonic component.
         * @param is3D Whether the ambisonic component is 3D or not (has height).
         *
         * @return @c true if the configuration is successful, @c false otherwise.
         */
        virtual bool Configure(AmUInt32 order, bool is3D);

        /**
         * @brief Resets the ambisonic component to its initial state, so it can be reconfigured again.
         */
        virtual void Reset() = 0;

        /**
         * @brief Refreshes the ambisonic component.
         */
        virtual void Refresh() = 0;

    protected:
        AmUInt32 m_order;
        bool m_is3D;
        AmUInt32 m_channelCount;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_COMPONENT_H
