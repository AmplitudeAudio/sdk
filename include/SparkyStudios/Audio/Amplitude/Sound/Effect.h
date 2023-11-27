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

#ifndef SS_AMPLITUDE_AUDIO_EFFECT_H
#define SS_AMPLITUDE_AUDIO_EFFECT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;

    struct EffectDefinition;

    class EffectInstance;

    /**
     * @brief Amplitude Effect.
     *
     * An effect is a sound filter applied to one or more sound objects
     * (sounds, collections, or switch containers) during playback.
     *
     * Effects are customized using parameters and each parameters can be
     * updated at runtime using a RTPC.
     */
    class AM_API_PUBLIC Effect final : public Asset<AmEffectID, EffectDefinition>
    {
    public:
        /**
         * @brief Creates an uninitialized Effect.
         */
        Effect();

        /**
         * \brief Destroys the Effect asset and release all associated resources.
         */
        ~Effect() override;

        /**
         * @brief Creates an instance of this effect.
         *
         * @return The effect instance.
         */
        [[nodiscard]] EffectInstance* CreateInstance() const;

        /**
         * @brief Destroys an instance of this effect.
         *
         * @param instance The effect instance to delete.
         */
        void DestroyInstance(EffectInstance* instance) const;

        /**
         * @brief Updates the effect parameters on each frames.
         */
        void Update();

        bool LoadDefinition(const EffectDefinition* definition, EngineInternalState* state) override;
        [[nodiscard]] const EffectDefinition* GetDefinition() const override;

    private:
        friend class EffectInstance;

        std::vector<EffectInstance*> _instances;

        std::vector<RtpcValue> _parameters;

        Filter* _filter;
    };

    /**
     * @brief An instance of an Effect asset.
     *
     * The effect instance is the real filter applied to only one sound object
     * at a time. It is used to not share the same state between multiple sound
     * objects.
     */
    class AM_API_PUBLIC EffectInstance
    {
    public:
        /**
         * @brief Creates a new EffectInstance.
         *
         * @param parent The parent Effect asset.
         */
        explicit EffectInstance(const Effect* parent);

        /**
         * @brief Destroys the EffectInstance.
         */
        ~EffectInstance();

        /**
         * @brief Get the filter instance wrapped by this effect.
         *
         * @return The filter instance.
         */
        [[nodiscard]] FilterInstance* GetFilter() const;

    private:
        const Effect* _parent;
        FilterInstance* _filterInstance;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_EFFECT_H
