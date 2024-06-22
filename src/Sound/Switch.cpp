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

#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>

#include <Core/EngineInternalState.h>

#include "switch_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool SwitchState::Valid() const
    {
        return m_id != kAmInvalidObjectId && !m_name.empty();
    }

    bool SwitchState::operator==(const SwitchState& other) const
    {
        return m_id == other.m_id && m_name == other.m_name;
    }

    bool SwitchState::operator!=(const SwitchState& other) const
    {
        return !(*this == other);
    }

    Switch::Switch()
        : _activeState()
        , _states()
        , _refCounter()
    {}

    Switch::~Switch()
    {
        _states.clear();
    }

    const SwitchState& Switch::GetState() const
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);
        return _activeState;
    }

    void Switch::SetState(const SwitchState& state)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        if (!state.Valid())
        {
            amLogError("Invalid switch state provided.");
            return;
        }

        if (const auto findIt = std::ranges::find(_states, state); findIt != _states.end())
        {
            _activeState = state;
        }
    }

    void Switch::SetState(AmObjectID id)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);
        if (const auto findIt = std::ranges::find_if(
                _states,
                [id](const SwitchState& state)
                {
                    return state.m_id == id;
                });
            findIt != _states.end())
        {
            _activeState = *findIt;
        }
    }

    void Switch::SetState(const std::string& name)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);
        if (const auto findIt = std::ranges::find_if(
                _states,
                [name](const SwitchState& state)
                {
                    return state.m_name == name;
                });
            findIt != _states.end())
        {
            _activeState = *findIt;
        }
    }

    const std::vector<SwitchState>& Switch::GetSwitchStates() const
    {
        return _states;
    }

    bool Switch::LoadDefinition(const SwitchDefinition* definition, EngineInternalState* state)
    {
        _id = definition->id();
        _name = definition->name()->str();

        const flatbuffers::uoffset_t size = definition->states() ? definition->states()->size() : 0;
        _states.resize(size);

        for (flatbuffers::uoffset_t i = 0; i < size; ++i)
        {
            _states[i].m_id = definition->states()->Get(i)->id();
            _states[i].m_name = definition->states()->Get(i)->name()->str();
        }

        return true;
    }

    const SwitchDefinition* Switch::GetDefinition() const
    {
        return GetSwitchDefinition(_source.c_str());
    }
} // namespace SparkyStudios::Audio::Amplitude
