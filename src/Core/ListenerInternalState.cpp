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
        : _inverseMatrix(AM_Mat4d(1))
        , _location()
        , _direction()
        , _up()
        , _id(kAmInvalidObjectId)
        , _lastLocation()
        , _velocity()
    {}

    AmListenerID ListenerInternalState::GetId() const
    {
        return _id;
    }

    void ListenerInternalState::SetId(AmListenerID id)
    {
        _id = id;
    }

    void ListenerInternalState::SetLocation(const hmm_vec3& location)
    {
        _lastLocation = _location;
        _location = location;
    }

    const hmm_vec3& ListenerInternalState::GetLocation() const
    {
        return _location;
    }

    void ListenerInternalState::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up)
    {
        _direction = direction;
        _up = up;
    }

    const hmm_vec3& ListenerInternalState::GetDirection() const
    {
        return _direction;
    }

    const hmm_vec3& ListenerInternalState::GetUp() const
    {
        return _up;
    }

    hmm_mat4& ListenerInternalState::GetInverseMatrix()
    {
        return _inverseMatrix;
    }

    const hmm_mat4& ListenerInternalState::GetInverseMatrix() const
    {
        return _inverseMatrix;
    }

    const hmm_vec3& ListenerInternalState::GetVelocity() const
    {
        return _velocity;
    }

    void ListenerInternalState::Update()
    {
        _velocity = _location - _lastLocation;
        _inverseMatrix = AM_LookAt(_location, _location + _direction, _up);
    }
} // namespace SparkyStudios::Audio::Amplitude
