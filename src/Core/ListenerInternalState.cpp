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

#include <Core/ListenerInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    ListenerInternalState::ListenerInternalState()
        : _id(kAmInvalidObjectId)
        , _location()
        , _orientation(Orientation::Zero())
        , _lastLocation()
        , _velocity()
        , _directivity(0.0f)
        , _directivitySharpness(1.0f)
        , _inverseMatrix(AM_M4D(1.0f))
        , _playingSoundList(&ChannelInternalState::listener_node)
    {}

    void ListenerInternalState::SetLocation(const AmVec3& location)
    {
        _lastLocation = _location;
        _location = location;
    }

    void ListenerInternalState::SetDirectivity(AmReal32 directivity, AmReal32 sharpness)
    {
        _directivity = directivity;
        _directivitySharpness = sharpness;
    }

    void ListenerInternalState::Update()
    {
        _velocity = _location - _lastLocation;
        _inverseMatrix = _orientation.GetLookAtMatrix(_location);
    }
} // namespace SparkyStudios::Audio::Amplitude
