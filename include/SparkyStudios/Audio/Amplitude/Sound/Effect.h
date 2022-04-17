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
     * updated at runtime using RTPC.
     */
    class Effect
    {
        friend class EffectInstance;

    public:
        Effect();
        ~Effect();

        /**
         * @brief Loads the effect from the given source.
         *
         * @param source The effect file content to load.
         *
         * @return true if the effect was loaded successfully, false otherwise.
         */
        bool LoadEffectDefinition(const std::string& source);

        /**
         * @brief Loads the effect from the given file path.
         *
         * @param filename The path to the effect file to load.
         *
         * @return true if the effect was loaded successfully, false otherwise.
         */
        bool LoadEffectDefinitionFromFile(AmOsString filename);

        /**
         * @brief Acquires referenced objects in this Effect.
         *
         * @param state The engine state used while loading the effect.
         */
        void AcquireReferences(EngineInternalState* state);

        /**
         * @brief Releases the references acquired when loading the effect.
         *
         * @param state The engine state used while loading the effect.
         */
        void ReleaseReferences(EngineInternalState* state);

        /**
         * @brief Returns the loaded effect definition.
         *
         * @return The loaded effect definition.
         */
        [[nodiscard]] const EffectDefinition* GetEffectDefinition() const;

        /**
         * @brief Create an instance of this effect.
         *
         * @return The effect instance.
         */
        [[nodiscard]] EffectInstance* CreateInstance() const;

        /**
         * @brief Deletes an instance of this effect.
         *
         * @param instance The effect instance to delete.
         */
        void DeleteInstance(EffectInstance* instance) const;

        /**
         * @brief Updates parameters on each frames.
         */
        void Update();

        /**
         * @brief Returns the unique ID of this Effect.
         *
         * @return The Effect unique ID.
         */
        [[nodiscard]] AmEffectID GetId() const;

        /**
         * @brief Get the name of this effect.
         *
         * @return The effect name.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Get the references counter for this object.
         *
         * @return The references counter.
         */
        RefCounter* GetRefCounter();

        /**
         * @brief Get the references counter for this object.
         *
         * @return The references counter.
         */
        [[nodiscard]] const RefCounter* GetRefCounter() const;

    private:
        std::string _source;

        AmEffectID _id;
        std::string _name;

        std::vector<RtpcValue> _parameters;

        RefCounter _refCounter;

        Filter* _filter;
    };

    /**
     * @brief The actual effect applied to a SoundInstance.
     */
    class EffectInstance
    {
    public:
        explicit EffectInstance(const Effect* parent);
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
