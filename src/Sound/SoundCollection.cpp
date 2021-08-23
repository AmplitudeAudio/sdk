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

        _worldScopeScheduler = CreateScheduler(def);
        _worldScopeScheduler->Init(def);

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

    Sound* SoundCollection::SelectFromWorld(const std::vector<const Sound*>& toSkip)
    {
        const SoundCollectionDefinition* sound_def = GetSoundCollectionDefinition();
        if (_worldScopeScheduler == nullptr || !_worldScopeScheduler->Valid())
        {
            CallLogFunc("Sound collection %s does not have a valid scheduler.\n", sound_def->name()->c_str());
            return nullptr;
        }

        return _worldScopeScheduler->Select(_sounds, toSkip);
    }

    Sound* SoundCollection::SelectFromEntity(const Entity& entity, const std::vector<const Sound*>& toSkip)
    {
        const SoundCollectionDefinition* sound_def = GetSoundCollectionDefinition();
        if (auto findIt = _entityScopeSchedulers.find(entity.GetId()); findIt == _entityScopeSchedulers.end())
        {
            _entityScopeSchedulers.insert({ entity.GetId(), CreateScheduler(sound_def) });
        }

        return _entityScopeSchedulers[entity.GetId()]->Select(_sounds, toSkip);
    }

    Scheduler* SoundCollection::CreateScheduler(const SoundCollectionDefinition* definition)
    {
        if (!definition->scheduler())
        {
            CallLogFunc(
                "[Debug] Sound collection %s does not specify a scheduler, using the RandomScheduler by default.\n",
                definition->name()->c_str());
            return new RandomScheduler();
        }
        else
        {
            switch (definition->scheduler())
            {
            default:
            case SoundScheduler_Random:
                return new RandomScheduler();
            case SoundScheduler_Sequence:
                return new SequenceScheduler();
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude