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

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>
#include <SparkyStudios/Audio/Amplitude/Core/Log.h>

#include <Core/BusInternalState.h>

#include "buses_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool DuckBusInternalState::Initialize(const DuckBusDefinition* definition)
    {
        if (definition == nullptr)
            return false;

        if (definition->id() == kAmInvalidObjectId)
        {
            CallLogFunc("[ERROR] Cannot initialize duck-bus internal state: the duck-bus ID is invalid.");
            return false;
        }

        Engine* engine = Engine::GetInstance();

        _bus = engine->FindBus(definition->id());
        if (!_bus.Valid())
        {
            CallLogFunc("[ERROR] Cannot initialize duck-bus internal state: unable to find a duck-bus with ID %u.", definition->id());
            return false;
        }

        _targetGain = definition->target_gain();
        _fadeInDuration = definition->fade_in()->duration();
        _fadeOutDuration = definition->fade_out()->duration();

        _faderIn = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fade_in()->fader()));
        _faderIn->Set(_bus.GetGain(), _targetGain, _fadeInDuration);

        _faderOut = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(definition->fade_out()->fader()));
        _faderOut->Set(_targetGain, _bus.GetGain(), _fadeOutDuration);

        _initialized = true;
        return true;
    }

    void DuckBusInternalState::Update(AmTime deltaTime)
    {
        if (!_initialized)
            return; // Don't waste time with an uninitialized state.

        bool playing = !_parent->_playingSoundList.empty();
        float duckGain = _bus.GetState()->_duckGain;

        if (playing && _transitionPercentage <= 1.0)
        {
            // Fading to duck gain.
            if (_fadeInDuration > 0.0)
            {
                _transitionPercentage += deltaTime / _fadeInDuration;
                _transitionPercentage = AM_MIN(_transitionPercentage, 1.0);
            }
            else
            {
                _transitionPercentage = 1.0;
            }

            duckGain = _faderIn->GetFromPercentage(_transitionPercentage);
        }
        else if (!playing && _transitionPercentage >= 0.0)
        {
            // Fading to standard gain.
            if (_fadeOutDuration > 0.0)
            {
                _transitionPercentage -= deltaTime / _fadeOutDuration;
                _transitionPercentage = AM_MAX(_transitionPercentage, 0.0);
            }
            else
            {
                _transitionPercentage = 0.0;
            }

            duckGain = _faderOut->GetFromPercentage(1.0 - _transitionPercentage);
        }

        _bus.GetState()->_duckGain = AM_MIN(duckGain, _bus.GetState()->_duckGain);
    }

    void BusInternalState::SetMute(bool mute)
    {
        _muted = mute;
    }

    bool BusInternalState::IsMute() const
    {
        return _muted;
    }

    void BusInternalState::FadeTo(float gain, AmTime duration)
    {
        // Setup fader
        _targetUserGain = gain;
        _gainFader->Set(_userGain, _targetUserGain, duration);

        // Set now as the stat time of the transition
        _gainFader->Start(Engine::GetInstance()->GetTotalTime());
    }

    void BusInternalState::Initialize(const BusDefinition* bus_def)
    {
        // Make sure we only initialize once.
        AMPLITUDE_ASSERT(_busDefinition == nullptr);
        _busDefinition = bus_def;

        // Initialize the ID with the value specified by the definition file.
        _id = _busDefinition->id();
        // Initialize the name with the value specified by the definition file.
        _name = _busDefinition->name()->str();
        // Initialize the gain with the value specified by the definition file.
        _gain = _busDefinition->gain();

        _gainFader = Fader::Create(static_cast<Fader::FADER_ALGORITHM>(_busDefinition->fader()));

        _childBuses.clear();
        _duckBuses.clear();
    }

    void BusInternalState::UpdateDuckGain(AmTime delta_time)
    {
        for (auto&& bus : _duckBuses)
        {
            bus->Update(delta_time);
        }
    }

    void BusInternalState::AdvanceFrame(AmTime delta_time, float parent_gain)
    {
        if (_gainFader->GetState() == AM_FADER_STATE_ACTIVE)
        {
            // Update fading.
            _userGain = _gainFader->GetFromTime(Engine::GetInstance()->GetTotalTime());

            if (_userGain == _targetUserGain)
            {
                // Fading is ended, disable fader.
                _gainFader->SetState(AM_FADER_STATE_STOPPED);
            }
        }

        // Update final gain.
        _gain = _busDefinition->gain() * parent_gain * _duckGain * _userGain;

        // Advance frames in playing channels.
        for (auto&& channel : _playingSoundList)
        {
            channel.AdvanceFrame(delta_time);
        }

        // Advance frames in child buses.
        for (auto&& child_bus : _childBuses)
        {
            if (child_bus)
            {
                child_bus->AdvanceFrame(delta_time, _gain);
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
