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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EnvironmentInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    EnvironmentInternalState::EnvironmentInternalState()
        : _id(kAmInvalidObjectId)
        , _zone(nullptr)
        , _effect(nullptr)
    {}

    AmEnvironmentID EnvironmentInternalState::GetId() const
    {
        return _id;
    }

    void EnvironmentInternalState::SetId(AmEnvironmentID id)
    {
        _id = id;
    }

    void EnvironmentInternalState::SetLocation(const AmVec3& location)
    {
        _zone->SetLocation(location);
    }

    const AmVec3& EnvironmentInternalState::GetLocation() const
    {
        return _zone->GetLocation();
    }

    void EnvironmentInternalState::SetOrientation(const AmVec3& direction, const AmVec3& up)
    {
        _zone->SetOrientation(direction, up);
    }

    const AmVec3& EnvironmentInternalState::GetDirection() const
    {
        return _zone->GetDirection();
    }

    const AmVec3& EnvironmentInternalState::GetUp() const
    {
        return _zone->GetUp();
    }

    AmReal32 EnvironmentInternalState::GetFactor(const AmVec3& location) const
    {
        if (amEngine->IsGameTrackingEnvironmentAmounts())
            return 0.0f;

        return _zone->GetFactor(location);
    }

    AmReal32 EnvironmentInternalState::GetFactor(const Entity& entity) const
    {
        return GetFactor(entity.GetLocation());
    }

    void EnvironmentInternalState::SetEffect(AmEffectID effect)
    {
        _effect = amEngine->GetEffectHandle(effect);
    }

    void EnvironmentInternalState::SetEffect(const std::string& effect)
    {
        _effect = amEngine->GetEffectHandle(effect);
    }

    void EnvironmentInternalState::SetEffect(const Effect* effect)
    {
        _effect = effect;
    }

    const Effect* EnvironmentInternalState::GetEffect() const
    {
        return _effect;
    }

    const Zone* EnvironmentInternalState::GetZone() const
    {
        return _zone;
    }

    void EnvironmentInternalState::SetZone(Zone* zone)
    {
        _zone = zone;
        Update();
    }

    Zone* EnvironmentInternalState::GetZone()
    {
        return _zone;
    }

    void EnvironmentInternalState::Update()
    {}
} // namespace SparkyStudios::Audio::Amplitude
