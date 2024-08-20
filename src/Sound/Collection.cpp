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

#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <Sound/Collection.h>
#include <Sound/Schedulers/RandomScheduler.h>
#include <Sound/Schedulers/SequenceScheduler.h>

namespace SparkyStudios::Audio::Amplitude
{
    template class AssetImpl<AmCollectionID, CollectionDefinition>;

    CollectionImpl::CollectionImpl()
        : _worldScopeScheduler(nullptr)
        , _entityScopeSchedulers()
        , _sounds()
        , _soundSettings()
    {}

    CollectionImpl::~CollectionImpl()
    {
        ampooldelete(MemoryPoolKind::Engine, Scheduler, _worldScopeScheduler);
        _worldScopeScheduler = nullptr;

        for (const auto& scheduler : _entityScopeSchedulers | std::views::values)
            ampooldelete(MemoryPoolKind::Engine, Scheduler, scheduler);

        _entityScopeSchedulers.clear();

        m_effect = nullptr;
        m_attenuation = nullptr;
    }

    Sound* CollectionImpl::SelectFromWorld(const std::vector<AmSoundID>& toSkip) const
    {
        const CollectionDefinition* definition = GetDefinition();
        if (_worldScopeScheduler == nullptr || !_worldScopeScheduler->Valid())
        {
            amLogWarning("Collection %s does not have a valid scheduler.", definition->name()->c_str());
            return nullptr;
        }

        return _worldScopeScheduler->Select(toSkip);
    }

    Sound* CollectionImpl::SelectFromEntity(const Entity& entity, const std::vector<AmSoundID>& toSkip)
    {
        const CollectionDefinition* definition = GetDefinition();
        if (const auto findIt = _entityScopeSchedulers.find(entity.GetId()); findIt == _entityScopeSchedulers.end())
            _entityScopeSchedulers.insert({ entity.GetId(), CreateScheduler(definition) });

        return _entityScopeSchedulers[entity.GetId()]->Select(toSkip);
    }

    void CollectionImpl::ResetEntityScopeScheduler(const Entity& entity)
    {
        if (const auto findIt = _entityScopeSchedulers.find(entity.GetId()); findIt != _entityScopeSchedulers.end())
            findIt->second->Reset();
    }

    void CollectionImpl::ResetWorldScopeScheduler()
    {
        if (_worldScopeScheduler == nullptr || !_worldScopeScheduler->Valid())
            return;

        _worldScopeScheduler->Reset();
    }

    Scheduler* CollectionImpl::CreateScheduler(const CollectionDefinition* definition)
    {
        Scheduler* scheduler;

        if (!definition->scheduler())
        {
            amLogDebug("Collection %s does not specify a scheduler, using the RandomScheduler by default.", definition->name()->c_str());
            scheduler = ampoolnew(MemoryPoolKind::Engine, RandomScheduler, nullptr);
        }
        else
        {
            switch (const SoundSchedulerSettings* schedulerSettings = definition->scheduler(); schedulerSettings->mode())
            {
            default:
            case SoundSchedulerMode_Random:
                scheduler = ampoolnew(MemoryPoolKind::Engine, RandomScheduler, schedulerSettings->config_as_Random());
                break;
            case SoundSchedulerMode_Sequence:
                scheduler = ampoolnew(MemoryPoolKind::Engine, SequenceScheduler, schedulerSettings->config_as_Sequence());
                break;
            }
        }

        scheduler->Init(definition);

        return scheduler;
    }

    const std::vector<AmSoundID>& CollectionImpl::GetSounds() const
    {
        return _sounds;
    }

    bool CollectionImpl::LoadDefinition(const CollectionDefinition* definition, EngineInternalState* state)
    {
        if (!definition->bus())
        {
            amLogError("Collection %s does not specify a bus.", definition->name()->c_str());
            return false;
        }

        m_bus = FindBusInternalState(state, definition->bus());
        if (!m_bus)
        {
            amLogError("Collection %s specifies an unknown bus ID: %llu.", definition->name()->c_str(), definition->bus());
            return false;
        }

        if (definition->effect() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->effect_map.find(definition->effect()); findIt != state->effect_map.end())
            {
                m_effect = findIt->second.get();
            }
            else
            {
                amLogError("Sound definition is invalid: invalid effect ID '%llu'.", definition->effect());
                return false;
            }
        }

        if (definition->attenuation() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->attenuation_map.find(definition->attenuation()); findIt != state->attenuation_map.end())
            {
                m_attenuation = findIt->second.get();
            }

            if (!m_attenuation)
            {
                amLogError(
                    "Collection %s specifies an unknown attenuation ID: %llu.", definition->name()->c_str(), definition->attenuation());
                return false;
            }
        }

        m_id = definition->id();
        m_name = definition->name()->str();

        RtpcValue::Init(m_gain, definition->gain(), 1);
        RtpcValue::Init(m_pitch, definition->pitch(), 1);
        RtpcValue::Init(m_priority, definition->priority(), 1);

        const flatbuffers::uoffset_t sampleCount = definition->sounds() ? definition->sounds()->size() : 0;

        _sounds.resize(sampleCount);
        _soundSettings.clear();

        for (flatbuffers::uoffset_t i = 0; i < sampleCount; ++i)
        {
            const auto* entry = definition->sounds()->GetAs<DefaultCollectionEntry>(i);
            AmSoundID id = entry->sound();

            if (id == kAmInvalidObjectId)
            {
                amLogError("Collection %s specifies an invalid sound ID: %llu.", definition->name()->c_str(), id);
                return false;
            }

            if (auto findIt = state->sound_map.find(id); findIt == state->sound_map.end())
            {
                amLogError("Collection %s specifies an unknown sound ID: %llu", definition->name()->c_str(), id);
                return false;
            }
            else
            {
                SoundInstanceSettings settings;
                settings.m_id = definition->id();
                settings.m_kind = SoundKind::Contained;
                settings.m_busID = definition->bus();
                settings.m_effectID = definition->effect();
                settings.m_attenuationID = definition->attenuation();
                settings.m_spatialization = definition->spatialization();
                settings.m_priority = RtpcValue(m_priority);
                RtpcValue::Init(settings.m_gain, entry->gain(), 1);
                RtpcValue::Init(settings.m_pitch, entry->pitch(), 1);
                settings.m_loop = findIt->second->_loop;
                settings.m_loopCount = findIt->second->_loopCount;

                _sounds[i] = id;
                _soundSettings[id] = settings;
            }
        }

        _worldScopeScheduler = CreateScheduler(definition);

        return true;
    }

    const CollectionDefinition* CollectionImpl::GetDefinition() const
    {
        return GetCollectionDefinition(m_source.c_str());
    }

    void CollectionImpl::AcquireReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        if (m_effect)
            m_effect->GetRefCounter()->Increment();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Increment();

        for (auto&& sound : _sounds)
            if (auto findIt = state->sound_map.find(sound); findIt != state->sound_map.end())
                findIt->second->GetRefCounter()->Increment();
    }

    void CollectionImpl::ReleaseReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        if (m_effect)
            m_effect->GetRefCounter()->Decrement();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Decrement();

        for (auto&& sound : _sounds)
            if (auto findIt = state->sound_map.find(sound); findIt != state->sound_map.end())
                findIt->second->GetRefCounter()->Decrement();
    }
} // namespace SparkyStudios::Audio::Amplitude
