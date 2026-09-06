// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelInstance.h>

#include <Core/Playback/ChannelInstanceInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    ChannelInstance::ChannelInstance()
        : _state(nullptr)
        , _id(kAmInvalidObjectId)
        , _generation(0)
    {}

    ChannelInstance::ChannelInstance(ChannelInstanceInternalState* state)
        : _state(state)
        , _id(state != nullptr ? state->GetId() : kAmInvalidObjectId)
        , _generation(state != nullptr ? state->GetGeneration() : 0)
    {}

    void ChannelInstance::Clear()
    {
        _state = nullptr;
        _id = kAmInvalidObjectId;
        _generation = 0;
    }

    bool ChannelInstance::Valid() const
    {
        return _state != nullptr && _id != kAmInvalidObjectId && _state->GetId() == _id && _state->GetGeneration() == _generation;
    }

    AmChannelInstanceID ChannelInstance::GetId() const
    {
        return _id;
    }

    const AmVector3& ChannelInstance::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetLocation();
    }

    void ChannelInstance::SetLocation(const AmVector3& location) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetLocation(location);
    }

    Room ChannelInstance::GetRoom() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetRoom();
    }

    void ChannelInstance::SetRoom(const Room& room) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetRoom(room);
    }

    AmReal32 ChannelInstance::GetWeight() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetWeight();
    }

    void ChannelInstance::SetWeight(AmReal32 weight) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetWeight(weight);
    }

    ChannelInstanceInternalState* ChannelInstance::GetState() const
    {
        return _state;
    }
} // namespace SparkyStudios::Audio::Amplitude
