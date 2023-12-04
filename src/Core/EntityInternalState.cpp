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
        : _id(kAmInvalidObjectId)
        , _lastLocation()
        , _velocity()
        , _location()
        , _direction()
        , _up()
        , _inverseMatrix(AM_M4D(1.0f))
        , _obstruction(0.0f)
        , _occlusion(0.0f)
        , _environmentFactors()
        , _playingSoundList(&ChannelInternalState::entity_node)
    {}

    AmEntityID EntityInternalState::GetId() const
    {
        return _id;
    }

    void EntityInternalState::SetId(AmEntityID id)
    {
        _id = id;
    }

    const AmVec3& EntityInternalState::GetVelocity() const
    {
        return _velocity;
    }

    void EntityInternalState::SetLocation(const AmVec3& location)
    {
        _lastLocation = _location;
        _location = location;
    }

    const AmVec3& EntityInternalState::GetLocation() const
    {
        return _location;
    }

    void EntityInternalState::SetOrientation(const AmVec3& direction, const AmVec3& up)
    {
        _direction = direction;
        _up = up;
    }

    const AmVec3& EntityInternalState::GetDirection() const
    {
        return _direction;
    }

    const AmVec3& EntityInternalState::GetUp() const
    {
        return _up;
    }

    const AmMat4& EntityInternalState::GetInverseMatrix() const
    {
        return _inverseMatrix;
    }

    void EntityInternalState::Update()
    {
        _velocity = _location - _lastLocation;
        _inverseMatrix = AM_LookAt_RH(_location, _location + _direction, _up);
    }

    void EntityInternalState::SetObstruction(AmReal32 obstruction)
    {
        _obstruction = obstruction;
        for (auto&& sound : _playingSoundList)
        {
            sound.SetObstruction(obstruction);
        }
    }

    void EntityInternalState::SetOcclusion(AmReal32 occlusion)
    {
        _occlusion = occlusion;
        for (auto&& sound : _playingSoundList)
        {
            sound.SetOcclusion(occlusion);
        }
    }

    AmReal32 EntityInternalState::GetObstruction() const
    {
        return _obstruction;
    }

    AmReal32 EntityInternalState::GetOcclusion() const
    {
        return _occlusion;
    }

    void EntityInternalState::SetEnvironmentFactor(AmEnvironmentID environment, AmReal32 factor)
    {
        _environmentFactors[environment] = factor;
    }

    AmReal32 EntityInternalState::GetEnvironmentFactor(AmEnvironmentID environment)
    {
        if (!_environmentFactors.contains(environment))
        {
            _environmentFactors[environment] = 0.0f;
        }

        return _environmentFactors[environment];
    }

    const std::map<AmEnvironmentID, AmReal32>& EntityInternalState::GetEnvironments() const
    {
        return _environmentFactors;
    }
} // namespace SparkyStudios::Audio::Amplitude
