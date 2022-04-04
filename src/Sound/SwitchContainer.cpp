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
        : _bus(nullptr)
        , _switch(nullptr)
        , _source()
        , _sounds()
        , _id(kAmInvalidObjectId)
        , _name()
        , _effect(nullptr)
        , _attenuation(nullptr)
        , _refCounter()
    {}

    SwitchContainer::~SwitchContainer()
    {
        _switch = nullptr;
        _sounds.clear();
        _effect = nullptr;
        _attenuation = nullptr;
    }

    bool SwitchContainer::LoadSwitchContainerDefinition(const std::string& source, EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(_id == kAmInvalidObjectId);

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

        _bus = FindBusInternalState(state, definition->bus());
        if (!_bus)
        {
            CallLogFunc("[ERROR] SwitchContainer %s specifies an unknown bus ID: %u.\n", definition->name(), definition->bus());
            return false;
        }

        if (auto findIt = state->switch_map.find(definition->switch_group()); findIt != state->switch_map.end())
        {
            _switch = findIt->second.get();
        }

        if (definition->effect() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->effect_map.find(definition->effect()); findIt != state->effect_map.end())
            {
                _effect = findIt->second.get();
            }
            else
            {
                CallLogFunc("[ERROR] Sound definition is invalid: invalid effect ID \"%u\"", definition->effect());
                return false;
            }
        }

        if (definition->attenuation() != kAmInvalidObjectId)
        {
            if (auto findIt = state->attenuation_map.find(definition->attenuation()); findIt != state->attenuation_map.end())
            {
                _attenuation = findIt->second.get();
            }

            if (!_attenuation)
            {
                CallLogFunc(
                    "[ERROR] SwitchContainer " AM_OS_CHAR_FMT " specifies an unknown attenuation ID: %u.\n",
                    AM_STRING_TO_OS_STRING(definition->name()->c_str()), definition->attenuation());
                return false;
            }
        }

        _id = definition->id();
        _name = definition->name()->str();

        _gain = RtpcValue(definition->gain());
        _priority = RtpcValue(definition->priority());

        const auto& states = _switch->GetSwitchStates();
        for (const auto& state : states)
        {
            if (_sounds.count(state.m_id) == 0)
            {
                _sounds.insert({ state.m_id, std::vector<SwitchContainerItem>() });
            }
        }

        flatbuffers::uoffset_t count = definition->entries() ? definition->entries()->size() : 0;
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
            _fadersIn[id] = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(entry->fade_in()->fader()));
            _fadersOut[id] = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(entry->fade_out()->fader()));

            _fadersIn[id]->SetDuration(entry->fade_in()->duration());
            _fadersOut[id]->SetDuration(entry->fade_out()->duration());

            SwitchContainerItem item;
            item.m_id = id;
            item.m_continueBetweenStates = entry->continue_between_states();
            item.m_fadeInDuration = entry->fade_in()->duration();
            item.m_fadeOutDuration = entry->fade_out()->duration();
            item.m_fadeInAlgorithm = entry->fade_in()->fader();
            item.m_fadeOutAlgorithm = entry->fade_out()->fader();
            item.m_gain = RtpcValue(entry->gain());

            flatbuffers::uoffset_t statesCount = entry->switch_states()->size();
            for (flatbuffers::uoffset_t j = 0; j < statesCount; ++j)
            {
                AmObjectID stateId = entry->switch_states()->Get(j);
                _sounds[stateId].push_back(item);
            }
        }

        return true;
    }

    bool SwitchContainer::LoadSwitchContainerDefinitionFromFile(AmOsString filename, EngineInternalState* state)
    {
        std::string source;
        return Amplitude::LoadFile(filename, &source) && LoadSwitchContainerDefinition(source, state);
    }

    void SwitchContainer::AcquireReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        _switch->GetRefCounter()->Increment();

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
            if (auto findIt = state->sound_map.find(sound.first); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Increment();
            }
            else if (auto findIt = state->collection_map.find(sound.first); findIt != state->collection_map.end())
            {
                findIt->second->GetRefCounter()->Increment();
            }
        }
    }

    void SwitchContainer::ReleaseReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        _switch->GetRefCounter()->Decrement();

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
            if (auto findIt = state->sound_map.find(sound.first); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Decrement();
            }
            else if (auto findIt = state->collection_map.find(sound.first); findIt != state->collection_map.end())
            {
                findIt->second->GetRefCounter()->Decrement();
            }
        }
    }

    const SwitchContainerDefinition* SwitchContainer::GetSwitchContainerDefinition() const
    {
        return Amplitude::GetSwitchContainerDefinition(_source.c_str());
    }

    const RtpcValue& SwitchContainer::GetGain() const
    {
        return _gain;
    }

    const RtpcValue& SwitchContainer::GetPriority() const
    {
        return _priority;
    }

    AmCollectionID SwitchContainer::GetId() const
    {
        return _id;
    }

    const std::string& SwitchContainer::GetName() const
    {
        return _name;
    }

    BusInternalState* SwitchContainer::GetBus() const
    {
        return _bus;
    }

    Fader* SwitchContainer::GetFaderIn(AmObjectID id) const
    {
        if (_fadersIn.find(id) != _fadersIn.end())
        {
            return _fadersIn.at(id);
        }

        return nullptr;
    }

    Fader* SwitchContainer::GetFaderOut(AmObjectID id) const
    {
        if (_fadersOut.find(id) != _fadersOut.end())
        {
            return _fadersOut.at(id);
        }

        return nullptr;
    }

    RefCounter* SwitchContainer::GetRefCounter()
    {
        return &_refCounter;
    }

    const std::vector<SwitchContainerItem>& SwitchContainer::GetSoundObjects(AmObjectID stateId) const
    {
        return _sounds.at(stateId);
    }

    const Effect* SwitchContainer::GetEffect() const
    {
        return _effect;
    }

    const Attenuation* SwitchContainer::GetAttenuation() const
    {
        return _attenuation;
    }

    const Switch* SwitchContainer::GetSwitch() const
    {
        return _switch;
    }
} // namespace SparkyStudios::Audio::Amplitude
