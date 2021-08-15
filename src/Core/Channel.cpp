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
    const int kFadeOutDurationMs = 10;

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

    void Channel::Stop()
    {
        AMPLITUDE_ASSERT(Valid());
        // Fade out rather than halting to avoid clicks.  However, SDL_Mixer will
        // not fade out channels with a volume of 0.  Manually halt channels in this
        // case.
        if (!_state->IsReal() || _state->GetRealChannel().GetGain() == 0.0f)
        {
            _state->Halt();
        }
        else
        {
            _state->FadeOut(kFadeOutDurationMs);
        }
    }

    void Channel::Pause()
    {
        AMPLITUDE_ASSERT(Valid());
        _state->Pause();
    }

    void Channel::Resume()
    {
        AMPLITUDE_ASSERT(Valid());
        _state->Resume();
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

    void Channel::SetGain(float gain)
    {
        return _state->SetUserGain(gain);
    }

    float Channel::GetGain() const
    {
        return _state->GetUserGain();
    }
} // namespace SparkyStudios::Audio::Amplitude
