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

#include <SparkyStudios/Audio/Amplitude/Core/Bus.h>

#include <Core/BusInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    void Bus::Clear()
    {
        _state = nullptr;
    }

    bool Bus::Valid() const
    {
        return _state != nullptr;
    }

    void Bus::SetGain(float gain)
    {
        return _state->SetUserGain(gain);
    }

    float Bus::GetGain() const
    {
        return _state->GetUserGain();
    }

    void Bus::FadeTo(float gain, AmTime duration)
    {
        _state->FadeTo(gain, duration);
    }

    float Bus::GetFinalGain() const
    {
        return _state->GetGain();
    }

    void Bus::SetMute(bool mute)
    {
        _state->SetMute(mute);
    }

    bool Bus::IsMute() const
    {
        return _state->IsMute();
    }

    BusInternalState* Bus::GetState() const
    {
        return _state;
    }
} // namespace SparkyStudios::Audio::Amplitude
