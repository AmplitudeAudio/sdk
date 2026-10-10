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

#ifndef _AM_IMPLEMENTATION_DSP_SPREAD_EMITTERS_H
#define _AM_IMPLEMENTATION_DSP_SPREAD_EMITTERS_H

#include <array>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief One source channel placed as a virtual emitter.
     */
    struct SpreadEmitter
    {
        AmUInt16 channel = 0;     ///< The source channel.
        AmReal32 azimuth = 0.0f;  ///< Degrees, positive to the left, listener-relative.
        AmReal32 elevation = 0.0f;
        AmReal32 weight = 1.0f;   ///< Gain applied to the channel before panning.
    };

    struct SpreadEmitterSet
    {
        AmUInt16 count = 0;
        std::array<SpreadEmitter, kAmMaxLayoutChannelCount> emitters{};
    };

    /**
     * @brief Places the non-LFE channels of a source around its direction, scaled by a spread angle.
     */
    class SpreadEmitters
    {
    public:
        /**
         * @brief Turns a spread setting into degrees: @c kAmSpreadNative gives the source's native width; anything
         * else is clamped to [0, 360] (NaN gives 0).
         */
        static AmReal32 Resolve(AmReal32 spread, const ChannelLayout& source);

        /**
         * @brief Computes the emitters, in source channel order with the LFE skipped.
         *
         * Up to the native width, channel @c i sits at @c centre + a_i·spread/native; above it, it moves linearly
         * towards evenly spaced positions reached at 360. The weight goes from the channel's ITU fold-down-to-mono
         * coefficient at spread 0 to 1 at the native width.
         *
         * @param[in] source The source layout.
         * @param[in] centreAzimuth The sound's direction, degrees, positive to the left.
         * @param[in] centreElevation The sound's elevation, degrees; every emitter keeps it.
         * @param[in] spread Degrees, already resolved (0-360).
         */
        static SpreadEmitterSet Compute(const ChannelLayout& source, AmReal32 centreAzimuth, AmReal32 centreElevation, AmReal32 spread);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_SPREAD_EMITTERS_H
