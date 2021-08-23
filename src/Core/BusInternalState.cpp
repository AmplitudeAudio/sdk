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

#include <Core/BusInternalState.h>

#include "buses_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
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
        _targetUserGain = gain;
        _targetUserGainStep = (_targetUserGain - _userGain) / (float)duration;
    }

    void BusInternalState::Initialize(const BusDefinition* bus_def)
    {
        // Make sure we only initialize once.
        AMPLITUDE_ASSERT(_busDefinition == nullptr);
        _busDefinition = bus_def;

        // Initialize the gain with the value specified by the definition file.
        _gain = _busDefinition->gain();
    }

    void BusInternalState::UpdateDuckGain(AmTime delta_time)
    {
        bool playing = !_playingSoundList.empty();
        if (playing && _transitionPercentage <= 1.0f)
        {
            // Fading to duck gain.
            float fade_in_time = _busDefinition->duck_fade_in_time();
            if (fade_in_time > 0)
            {
                _transitionPercentage += (float)delta_time / fade_in_time;
                _transitionPercentage = AM_MIN(_transitionPercentage, 1.0f);
            }
            else
            {
                _transitionPercentage = 1.0f;
            }
        }
        else if (!playing && _transitionPercentage >= 0.0f)
        {
            // Fading to standard gain.
            float fade_out_time = _busDefinition->duck_fade_out_time();
            if (fade_out_time > 0)
            {
                _transitionPercentage -= (float)delta_time / fade_out_time;
                _transitionPercentage = AM_MAX(_transitionPercentage, 0.0f);
            }
            else
            {
                _transitionPercentage = 0.0f;
            }
        }
        float duck_gain = AM_Lerp(1.0f, _busDefinition->duck_gain(), _transitionPercentage);
        for (auto bus : _duckBuses)
        {
            bus->_duckGain = AM_MIN(duck_gain, bus->_duckGain);
        }
    }

    void BusInternalState::AdvanceFrame(AmTime delta_time, float parent_gain)
    {
        // Update fading.
        _userGain += (float)delta_time * _targetUserGainStep;
        bool fading_complete =
            (_targetUserGainStep < 0 && _userGain < _targetUserGain) || (_targetUserGainStep > 0 && _userGain > _targetUserGain);
        if (fading_complete)
        {
            _userGain = _targetUserGain;
            _targetUserGainStep = 0;
        }

        // Update final gain.
        _gain = _busDefinition->gain() * parent_gain * _duckGain * _userGain;

        // Advance frames in playing channels.
        for (auto& channel: _playingSoundList)
        {
            channel.AdvanceFrame(delta_time);
        }

        // Advance frames in child buses.
        for (auto& child_bus : _childBuses)
        {
            if (child_bus)
            {
                child_bus->AdvanceFrame(delta_time, _gain);
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
