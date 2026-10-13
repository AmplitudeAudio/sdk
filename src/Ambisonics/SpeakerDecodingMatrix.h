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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_SPEAKER_DECODING_MATRIX_H
#define _AM_IMPLEMENTATION_AMBISONICS_SPEAKER_DECODING_MATRIX_H

#include <array>
#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/DSP/ChannelLayout.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Immutable AllRAD decoder from a 3D SN3D/ACN sound field to a channel layout's speakers.
     *
     * The sound field is decoded onto a weighted 1202-point Lebedev grid, and each grid direction is panned onto the
     * speakers with SpeakerPanner::PanSphere, so a source on the horizon lands where the direct path puts it. The matrix
     * gives a point source unit power on average (the direct path's Σg² = 1); the high-frequency gains are max-rE
     * weights scaled so the high band has the low band's energy. Built once per layout and order and shared.
     */
    class SpeakerDecodingMatrix
    {
    public:
        static constexpr AmUInt32 kMaxAmbisonicChannels = 16;

        /**
         * @brief Builds the decoder of a layout.
         *
         * @param order The ambisonic order, 1 to 3.
         * @param layout The output layout.
         *
         * @return The matrix, or nullptr when the order is out of range.
         */
        static std::shared_ptr<const SpeakerDecodingMatrix> Build(AmUInt32 order, const ChannelLayout& layout);

        SpeakerDecodingMatrix() = default;

        /**
         * @brief Gets the ambisonic order.
         */
        [[nodiscard]] AmUInt32 GetOrder() const;

        /**
         * @brief Gets the output layout.
         */
        [[nodiscard]] eChannelLayout GetLayout() const;

        /**
         * @brief Gets the number of ambisonic channels, (order + 1)².
         */
        [[nodiscard]] AmUInt32 GetChannelCount() const;

        /**
         * @brief Gets the gain of an ambisonic channel on an output channel; 0 for the LFE.
         */
        [[nodiscard]] AmReal32 GetCoefficient(AmUInt16 outputChannel, AmUInt32 ambisonicChannel) const;

        /**
         * @brief Gets the per-degree high-frequency gains to give AmbisonicShelfFilter::SetHighFrequencyGains.
         */
        [[nodiscard]] const std::vector<AmReal32>& GetHighFrequencyGains() const;

    private:
        AmUInt32 _order = 0;
        eChannelLayout _layout = eChannelLayout_Mono;
        AmUInt32 _channelCount = 0;
        std::array<std::array<AmReal32, kMaxAmbisonicChannels>, kAmMaxLayoutChannelCount> _coefficients{};
        std::vector<AmReal32> _highFrequencyGains;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_SPEAKER_DECODING_MATRIX_H
