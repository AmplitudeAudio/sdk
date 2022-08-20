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

#include <SparkyStudios/Audio/Amplitude/Core/Environment.h>

#include <Core/EngineInternalState.h>
#include <Core/EnvironmentInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    Environment::Environment()
        : _state(nullptr)
    {}

    Environment::Environment(EnvironmentInternalState* state)
        : _state(state)
    {}

    void Environment::Clear()
    {
        _state = nullptr;
    }

    bool Environment::Valid() const
    {
        return _state != nullptr && _state->GetId() != kAmInvalidObjectId && _state->node.in_list();
    }

    AmEnvironmentID Environment::GetId() const
    {
        return _state != nullptr ? _state->GetId() : kAmInvalidObjectId;
    }

    void Environment::SetLocation(const hmm_vec3& location) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetLocation(location);
    }

    const hmm_vec3& Environment::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetLocation();
    }

    void Environment::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->SetOrientation(direction, up);
    }

    const hmm_vec3& Environment::GetDirection() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetDirection();
    }

    const hmm_vec3& Environment::GetUp() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetUp();
    }

    AmReal32 Environment::GetFactor(const hmm_vec3& location) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetFactor(location);
    }

    AmReal32 Environment::GetFactor(const Entity& entity) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetFactor(entity);
    }

    void Environment::SetEffect(AmEffectID effect) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetEffect(effect);
    }

    void Environment::SetEffect(const std::string& effect) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetEffect(effect);
    }

    void Environment::SetEffect(const Effect* effect) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetEffect(effect);
    }

    const Effect* Environment::GetEffect() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetEffect();
    }

    void Environment::SetZone(Zone* zone) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetZone(zone);
    }

    Zone* Environment::GetZone() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetZone();
    }

    EnvironmentInternalState* Environment::GetState() const
    {
        return _state;
    }

    void Environment::Update() const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->Update();
    }
} // namespace SparkyStudios::Audio::Amplitude
