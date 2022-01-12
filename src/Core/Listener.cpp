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

#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>

#include <Core/ListenerInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    Listener::Listener()
        : _state(nullptr)
    {}

    Listener::Listener(ListenerInternalState* state)
        : _state(state)
    {}

    void Listener::Clear()
    {
        _state = nullptr;
    }

    bool Listener::Valid() const
    {
        return _state != nullptr && _state->GetId() != kAmInvalidObjectId && _state->node.in_list();
    }

    AmListenerID Listener::GetId() const
    {
        return _state->GetId();
    }

    void Listener::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetOrientation(direction, up);
    }

    hmm_vec3 Listener::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetLocation();
    }

    void Listener::SetLocation(const hmm_vec3& location)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetLocation(location);
    }
} // namespace SparkyStudios::Audio::Amplitude
