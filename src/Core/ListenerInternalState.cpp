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
        _location = location;
    }

    const hmm_vec3& ListenerInternalState::GetLocation() const
    {
        return _location;
    }

    void ListenerInternalState::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up)
    {
        _direction = AM_Normalize(direction);
        _up = AM_Normalize(up);
    }

    hmm_mat4& ListenerInternalState::GetInverseMatrix()
    {
        return _inverseMatrix;
    }

    const hmm_mat4& ListenerInternalState::GetInverseMatrix() const
    {
        return _inverseMatrix;
    }

    void ListenerInternalState::Update()
    {
        hmm_vec3 eye = AM_Vec3(-_location.X, _location.Y, _location.Z);
        hmm_vec3 forward = AM_Vec3(-_direction.X, _direction.Y, _direction.Z);

        _inverseMatrix = AM_LookAt(eye, eye + forward, _up);
    }
}
