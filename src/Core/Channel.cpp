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
    Channel::Channel()
        : _state(nullptr)
    {}

    Channel::Channel(ChannelInternalState* state)
        : _state(state)
    {}

    void Channel::Clear()
    {
        _state = nullptr;
    }

    bool Channel::Valid() const
    {
        return _state != nullptr;
    }

    bool Channel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->Playing();
    }

    void Channel::Stop(AmTime duration)
    {
        AMPLITUDE_ASSERT(Valid());
        if (!_state->IsReal() || _state->GetRealChannel().GetGain() == 0.0f)
        {
            _state->Halt();
        }
        else
        {
            _state->FadeOut(duration, ChannelState::Stopped);
        }
    }

    void Channel::Pause(AmTime duration)
    {
        AMPLITUDE_ASSERT(Valid());
        if (!_state->IsReal() || _state->GetRealChannel().GetGain() == 0.0f)
        {
            _state->Pause();
        }
        else
        {
            _state->FadeOut(duration, ChannelState::Paused);
        }
    }

    void Channel::Resume(AmTime duration)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->FadeIn(duration);
    }

    const hmm_vec3& Channel::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetLocation();
    }

    void Channel::SetLocation(const hmm_vec3& location)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetLocation(location);
    }

    void Channel::SetGain(const float gain)
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->SetUserGain(gain);
    }

    float Channel::GetGain() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetUserGain();
    }

    ChannelInternalState* Channel::GetState() const
    {
        return _state;
    }
} // namespace SparkyStudios::Audio::Amplitude
