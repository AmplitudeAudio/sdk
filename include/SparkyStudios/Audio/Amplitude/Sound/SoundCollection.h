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

#ifndef SPARK_AUDIO_SOUND_COLLECTION_H
#define SPARK_AUDIO_SOUND_COLLECTION_H

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Scheduler.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    class RealChannel;

    class BusInternalState;
    struct EngineInternalState;
    struct SoundCollectionDefinition;

    /**
     * @brief SoundCollection represent an abstract sound (like a 'whoosh'), which contains
     *        a number of pieces of audio which are selected through the configured Scheduler.
     */
    class SoundCollection
    {
    public:
        SoundCollection();

        /**
         * @brief Loads the sound collection from the given source.
         *
         * @param source The sound collection file content to load.
         * @param state The engine state to use while loading the sound collection.
         *
         * @return true if the sound collection was loaded successfully, false otherwise.
         */
        bool LoadSoundCollectionDefinition(const std::string& source, EngineInternalState* state);

        /**
         * @brief Loads the sound collection from the given file path.
         *
         * @param filename The path to the sound collection file to load.
         * @param state The engine state to use while loading the sound collection.
         *
         * @return true if the sound collection was loaded successfully, false otherwise.
         */
        bool LoadSoundCollectionDefinitionFromFile(AmOsString filename, EngineInternalState* state);

        /**
         * @brief Returns the loaded sound collection definition.
         *
         * @return The loaded sound collection definition.
         */
        [[nodiscard]] const SoundCollectionDefinition* GetSoundCollectionDefinition() const;

        /**
         * @brief Returns a Sound from this sound collection from the World scope.
         *
         * @param toSkip The list of sound instance to skip fom the selection.
         *
         * @return The selected Sound.
         */
        Sound* SelectFromWorld(const std::vector<const Sound*>& toSkip);

        /**
         * @brief Returns a Sound from this sound collection from an Entity scope.
         *
         * @param entity The entity from which pick the sound.
         * @param toSkip The list of sound instance to skip fom the selection.
         *
         * @return The selected Sound.
         */
        Sound* SelectFromEntity(const Entity& entity, const std::vector<const Sound*>& toSkip);

        /**
         * @brief Returns the unique ID of this SoundCollection.
         *
         * @return The SoundCollection unique ID.
         */
        [[nodiscard]] AmSoundCollectionID GetId() const;

        /**
         * @brief Returns the name of this SoundCollection.
         *
         * @return The SoundCollection name.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Return the bus this SoundCollection will play on.
         *
         * @return The bus this SoundCollection will play on.
         */
        [[nodiscard]] BusInternalState* GetBus() const;

        /**
         * @brief Returns the attenuation attached to this SoundCollection.
         *
         * @return The attenuation of this SoundCollection if available or nullptr.
         */
        [[nodiscard]] const Attenuation* GetAttenuation() const;

        /**
         * @brief Get the references counter of this instance.
         *
         * @return The references counter.
         */
        RefCounter* GetRefCounter();

        /**
         * @brief Returns the list of audio samples stored in this sound collection.
         *
         * @return The list of audio samples.
         */
        [[nodiscard]] const std::vector<Sound>& GetAudioSamples() const;

    private:
        static Scheduler* CreateScheduler(const SoundCollectionDefinition* definition);

        // The GetBus this SoundCollection will play on.
        BusInternalState* _bus;

        // The World scope sound scheduler
        Scheduler* _worldScopeScheduler;

        // Entity scope sound schedulers
        std::map<AmUInt64, Scheduler*> _entityScopeSchedulers;

        std::string _source;
        std::vector<Sound> _sounds;

        AmSoundCollectionID _id;
        std::string _name;

        Attenuation* _attenuation;

        RefCounter _refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_COLLECTION_H
