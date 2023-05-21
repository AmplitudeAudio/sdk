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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>

#include "switch_container_definition_generated.h"
#include "switch_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    SwitchContainer::SwitchContainer()
        : SoundObject()
        , _switch(nullptr)
        , _source()
        , _sounds()
    {}

    SwitchContainer::~SwitchContainer()
    {
        _switch = nullptr;
        _sounds.clear();
        m_effect = nullptr;
        m_attenuation = nullptr;
    }

    bool SwitchContainer::LoadDefinition(const AmString& source, EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id == kAmInvalidObjectId);

        _source = source;
        const SwitchContainerDefinition* definition = GetSwitchContainerDefinition();

        if (!definition->bus())
        {
            CallLogFunc("[ERROR] SwitchContainer %s does not specify a bus.\n", definition->name()->c_str());
            return false;
        }

        if (!definition->switch_group())
        {
            CallLogFunc("[ERROR] SwitchContainer %s does not specify a switch.\n", definition->name()->c_str());
            return false;
        }

        m_bus = FindBusInternalState(state, definition->bus());
        if (!m_bus)
        {
            CallLogFunc("[ERROR] SwitchContainer %s specifies an unknown bus ID: %u.\n", definition->name(), definition->bus());
            return false;
        }

        if (const auto findIt = state->switch_map.find(definition->switch_group()); findIt != state->switch_map.end())
        {
            _switch = findIt->second.get();
        }

        if (definition->effect() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->effect_map.find(definition->effect()); findIt != state->effect_map.end())
            {
                m_effect = findIt->second.get();
            }
            else
            {
                CallLogFunc("[ERROR] Sound definition is invalid: invalid effect ID \"%u\"", definition->effect());
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
                CallLogFunc(
                    "[ERROR] SwitchContainer " AM_OS_CHAR_FMT " specifies an unknown attenuation ID: %u.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()), definition->attenuation());
                return false;
            }
        }

        m_id = definition->id();
        m_name = definition->name()->str();

        m_gain = RtpcValue(definition->gain());
        m_priority = RtpcValue(definition->priority());

        const auto& states = _switch->GetSwitchStates();
        for (const auto& switchState : states)
        {
            if (_sounds.count(switchState.m_id) == 0)
            {
                _sounds.insert({ switchState.m_id, std::vector<SwitchContainerItem>() });
            }
        }

        const flatbuffers::uoffset_t count = definition->entries() ? definition->entries()->size() : 0;
        for (flatbuffers::uoffset_t i = 0; i < count; ++i)
        {
            const SwitchContainerEntry* entry = definition->entries()->Get(i);
            AmObjectID id = entry->object();

            if (id == kAmInvalidObjectId)
            {
                CallLogFunc("[ERROR] SwitchContainer %s specifies an invalid sound object ID: %u.", definition->name()->c_str(), id);
                return false;
            }

            if (state->sound_map.count(id) == 0 && state->collection_map.count(id) == 0)
            {
                CallLogFunc(
                    "[ERROR] SwitchContainer %s specifies an unknown sound object ID: %u. It's neither a Sound nor a Collection.",
                    definition->name()->c_str(), id);
                return false;
            }

            // Setup entry Faders
            Fader* fader = Fader::Find(entry->fade_in()->fader()->str());
            FaderInstance* faderInstance = fader->CreateInstance();
            faderInstance->SetDuration(entry->fade_in()->duration());

            _fadersIn[id] = std::make_tuple(fader, faderInstance);

            fader = Fader::Find(entry->fade_out()->fader()->str());
            faderInstance = fader->CreateInstance();
            faderInstance->SetDuration(entry->fade_out()->duration());

            _fadersOut[id] = std::make_tuple(fader, faderInstance);

            SwitchContainerItem item;
            item.m_id = id;
            item.m_continueBetweenStates = entry->continue_between_states();
            item.m_fadeInDuration = entry->fade_in()->duration();
            item.m_fadeOutDuration = entry->fade_out()->duration();
            item.m_fadeInAlgorithm = entry->fade_in()->fader()->str();
            item.m_fadeOutAlgorithm = entry->fade_out()->fader()->str();
            item.m_gain = RtpcValue(entry->gain());

            const flatbuffers::uoffset_t statesCount = entry->switch_states()->size();
            for (flatbuffers::uoffset_t j = 0; j < statesCount; ++j)
            {
                AmObjectID stateId = entry->switch_states()->Get(j);
                _sounds[stateId].push_back(item);
            }
        }

        return true;
    }

    bool SwitchContainer::LoadDefinitionFromFile(const AmOsString& filename, EngineInternalState* state)
    {
        std::string source;
        return Amplitude::LoadFile(filename, &source) && LoadDefinition(source, state);
    }

    void SwitchContainer::AcquireReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        _switch->GetRefCounter()->Increment();

        if (m_effect)
        {
            m_effect->GetRefCounter()->Increment();
        }

        if (m_attenuation)
        {
            m_attenuation->GetRefCounter()->Increment();
        }

        for (auto&& sound : _sounds)
        {
            if (auto findIt = state->sound_map.find(sound.first); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Increment();
                continue;
            }

            if (auto findIt = state->collection_map.find(sound.first); findIt != state->collection_map.end())
            {
                findIt->second->GetRefCounter()->Increment();
                continue;
            }
        }
    }

    void SwitchContainer::ReleaseReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        _switch->GetRefCounter()->Decrement();

        if (m_effect)
        {
            m_effect->GetRefCounter()->Decrement();
        }

        if (m_attenuation)
        {
            m_attenuation->GetRefCounter()->Decrement();
        }

        for (auto&& sound : _sounds)
        {
            if (auto findIt = state->sound_map.find(sound.first); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Decrement();
                continue;
            }

            if (auto findIt = state->collection_map.find(sound.first); findIt != state->collection_map.end())
            {
                findIt->second->GetRefCounter()->Decrement();
                continue;
            }
        }
    }

    const SwitchContainerDefinition* SwitchContainer::GetSwitchContainerDefinition() const
    {
        return Amplitude::GetSwitchContainerDefinition(_source.c_str());
    }

    FaderInstance* SwitchContainer::GetFaderIn(AmObjectID id) const
    {
        if (_fadersIn.find(id) != _fadersIn.end())
            return std::get<1>(_fadersIn.at(id));

        return nullptr;
    }

    FaderInstance* SwitchContainer::GetFaderOut(AmObjectID id) const
    {
        if (_fadersOut.find(id) != _fadersOut.end())
            return std::get<1>(_fadersOut.at(id));

        return nullptr;
    }

    const std::vector<SwitchContainerItem>& SwitchContainer::GetSoundObjects(AmObjectID stateId) const
    {
        return _sounds.at(stateId);
    }

    const Switch* SwitchContainer::GetSwitch() const
    {
        return _switch;
    }
} // namespace SparkyStudios::Audio::Amplitude
