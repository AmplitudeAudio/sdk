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

#include <Core/EntityInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    EntityInternalState::EntityInternalState()
        : _playingSoundList(&ChannelInternalState::entity_node)
        , _id(kAmInvalidObjectId)
        , _location()
        , _direction()
        , _up()
        , _inverseMatrix(AM_Mat4d(1.0f))
    {}

    AmEntityID EntityInternalState::GetId() const
    {
        return _id;
    }

    void EntityInternalState::SetId(AmEntityID id)
    {
        _id = id;
    }

    void EntityInternalState::SetLocation(const hmm_vec3& location)
    {
        _location = location;
    }

    const hmm_vec3& EntityInternalState::GetLocation() const
    {
        return _location;
    }

    void EntityInternalState::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up)
    {
        _direction = AM_Normalize(direction);
        _up = AM_Normalize(up);
    }

    const hmm_vec3& EntityInternalState::GetDirection() const
    {
        return _direction;
    }

    const hmm_vec3& EntityInternalState::GetUp() const
    {
        return _up;
    }

    const hmm_mat4& EntityInternalState::GetInverseMatrix() const
    {
        return _inverseMatrix;
    }

    void EntityInternalState::Update()
    {
        hmm_vec3 eye = AM_Vec3(-_location.X, _location.Y, _location.Z);
        hmm_vec3 forward = AM_Vec3(-_direction.X, _direction.Y, _direction.Z);

        _inverseMatrix = AM_LookAt(eye, eye + forward, _up);
    }
} // namespace SparkyStudios::Audio::Amplitude
