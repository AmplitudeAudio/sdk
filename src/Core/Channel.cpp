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

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Channel.h>

#include <Core/ChannelInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    static AmUInt64 globalStateId = 0;
    static AmVec3 globalPosition = { 0.0f, 0.0f, 0.0f };

    Channel::Channel()
        : _state(nullptr)
        , _stateId(0)
    {}

    Channel::Channel(ChannelInternalState* state)
        : Channel(state, ++globalStateId)
    {}

    Channel::Channel(const Channel& other) = default;

    void Channel::Clear()
    {
        _state = nullptr;
        _stateId = 0;
    }

    bool Channel::Valid() const
    {
        return _state != nullptr && _stateId != 0;
    }

    bool Channel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            return _state->Playing();

        return false;
    }

    void Channel::Stop(AmTime duration) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (!IsValidStateId())
            return;

        if (_state->Stopped())
            return;

        if (duration == 0.0)
            _state->Halt();
        else
            _state->FadeOut(duration, ChannelPlaybackState::Stopped);
    }

    void Channel::Pause(AmTime duration) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (!IsValidStateId())
            return;

        if (_state->Paused())
            return;

        if (duration == 0.0)
            _state->Pause();
        else
            _state->FadeOut(duration, ChannelPlaybackState::Paused);
    }

    void Channel::Resume(AmTime duration) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (!IsValidStateId())
            return;

        if (_state->Playing())
            return;

        if (duration == 0.0)
            _state->Resume();
        else
            _state->FadeIn(duration);
    }

    const AmVec3& Channel::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            return _state->GetLocation();

        return globalPosition;
    }

    void Channel::SetLocation(const AmVec3& location) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            _state->SetLocation(location);
    }

    void Channel::SetGain(const AmReal32 gain) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            _state->SetUserGain(gain);
    }

    AmReal32 Channel::GetGain() const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            return _state->GetUserGain();

        return 0.0f;
    }

    ChannelPlaybackState Channel::GetPlaybackState() const
    {
        return _state->GetChannelState();
    }

    ChannelInternalState* Channel::GetState() const
    {
        return _state;
    }

    Channel::Channel(ChannelInternalState* state, const AmUInt64 id)
        : _state(state)
        , _stateId(id)
    {
        if (_state != nullptr)
            _state->SetChannelStateId(_stateId);
    }

    bool Channel::IsValidStateId() const
    {
        return _state->GetChannelStateId() == _stateId;
    }
} // namespace SparkyStudios::Audio::Amplitude
