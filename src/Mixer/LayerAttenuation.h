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

#ifndef _AM_IMPLEMENTATION_MIXER_LAYER_ATTENUATION_H
#define _AM_IMPLEMENTATION_MIXER_LAYER_ATTENUATION_H

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

#include <Math/LinearAlgebra.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Computes the audible attenuation gain of a layer for its listener.
     *
     * This is the one definition of how loud the attenuation lets a layer be heard. The attenuation node scales the
     * dry signal with it, and the nodes sending to the room scale their send with it, so a sound that is silent
     * past its max distance does not leak through the room.
     *
     * @param[in] layer The layer to evaluate.
     * @param[out] effectiveLocation When not @c nullptr, receives the location the gain was evaluated at: the
     * layer location, or the weighted center of its instances when it is multi-position.
     *
     * @return The attenuation gain, @c 1 when the layer has no attenuation, and @c 0 when it has one but no listener.
     */
    [[nodiscard]] inline AmReal32 ComputeLayerAttenuationGain(const AmplimixLayer& layer, AmVector3* effectiveLocation = nullptr)
    {
        AmVector3 location = layer.GetLocation();
        AmReal32 gain = 1.0f;

        if (const Attenuation* attenuation = layer.GetAttenuation(); attenuation != nullptr)
        {
            if (const Listener listener = layer.GetListener(); !listener.Valid())
            {
                // No sound without listener on an attenuated source
                gain = 0.0f;
            }
            else if (layer.IsMultiPosition())
            {
                const AmSize instanceCount = layer.GetInstanceCount();
                AmReal32 totalWeight = 0.0f;
                AmReal32 blendedGain = 0.0f;
                AmVector3 weightedLocation = kVector3Zero;

                for (AmSize i = 0; i < instanceCount; ++i)
                {
                    const AmReal32 weight = layer.GetInstanceWeight(i);

                    blendedGain += layer.GetInstanceAttenuationGain(i) * weight;
                    weightedLocation = Add(weightedLocation, Scale(layer.GetInstanceLocation(i), weight));
                    totalWeight += weight;
                }

                // Normalize by total weight
                constexpr AmReal32 kMinTotalWeight = 1e-3f;

                if (totalWeight > kMinTotalWeight)
                {
                    const AmReal32 invTotalWeight = 1.0f / totalWeight;
                    gain = blendedGain * invTotalWeight;
                    location = Scale(weightedLocation, invTotalWeight);
                }
                else
                {
                    gain = 0.0f;
                    location = kVector3Zero;
                }
            }
            else
            {
                const eSpatialization spatialization = layer.GetSpatialization();

                if (spatialization == eSpatialization_PositionOrientation ||
                    (spatialization == eSpatialization_HRTF && layer.GetEntity().Valid()))
                {
                    gain = attenuation->GetGain(location, layer.GetOrientation(), layer.GetListener());
                }
                else if (spatialization == eSpatialization_Position)
                {
                    // Position-based spatialization, or HRTF-based spatialization without entity
                    gain = attenuation->GetGain(location, layer.GetListener());
                }
            }
        }

        if (effectiveLocation != nullptr)
            *effectiveLocation = location;

        return gain;
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_LAYER_ATTENUATION_H
