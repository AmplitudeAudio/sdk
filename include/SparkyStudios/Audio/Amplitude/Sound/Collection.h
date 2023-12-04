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

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Scheduler.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineInternalState;
    class BusInternalState;

    struct CollectionDefinition;

    class RealChannel;

    /**
     * @brief Amplitude Collection.
     *
     * A Collection is a container sound object that group multiple sounds over the same name. Only
     * one sound can be playing at a time in the same collection, and the sound picked for playback
     * is choosen by the collection's Scheduler.
     */
    class AM_API_PUBLIC Collection final
        : public SoundObject
        , public Asset<AmCollectionID, CollectionDefinition>
    {
    public:
        /**
         * @brief Creates an uninitialized collection.
         */
        Collection();

        /**
         * @bref Destroys the collection asset and all related resources.
         */
        ~Collection() override;

        /**
         * @brief Returns a Sound from this collection from the World scope.
         *
         * @param toSkip The list of Sound IDs to skip fom the selection.
         *
         * @return The selected Sound.
         */
        [[nodiscard]] Sound* SelectFromWorld(const std::vector<AmSoundID>& toSkip = {}) const;

        /**
         * @brief Returns a Sound from this collection from an Entity scope.
         *
         * @param entity The entity from which pick the sound.
         * @param toSkip The list of Sound IDs to skip fom the selection.
         *
         * @return The selected Sound.
         */
        Sound* SelectFromEntity(const Entity& entity, const std::vector<AmSoundID>& toSkip = {});

        /**
         * @brief Resets the internal state of the scheduler running for the given Entity.
         *
         * @param entity The entity for which reset the scheduler state.
         */
        void ResetEntityScopeScheduler(const Entity& entity);

        /**
         * @brief Returns the list of Sound objects referenced in this collection.
         *
         * @return The list of Sound IDs.
         */
        [[nodiscard]] const std::vector<AmSoundID>& GetSounds() const;

        bool LoadDefinition(const CollectionDefinition* definition, EngineInternalState* state) override;
        [[nodiscard]] const CollectionDefinition* GetDefinition() const override;
        void AcquireReferences(EngineInternalState* state) override;
        void ReleaseReferences(EngineInternalState* state) override;

    private:
        friend class Sound;

        static Scheduler* CreateScheduler(const CollectionDefinition* definition);

        // The World scope sound scheduler
        Scheduler* _worldScopeScheduler;

        // Entity scope sound schedulers
        std::map<AmUInt64, Scheduler*> _entityScopeSchedulers;

        std::vector<AmSoundID> _sounds;
        std::map<AmSoundID, SoundInstanceSettings> _soundSettings;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_COLLECTION_H
