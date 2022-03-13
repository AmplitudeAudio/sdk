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

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

#include <Core/EngineInternalState.h>

#include "attenuation_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    Attenuation::Attenuation()
        : _gainCurve()
        , _id(kAmInvalidObjectId)
        , _name()
        , _maxDistance(0.0f)
        , _shape(nullptr)
        , _refCounter()
    {}

    bool Attenuation::LoadAttenuationDefinition(const std::string& attenuation)
    {
        // Ensure we do not load the attenuation more than once
        AMPLITUDE_ASSERT(_id == kAmInvalidObjectId);

        _source = attenuation;
        const AttenuationDefinition* definition = GetAttenuationDefinition();

        _id = definition->id();
        _name = definition->name()->str();

        _maxDistance = definition->max_distance();

        _gainCurve.Initialize(definition->gain_curve());

        _shape = AttenuationZone::Create(definition->shape());

        return true;
    }

    bool Attenuation::LoadAttenuationDefinitionFromFile(AmOsString filename)
    {
        std::string source;
        return LoadFile(filename, &source) && LoadAttenuationDefinition(source);
    }

    AmReal32 Attenuation::GetGain(const hmm_vec3& soundLocation, const Listener& listener) const
    {
        return _shape->GetAttenuationFactor(this, soundLocation, listener);
    }

    AmReal32 Attenuation::GetGain(const Entity& entity, const Listener& listener) const
    {
        return _shape->GetAttenuationFactor(this, entity, listener);
    }

    AmAttenuationID Attenuation::GetId() const
    {
        return _id;
    }

    const std::string& Attenuation::GetName() const
    {
        return _name;
    }

    const AttenuationDefinition* Attenuation::GetAttenuationDefinition() const
    {
        return Amplitude::GetAttenuationDefinition(_source.c_str());
    }

    RefCounter* Attenuation::GetRefCounter()
    {
        return &_refCounter;
    }

    AttenuationZone* Attenuation::GetShape() const
    {
        return _shape;
    }

    const Curve& Attenuation::GetGainCurve() const
    {
        return _gainCurve;
    }

    double Attenuation::GetMaxDistance() const
    {
        return _maxDistance;
    }
} // namespace SparkyStudios::Audio::Amplitude