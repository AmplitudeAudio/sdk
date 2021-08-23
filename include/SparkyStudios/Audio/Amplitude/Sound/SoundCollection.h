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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>

#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Scheduler.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    class RealChannel;

    class BusInternalState;
    struct EngineInternalState;
    struct SoundCollectionDefinition;

    /**
     * SoundCollection represent an abstract sound (like a 'whoosh'), which contains
     * a number of pieces of audio with weighted probabilities to choose between
     * randomly when played. It holds objects of type `Audio`, which can be either
     * Sounds or Music
     */
    class SoundCollection
    {
    public:
        SoundCollection()
            : _bus(nullptr)
            , _worldScopeScheduler(nullptr)
            , _entityScopeSchedulers()
            , _source()
            , _sounds()
            , _refCounter()
        {}

        // Load the given flatbuffer data representing a SoundCollectionDef.
        bool LoadSoundCollectionDefinition(const std::string& source, EngineInternalState* state);

        // Load the given flatbuffer binary file containing a SoundDef.
        bool LoadSoundCollectionDefinitionFromFile(const std::string& filename, EngineInternalState* state);

        // Return the SoundDef.
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

        // Return the bus this SoundCollection will play on.
        BusInternalState* GetBus()
        {
            return _bus;
        }

        RefCounter* GetRefCounter()
        {
            return &_refCounter;
        }

        [[nodiscard]] const std::vector<Sound>& GetAudioSamples() const
        {
            return _sounds;
        }

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

        RefCounter _refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_COLLECTION_H
