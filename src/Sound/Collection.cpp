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
#include <Sound/Schedulers/RandomScheduler.h>
#include <Sound/Schedulers/SequenceScheduler.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    Collection::Collection()
        : _bus(nullptr)
        , _worldScopeScheduler(nullptr)
        , _entityScopeSchedulers()
        , _source()
        , _sounds()
        , _soundSettings()
        , _id(kAmInvalidObjectId)
        , _name()
        , _gain()
        , _priority()
        , _effect(nullptr)
        , _attenuation(nullptr)
        , _refCounter()
    {}

    Collection::~Collection()
    {
        delete _worldScopeScheduler;
        _worldScopeScheduler = nullptr;
        _entityScopeSchedulers.clear();
        _effect = nullptr;
        _attenuation = nullptr;
    }

    bool Collection::LoadCollectionDefinition(const std::string& source, EngineInternalState* state)
    {
        // Ensure we do not load the collection more than once
        AMPLITUDE_ASSERT(_id == kAmInvalidObjectId);

        _source = source;
        const CollectionDefinition* def = GetCollectionDefinition();

        if (!def->bus())
        {
            CallLogFunc("Collection %s does not specify a bus.\n", def->name()->c_str());
            return false;
        }

        if (state)
        {
            _bus = FindBusInternalState(state, def->bus());
            if (!_bus)
            {
                CallLogFunc("Collection %s specifies an unknown bus ID: %u.\n", def->name(), def->bus());
                return false;
            }

            if (def->effect() != kAmInvalidObjectId)
            {
                if (const auto findIt = state->effect_map.find(def->effect()); findIt != state->effect_map.end())
                {
                    _effect = findIt->second.get();
                }
                else
                {
                    CallLogFunc("[ERROR] Sound definition is invalid: invalid effect ID \"%u\"", def->effect());
                    return false;
                }
            }

            if (def->attenuation() != kAmInvalidObjectId)
            {
                if (auto findIt = state->attenuation_map.find(def->attenuation()); findIt != state->attenuation_map.end())
                {
                    _attenuation = findIt->second.get();
                }

                if (!_attenuation)
                {
                    CallLogFunc("Collection %s specifies an unknown attenuation ID: %u.\n", def->name(), def->attenuation());
                    return false;
                }
            }
        }

        _id = def->id();
        _name = def->name()->str();

        _gain = RtpcValue(def->gain());
        _priority = RtpcValue(def->priority());

        flatbuffers::uoffset_t sample_count = def->sounds() ? def->sounds()->size() : 0;

        _sounds.resize(sample_count);
        _soundSettings.clear();

        for (flatbuffers::uoffset_t i = 0; i < sample_count; ++i)
        {
            const DefaultCollectionEntry* entry = def->sounds()->GetAs<DefaultCollectionEntry>(i);
            AmSoundID id = entry->sound();

            if (id == kAmInvalidObjectId)
            {
                CallLogFunc("[ERROR] Collection %s specifies an invalid sound ID: %u.", def->name()->c_str(), id);
                return false;
            }

            if (auto findIt = state->sound_map.find(id); findIt == state->sound_map.end())
            {
                CallLogFunc("[ERROR] Collection %s specifies an unknown sound ID: %u", def->name()->c_str(), id);
                return false;
            }
            else
            {
                SoundInstanceSettings settings;
                settings.m_id = def->id();
                settings.m_kind = SoundKind::Contained;
                settings.m_busID = def->bus();
                settings.m_effectID = def->effect();
                settings.m_attenuationID = def->attenuation();
                settings.m_spatialization = def->spatialization();
                settings.m_priority = _priority;
                settings.m_gain = RtpcValue(entry->gain());
                settings.m_loop = findIt->second->_loop;
                settings.m_loopCount = findIt->second->_loopCount;

                _sounds[i] = id;
                _soundSettings[id] = settings;
            }
        }

        _worldScopeScheduler = CreateScheduler(def);

        return true;
    }

    bool Collection::LoadCollectionDefinitionFromFile(AmOsString filename, EngineInternalState* state)
    {
        std::string source;
        return Amplitude::LoadFile(filename, &source) && LoadCollectionDefinition(source, state);
    }

    void Collection::AcquireReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        if (_effect)
        {
            _effect->GetRefCounter()->Increment();
        }

        if (_attenuation)
        {
            _attenuation->GetRefCounter()->Increment();
        }

        for (auto&& sound : _sounds)
        {
            if (auto findIt = state->sound_map.find(sound); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Increment();
            }
        }
    }

    void Collection::ReleaseReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        if (_effect)
        {
            _effect->GetRefCounter()->Decrement();
        }

        if (_attenuation)
        {
            _attenuation->GetRefCounter()->Decrement();
        }

        for (auto&& sound : _sounds)
        {
            if (auto findIt = state->sound_map.find(sound); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Decrement();
            }
        }
    }

    const CollectionDefinition* Collection::GetCollectionDefinition() const
    {
        return Amplitude::GetCollectionDefinition(_source.c_str());
    }

    Sound* Collection::SelectFromWorld(const std::vector<AmSoundID>& toSkip)
    {
        const CollectionDefinition* sound_def = GetCollectionDefinition();
        if (_worldScopeScheduler == nullptr || !_worldScopeScheduler->Valid())
        {
            CallLogFunc("Collection %s does not have a valid scheduler.\n", sound_def->name()->c_str());
            return nullptr;
        }

        return _worldScopeScheduler->Select(toSkip);
    }

    Sound* Collection::SelectFromEntity(const Entity& entity, const std::vector<AmSoundID>& toSkip)
    {
        const CollectionDefinition* sound_def = GetCollectionDefinition();
        if (auto findIt = _entityScopeSchedulers.find(entity.GetId()); findIt == _entityScopeSchedulers.end())
        {
            _entityScopeSchedulers.insert({ entity.GetId(), CreateScheduler(sound_def) });
        }

        return _entityScopeSchedulers[entity.GetId()]->Select(toSkip);
    }

    void Collection::ResetEntityScopeScheduler(const Entity& entity)
    {
        if (auto findIt = _entityScopeSchedulers.find(entity.GetId()); findIt != _entityScopeSchedulers.end())
        {
            findIt->second->Reset();
        }
    }

    Scheduler* Collection::CreateScheduler(const CollectionDefinition* definition)
    {
        Scheduler* scheduler;

        if (!definition->scheduler())
        {
            CallLogFunc(
                "[Debug] Collection %s does not specify a scheduler, using the RandomScheduler by default.\n", definition->name()->c_str());
            scheduler = new RandomScheduler(nullptr);
        }
        else
        {
            const SoundSchedulerSettings* schedulerSettings = definition->scheduler();
            switch (schedulerSettings->mode())
            {
            default:
            case SoundSchedulerMode_Random:
                scheduler = new RandomScheduler(schedulerSettings->config_as_Random());
                break;
            case SoundSchedulerMode_Sequence:
                scheduler = new SequenceScheduler(schedulerSettings->config_as_Sequence());
                break;
            }
        }

        scheduler->Init(definition);

        return scheduler;
    }

    const RtpcValue& Collection::GetGain() const
    {
        return _gain;
    }

    const RtpcValue& Collection::GetPriority() const
    {
        return _priority;
    }

    AmBankID Collection::GetId() const
    {
        return _id;
    }

    const std::string& Collection::GetName() const
    {
        return _name;
    }

    BusInternalState* Collection::GetBus() const
    {
        return _bus;
    }

    RefCounter* Collection::GetRefCounter()
    {
        return &_refCounter;
    }

    const std::vector<AmSoundID>& Collection::GetAudioSamples() const
    {
        return _sounds;
    }

    const Effect* Collection::GetEffect() const
    {
        return _effect;
    }

    const Attenuation* Collection::GetAttenuation() const
    {
        return _attenuation;
    }
} // namespace SparkyStudios::Audio::Amplitude