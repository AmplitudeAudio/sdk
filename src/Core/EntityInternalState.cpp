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
        , _orientation(Orientation::Zero())
        , _inverseMatrix(AM_M4D(1.0f))
        , _obstruction(0.0f)
        , _occlusion(0.0f)
        , _directivity(0.0f)
        , _directivitySharpness(1.0f)
        , _environmentFactors()
        , _playingSoundList(&ChannelInternalState::entity_node)
    {}

    void EntityInternalState::SetLocation(const AmVec3& location)
    {
        _lastLocation = _location;
        _location = location;
    }

    void EntityInternalState::SetDirectivity(AmReal32 directivity, AmReal32 directivitySharpness)
    {
        _directivity = directivity;
        _directivitySharpness = directivitySharpness;
    }

    void EntityInternalState::Update()
    {
        _velocity = _location - _lastLocation;
        _inverseMatrix = _orientation.GetLookAtMatrix(_location);
    }

    void EntityInternalState::SetObstruction(AmReal32 obstruction)
    {
        _obstruction = obstruction;
        for (auto&& sound : _playingSoundList)
            sound.SetObstruction(obstruction);
    }

    void EntityInternalState::SetOcclusion(AmReal32 occlusion)
    {
        _occlusion = occlusion;
        for (auto&& sound : _playingSoundList)
            sound.SetOcclusion(occlusion);
    }

    void EntityInternalState::SetEnvironmentFactor(AmEnvironmentID environment, AmReal32 factor)
    {
        _environmentFactors[environment] = factor;
    }

    AmReal32 EntityInternalState::GetEnvironmentFactor(AmEnvironmentID environment)
    {
        if (!_environmentFactors.contains(environment))
            _environmentFactors[environment] = 0.0f;

        return _environmentFactors[environment];
    }
} // namespace SparkyStudios::Audio::Amplitude
