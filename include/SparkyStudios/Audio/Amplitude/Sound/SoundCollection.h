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

#include <memory>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Sound/RefCounter.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundCollectionScheduler.h>

namespace SparkyStudios::Audio::Amplitude
{
    class RealChannel;
    class Sound;

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
            , _scheduler(nullptr)
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

        // Return a random piece of audio from the set of audio for this sound.
        Sound* Select();

        // Return the bus this SoundCollection will play on.
        BusInternalState* GetBus()
        {
            return _bus;
        }

        RefCounter* GetRefCounter()
        {
            return &_refCounter;
        }

    private:
        // The GetBus this SoundCollection will play on.
        BusInternalState* _bus;

        // The sound collection scheduler
        SoundCollectionScheduler* _scheduler;

        std::string _source;
        std::vector<Sound> _sounds;

        RefCounter _refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_COLLECTION_H
