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

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>

#include <Core/EntityInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    Entity::Entity()
        : _state(nullptr)
    {}

    Entity::Entity(EntityInternalState* state)
        : _state(state)
    {}

    void Entity::Clear()
    {
        _state = nullptr;
    }

    bool Entity::Valid() const
    {
        return _state != nullptr && _state->GetId() != kAmInvalidObjectId && _state->node.in_list();
    }

    AmEntityID Entity::GetId() const
    {
        return _state != nullptr ? _state->GetId() : kAmInvalidObjectId;
    }

    const hmm_vec3& Entity::GetVelocity() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetVelocity();
    }

    void Entity::SetLocation(const hmm_vec3& location)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetLocation(location);
    }

    const hmm_vec3& Entity::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetLocation();
    }

    void Entity::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetOrientation(direction, up);
    }

    const hmm_vec3& Entity::GetDirection() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetDirection();
    }

    const hmm_vec3& Entity::GetUp() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetUp();
    }

    void Entity::Update()
    {
        AMPLITUDE_ASSERT(Valid());
        _state->Update();
    }

    void Entity::SetObstruction(AmReal32 obstruction)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetObstruction(obstruction);
    }

    void Entity::SetOcclusion(AmReal32 occlusion)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetOcclusion(occlusion);
    }

    AmReal32 Entity::GetObstruction() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetObstruction();
    }

    AmReal32 Entity::GetOcclusion() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetOcclusion();
    }

    void Entity::SetEnvironmentFactor(AmEnvironmentID environment, AmReal32 factor)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetEnvironmentFactor(environment, factor);
    }

    AmReal32 Entity::GetEnvironmentFactor(AmEnvironmentID environment) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetEnvironmentFactor(environment);
    }

    const std::map<AmEnvironmentID, AmReal32>& Entity::GetEnvironments() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetEnvironments();
    }

    EntityInternalState* Entity::GetState() const
    {
        return _state;
    }
} // namespace SparkyStudios::Audio::Amplitude
