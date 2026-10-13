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

#pragma once

#ifndef _AM_DSP_CHANNEL_LAYOUT_H
#define _AM_DSP_CHANNEL_LAYOUT_H

#include <array>
#include <initializer_list>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The largest number of channels of a named layout (7.1).
     *
     * @ingroup dsp
     */
    constexpr AmUInt16 kAmMaxLayoutChannelCount = 8;

    /**
     * @brief Spread value meaning "the native width of the source layout".
     *
     * @ingroup dsp
     */
    constexpr AmReal32 kAmSpreadNative = -1.0f;

    /**
     * @brief The named channel layouts sources and outputs can have.
     *
     * @ingroup dsp
     */
    enum eChannelLayout : AmUInt8
    {
        eChannelLayout_Mono = 0,
        eChannelLayout_Stereo,
        eChannelLayout_Quad,
        eChannelLayout_Surround_5_1,
        eChannelLayout_Surround_7_1,
        eChannelLayout_COUNT,
    };

    /**
     * @brief A speaker position in a channel layout.
     *
     * @ingroup dsp
     */
    enum eSpeaker : AmUInt8
    {
        eSpeaker_FrontLeft = 0,
        eSpeaker_FrontRight,
        eSpeaker_FrontCenter,
        eSpeaker_LowFrequency,
        eSpeaker_BackLeft,
        eSpeaker_BackRight,
        eSpeaker_SideLeft,
        eSpeaker_SideRight,
    };

    /**
     * @brief A linear map between two channel layouts: @c gains[output][input].
     *
     * @ingroup dsp
     */
    struct AM_API_PUBLIC ChannelMatrix
    {
        AmUInt16 inputCount = 0;
        AmUInt16 outputCount = 0;
        std::array<std::array<AmReal32, kAmMaxLayoutChannelCount>, kAmMaxLayoutChannelCount> gains{};
    };

    /**
     * @brief An ordered list of speakers (WAV / SMPTE order) with their azimuths.
     *
     * Azimuths are in degrees, positive to the left of the listener. Layouts are immutable singletons: compare them by
     * address or with @c operator==.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC ChannelLayout
    {
    public:
        /**
         * @brief Gets a named layout.
         *
         * @param[in] layout The layout to get.
         *
         * @return The layout.
         */
        static const ChannelLayout& Get(eChannelLayout layout);

        /**
         * @brief Gets the layout of an asset or device with @p channelCount channels.
         *
         * @param[in] channelCount The channel count: 1, 2, 4, 6 or 8.
         *
         * @return The layout, or @c nullptr when no named layout has that many channels.
         */
        static const ChannelLayout* FromChannelCount(AmUInt16 channelCount);

        /**
         * @brief Gets the largest named layout that fits in @p channelCount channels; stereo when @p channelCount is 0.
         *
         * @param[in] channelCount The channel count the device offers.
         *
         * @return The layout.
         */
        static const ChannelLayout& LargestFitting(AmUInt16 channelCount);

        /**
         * @brief Gets the ITU-R BS.775 fold-down from @p from to @p to.
         *
         * @param[in] from The source layout.
         * @param[in] to The target layout.
         *
         * @return The identity when the layouts match, the fold-down when @p to has fewer channels, and an empty matrix
         * (@c inputCount 0) when @p to has more channels than @p from.
         */
        static ChannelMatrix GetFoldDownMatrix(const ChannelLayout& from, const ChannelLayout& to);

        [[nodiscard]] AM_INLINE eChannelLayout GetType() const
        {
            return _type;
        }

        [[nodiscard]] AM_INLINE AmUInt16 GetChannelCount() const
        {
            return _channelCount;
        }

        /**
         * @brief Gets the speaker carried by @p channel.
         */
        [[nodiscard]] eSpeaker GetSpeaker(AmUInt16 channel) const;

        /**
         * @brief Gets the azimuth of @p channel in degrees, positive to the left. 0 for the LFE.
         */
        [[nodiscard]] AmReal32 GetAzimuth(AmUInt16 channel) const;

        /**
         * @brief Checks whether @p channel is the low-frequency channel.
         */
        [[nodiscard]] bool IsLfe(AmUInt16 channel) const;

        /**
         * @brief Gets the index of the low-frequency channel, or -1 when the layout has none.
         */
        [[nodiscard]] AmInt32 GetLfeChannel() const;

        /**
         * @brief Gets the channel carrying @p speaker, or -1 when the layout has no such speaker.
         */
        [[nodiscard]] AmInt32 FindSpeaker(eSpeaker speaker) const;

        /**
         * @brief Gets the layout's width, @c 2·max|azimuth| over its non-LFE speakers, in degrees.
         */
        [[nodiscard]] AmReal32 GetNativeWidth() const;

        [[nodiscard]] AM_INLINE bool operator==(const ChannelLayout& other) const
        {
            return _type == other._type;
        }

    private:
        struct SpeakerPosition
        {
            eSpeaker speaker;
            AmReal32 azimuth;
        };

        ChannelLayout(eChannelLayout type, std::initializer_list<SpeakerPosition> speakers);

        eChannelLayout _type;
        AmUInt16 _channelCount;
        std::array<eSpeaker, kAmMaxLayoutChannelCount> _speakers{};
        std::array<AmReal32, kAmMaxLayoutChannelCount> _azimuths{};
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_CHANNEL_LAYOUT_H
