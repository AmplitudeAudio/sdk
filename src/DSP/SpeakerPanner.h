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

#ifndef _AM_IMPLEMENTATION_DSP_SPEAKER_PANNER_H
#define _AM_IMPLEMENTATION_DSP_SPEAKER_PANNER_H

#include <array>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief One gain per channel of a layout.
     */
    using SpeakerGains = std::array<AmReal32, kAmMaxLayoutChannelCount>;

    /**
     * @brief Wraps an angle in degrees into (-180, 180].
     */
    AmReal32 WrapDegrees(AmReal32 degrees);

    /**
     * @brief Gets the direction of @p location seen by the listener whose view (inverse) matrix is @p listenerView.
     *
     * @param[in] location The source location in world space.
     * @param[in] listenerView The listener's inverse matrix.
     * @param[out] azimuth Degrees, positive to the left.
     * @param[out] elevation Degrees, positive up.
     *
     * @return @c false, with both angles 0 (straight ahead), when the source sits on the listener.
     */
    bool ListenerRelativeDirection(const AmVector3& location, const AmMatrix4& listenerView, AmReal32& azimuth, AmReal32& elevation);

    /**
     * @brief Constant-power pairwise 2D VBAP over a layout's non-LFE speakers.
     */
    class SpeakerPanner
    {
    public:
        /**
         * @brief Gets the speaker gains for a direction. The gains satisfy Σg² = 1; the LFE gain is always 0.
         *
         * On stereo, a direction behind the listener folds to its mirror in front (same side), then clamps to the pair.
         * Elevation is ignored: no layout has height speakers.
         *
         * @param[in] azimuth Degrees, positive to the left.
         * @param[in] elevation Degrees; unused.
         * @param[in] layout The output layout.
         */
        static SpeakerGains Pan(AmReal32 azimuth, AmReal32 elevation, const ChannelLayout& layout);

        /**
         * @brief Gets the speaker gains for a direction on the sphere. The gains satisfy Σg² = 1; the LFE gain is always 0.
         *
         * The speaker pair is found as Pan finds it (stereo folds the rear to the front). The direction is then split
         * by VBAP between that pair and an imaginary speaker straight up (elevation ≥ 0) or straight down, whose share
         * is spread evenly over every speaker. On the horizon the gains equal Pan's.
         *
         * @param[in] azimuth Degrees, positive to the left.
         * @param[in] elevation Degrees, positive up; clamped to [-90, 90].
         * @param[in] layout The output layout.
         */
        static SpeakerGains PanSphere(AmReal32 azimuth, AmReal32 elevation, const ChannelLayout& layout);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_SPEAKER_PANNER_H
