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
        , _obstruction(0.0f)
        , _occlusion(0.0f)
        , _environmentFactors()
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
        _direction = direction;
        _up = up;
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
        _inverseMatrix = AM_LookAt(_location, _location + _direction, _up);
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
        if (auto findIt = _environmentFactors.find(environment); findIt == _environmentFactors.end())
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
