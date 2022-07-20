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

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Scheduler.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;
    struct CollectionDefinition;

    class RealChannel;
    class BusInternalState;

    /**
     * @brief Collection represent an abstract sound (like a 'whoosh'), which contains
     *        a number of pieces of audio which are selected through the configured Scheduler.
     */
    class Collection : public SoundObject
    {
        friend class Sound;

    public:
        Collection();
        ~Collection() override;

        /**
         * @brief Loads the collection from the given source.
         *
         * @param source The collection file content to load.
         * @param state The engine state to use while loading the collection.
         *
         * @return true if the collection was loaded successfully, false otherwise.
         */
        bool LoadDefinition(const std::string& source, EngineInternalState* state) override;

        /**
         * @brief Loads the collection from the given file path.
         *
         * @param filename The path to the collection file to load.
         * @param state The engine state to use while loading the collection.
         *
         * @return true if the collection was loaded successfully, false otherwise.
         */
        bool LoadDefinitionFromFile(const AmOsString& filename, EngineInternalState* state) override;

        /**
         * @brief Acquires referenced objects in this Collection.
         *
         * @param state The engine state used while loading the collection.
         */
        void AcquireReferences(EngineInternalState* state) override;

        /**
         * @brief Releases the references acquired when loading the collection.
         *
         * @param state The engine state used while loading the collection.
         */
        void ReleaseReferences(EngineInternalState* state) override;

        /**
         * @brief Returns the loaded collection definition.
         *
         * @return The loaded collection definition.
         */
        [[nodiscard]] const CollectionDefinition* GetCollectionDefinition() const;

        /**
         * @brief Returns a Sound from this collection from the World scope.
         *
         * @param toSkip The list of Sound IDs to skip fom the selection.
         *
         * @return The selected Sound.
         */
        Sound* SelectFromWorld(const std::vector<AmSoundID>& toSkip);

        /**
         * @brief Returns a Sound from this collection from an Entity scope.
         *
         * @param entity The entity from which pick the sound.
         * @param toSkip The list of Sound IDs to skip fom the selection.
         *
         * @return The selected Sound.
         */
        Sound* SelectFromEntity(const Entity& entity, const std::vector<AmSoundID>& toSkip);

        /**
         * @brief Resets the internal of the scheduler running for the given Entity.
         *
         * @param entity The entity to reset the scheduler for.
         */
        void ResetEntityScopeScheduler(const Entity& entity);

        /**
         * @brief Returns the list of Sound objects referenced in this collection.
         *
         * @return The list of Sound IDs.
         */
        [[nodiscard]] const std::vector<AmSoundID>& GetAudioSamples() const;

    private:
        static Scheduler* CreateScheduler(const CollectionDefinition* definition);

        // The World scope sound scheduler
        Scheduler* _worldScopeScheduler;

        // Entity scope sound schedulers
        std::map<AmUInt64, Scheduler*> _entityScopeSchedulers;

        std::string _source;
        std::vector<AmSoundID> _sounds;
        std::map<AmSoundID, SoundInstanceSettings> _soundSettings;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_COLLECTION_H
