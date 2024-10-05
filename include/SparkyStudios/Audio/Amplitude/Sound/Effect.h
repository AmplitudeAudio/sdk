// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#ifndef _AM_SOUND_EFFECT_H
#define _AM_SOUND_EFFECT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>

namespace SparkyStudios::Audio::Amplitude
{
    class FilterInstance;
    class EffectInstance;

    /**
     * @brief Amplitude Effect Asset.
     *
     * An effect is a sound filter applied to one or more sound objects
     * (sounds, collections, or switch containers) during playback.
     *
     * Effects are customized using parameters and each parameters can be
     * updated at runtime using a `Rtpc`.
     *
     * @see [Rtpc](../Rtpc/index.md), [EffectInstance](../../engine/EffectInstance/index.md)
     *
     * @ingroup assets
     */
    class AM_API_PUBLIC Effect : public Asset<AmEffectID>
    {
    public:
        /**
         * @brief Updates the effect parameters on each frames.
         */
        virtual void Update() = 0;

        /**
         * @brief Creates an instance of this effect.
         *
         * @return The effect instance.
         */
        [[nodiscard]] virtual EffectInstance* CreateInstance() const = 0;

        /**
         * @brief Destroys an instance of this effect.
         *
         * @param[in] instance The effect instance to delete.
         */
        virtual void DestroyInstance(EffectInstance* instance) const = 0;
    };

    /**
     * @brief An instance of an `Effect` asset.
     *
     * The effect instance is the real place where the filter is applied to only one sound object
     * at a time. Each effect instance has its own state, and that state is not shared across sound objects.
     *
     * @see [Effect](../../assets/Effect/index.md), [FilterInstance](../../dsp/FilterInstance/index.md)
     *
     * @ingroup engine
     */
    class EffectInstance
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~EffectInstance() = default;

        /**
         * @brief Gets the filter instance wrapped by this effect.
         *
         * @return The filter instance.
         */
        [[nodiscard]] virtual FilterInstance* GetFilter() const = 0;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_EFFECT_H
