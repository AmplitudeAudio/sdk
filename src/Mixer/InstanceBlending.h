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

#ifndef _AM_IMPLEMENTATION_MIXER_INSTANCE_BLENDING_H
#define _AM_IMPLEMENTATION_MIXER_INSTANCE_BLENDING_H

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

#include <Core/Playback/ChannelInstanceInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Computes the audible gain of an instance of a blended-mode channel, from its position.
     *
     * This is the one definition of the gain the blended mix scales an instance with. The attenuation node mixes the
     * instances with it, and the obstruction and occlusion of the instances are weighted by it, so they always agree
     * on how much each instance is heard.
     *
     * @param[in] attenuation The attenuation of the sound.
     * @param[in] listener The listener hearing the sound.
     * @param[in] location The location of the instance.
     * @param[in] orientation The orientation set on the instance, or @c nullptr when it has none. Instances are placed
     * at arbitrary world positions, so the orientation of the entity would be meaningless for them: only one set on
     * the instance itself is applied.
     *
     * @return The attenuation gain of the instance for the listener.
     */
    [[nodiscard]] AM_INLINE AmReal32 ComputeInstanceGain(
        const Attenuation& attenuation, const Listener& listener, const AmVector3& location, const Orientation* orientation)
    {
        return orientation != nullptr ? attenuation.GetGain(location, *orientation, listener) : attenuation.GetGain(location, listener);
    }

    /**
     * @brief Blends a per-instance value of a blended-mode channel into the single value its sound is rendered with.
     *
     * In blended mode, the instances are one sound going through one pipeline, so a per-instance value (an
     * obstruction, an occlusion) cannot be applied to each of them. It is averaged instead, each instance counting
     * for how much it contributes to what the listener hears: its weight scaled by its audible gain, which falls
     * with its distance to the listener. A near instance dominates a far one. Instances without a value of their
     * own count with @p fallback, the value the channel has without instances.
     *
     * @tparam GainFunction Callable taking a @c ChannelInstanceData and returning its audible gain.
     *
     * @param[in] instances The instances of the channel.
     * @param[in] fallback The value of instances that have none of their own.
     * @param[in] value The member holding the instance's value.
     * @param[in] has The member telling whether the instance has a value of its own.
     * @param[in] gain Returns the audible gain of an instance, from its position.
     * @param[out] result The blended value. Left untouched when no instance has a value of its own.
     *
     * @return @c true if at least one instance has a value of its own, and @p result is set. @c false when the
     * channel keeps its own value.
     */
    template<typename GainFunction>
    bool BlendInstanceValues(
        const std::vector<ChannelInstanceData>& instances,
        AmReal32 fallback,
        AmReal32 ChannelInstanceData::*value,
        bool ChannelInstanceData::*has,
        GainFunction&& gain,
        AmReal32& result)
    {
        bool anyExplicit = false;
        for (const auto& instance : instances)
        {
            if (instance.*has)
            {
                anyExplicit = true;
                break;
            }
        }

        if (!anyExplicit)
            return false;

        // The audible contribution, and the plain instance weights it falls back to when nothing is audible.
        AmReal32 audibleSum = 0.0f;
        AmReal32 audibleTotal = 0.0f;
        AmReal32 plainSum = 0.0f;
        AmReal32 plainTotal = 0.0f;

        for (const auto& instance : instances)
        {
            const AmReal32 instanceValue = instance.*has ? instance.*value : fallback;
            const AmReal32 audibleWeight = instance.weight * AM_MAX(gain(instance), 0.0f);

            audibleSum += instanceValue * audibleWeight;
            audibleTotal += audibleWeight;

            plainSum += instanceValue * instance.weight;
            plainTotal += instance.weight;
        }

        if (audibleTotal > kEpsilon)
            result = audibleSum / audibleTotal;
        else if (plainTotal > kEpsilon)
            result = plainSum / plainTotal;
        else
            result = fallback;

        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_INSTANCE_BLENDING_H
