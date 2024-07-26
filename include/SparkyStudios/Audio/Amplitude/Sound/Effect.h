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

    /**
     * @brief Amplitude Effect.
     *
     * An effect is a sound filter applied to one or more sound objects
     * (sounds, collections, or switch containers) during playback.
     *
     * Effects are customized using parameters and each parameters can be
     * updated at runtime using a @c RTPC.
     */
    class AM_API_PUBLIC Effect : public Asset<AmEffectID>
    {
    public:
        /**
         * @brief Updates the effect parameters on each frames.
         */
        virtual void Update() = 0;
    };

    /**
     * @brief An instance of an Effect asset.
     *
     * The effect instance is the real filter applied to only one sound object
     * at a time. It is used to not share the same state between multiple sound
     * objects.
     */
    class EffectInstance
    {
    public:
        /**
         * @brief Destroys the EffectInstance.
         */
        virtual ~EffectInstance() = default;

        /**
         * @brief Get the filter instance wrapped by this effect.
         *
         * @return The filter instance.
         */
        [[nodiscard]] virtual FilterInstance* GetFilter() const = 0;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_EFFECT_H
