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
        , _direction()
        , _up()
        , _lastLocation()
        , _velocity()
        , _inverseMatrix(AM_M4D(1))
    {}

    AmListenerID ListenerInternalState::GetId() const
    {
        return _id;
    }

    void ListenerInternalState::SetId(AmListenerID id)
    {
        _id = id;
    }

    void ListenerInternalState::SetLocation(const AmVec3& location)
    {
        _lastLocation = _location;
        _location = location;
    }

    const AmVec3& ListenerInternalState::GetLocation() const
    {
        return _location;
    }

    void ListenerInternalState::SetOrientation(const AmVec3& direction, const AmVec3& up)
    {
        _direction = direction;
        _up = up;
    }

    const AmVec3& ListenerInternalState::GetDirection() const
    {
        return _direction;
    }

    const AmVec3& ListenerInternalState::GetUp() const
    {
        return _up;
    }

    AmMat4& ListenerInternalState::GetInverseMatrix()
    {
        return _inverseMatrix;
    }

    const AmMat4& ListenerInternalState::GetInverseMatrix() const
    {
        return _inverseMatrix;
    }

    const AmVec3& ListenerInternalState::GetVelocity() const
    {
        return _velocity;
    }

    void ListenerInternalState::Update()
    {
        _velocity = _location - _lastLocation;
        _inverseMatrix = AM_LookAt_RH(_location, _location + _direction, _up);
    }
} // namespace SparkyStudios::Audio::Amplitude
