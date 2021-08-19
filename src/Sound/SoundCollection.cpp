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

#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>

#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool SoundCollection::LoadSoundCollectionDefinition(const std::string& source, EngineInternalState* state)
    {
        _source = source;
        const SoundCollectionDefinition* def = GetSoundCollectionDefinition();
        flatbuffers::uoffset_t sample_count = def->audio_sample_set() ? def->audio_sample_set()->size() : 0;
        _sounds.resize(sample_count);
        for (flatbuffers::uoffset_t i = 0; i < sample_count; ++i)
        {
            const AudioSampleSetEntry* entry = def->audio_sample_set()->Get(i);
            AmString entry_filename = entry->audio_sample()->filename()->c_str();

            Sound& sound = _sounds[i];
            sound.Initialize(this);
            sound.LoadFile(entry_filename, &state->loader);
        }

        if (!def->bus())
        {
            CallLogFunc("Sound collection %s does not specify a bus.\n", def->name());
            return false;
        }

        if (!def->scheduler())
        {
            CallLogFunc("Sound collection %s does not specify a scheduler, using the RandomScheduler by default.\n", def->name());
            _scheduler = new RandomScheduler();
        }
        else
        {
            switch (def->scheduler())
            {
            case SoundScheduler_Random:
                _scheduler = new RandomScheduler();
                break;
            case SoundScheduler_Sequence:
                _scheduler = new SequenceScheduler();
                break;
            }
        }

        _scheduler->Init(def);

        if (state)
        {
            _bus = FindBusInternalState(state, def->bus()->c_str());
            if (!_bus)
            {
                CallLogFunc("Sound collection %s specifies an unknown bus: %s.\n", def->name(), def->bus()->c_str());
                return false;
            }
        }

        return true;
    }

    bool SoundCollection::LoadSoundCollectionDefinitionFromFile(const std::string& filename, EngineInternalState* state)
    {
        std::string source;
        return LoadFile(filename.c_str(), &source) && LoadSoundCollectionDefinition(source, state);
    }

    const SoundCollectionDefinition* SoundCollection::GetSoundCollectionDefinition() const
    {
        return Amplitude::GetSoundCollectionDefinition(_source.c_str());
    }

    Sound* SoundCollection::Select(const std::vector<const Sound*>& toSkip)
    {
        const SoundCollectionDefinition* sound_def = GetSoundCollectionDefinition();
        if (_scheduler == nullptr)
        {
            _scheduler = new RandomScheduler();
            _scheduler->Init(sound_def);
        }

        return _scheduler->Select(_sounds, toSkip);
    }

} // namespace SparkyStudios::Audio::Amplitude