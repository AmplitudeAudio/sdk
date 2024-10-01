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

#include <Sound/Attenuation.h>

namespace SparkyStudios::Audio::Amplitude
{
    AttenuationImpl::AttenuationImpl()
        : _maxDistance(0.0f)
        , _shape(nullptr)
        , _gainCurve()
        , _airAbsorptionEnabled(true)
        , _airAbsorptionCoefficients{ 0.0002f, 0.0017f, 0.0182f }
    {}

    AttenuationImpl::~AttenuationImpl()
    {
        _maxDistance = 0.0f;

        _shape = nullptr;
    }

    AmReal32 AttenuationImpl::GetGain(const AmVec3& soundLocation, const Listener& listener) const
    {
        return _shape->GetAttenuationFactor(this, soundLocation, listener);
    }

    AmReal32 AttenuationImpl::GetGain(const Entity& entity, const Listener& listener) const
    {
        return _shape->GetAttenuationFactor(this, entity, listener);
    }

    AttenuationZone* AttenuationImpl::GetShape() const
    {
        return _shape.get();
    }

    AmReal32 AttenuationImpl::EvaluateAirAbsorption(const AmVec3& soundLocation, const AmVec3& listenerLocation, AmUInt32 band) const
    {
        const AmReal32 distance = AM_Len(soundLocation - listenerLocation);
        return std::exp(-_airAbsorptionCoefficients[band] * distance);
    }

    bool AttenuationImpl::LoadDefinition(const AttenuationDefinition* definition, EngineInternalState* state)
    {
        m_id = definition->id();
        m_name = definition->name()->str();

        _maxDistance = definition->max_distance();

        _gainCurve.Initialize(definition->gain_curve());

        _shape.reset(AttenuationZoneImpl::Create(definition->shape()));

        if (const auto* model = definition->air_absorption(); model != nullptr)
        {
            _airAbsorptionEnabled = model->enabled();

            if (_airAbsorptionEnabled)
            {
                _airAbsorptionCoefficients[0] = model->coefficients()->Get(0);
                _airAbsorptionCoefficients[1] = model->coefficients()->Get(1);
                _airAbsorptionCoefficients[2] = model->coefficients()->Get(2);
            }
        }

        return true;
    }

    const AttenuationDefinition* AttenuationImpl::GetDefinition() const
    {
        return GetAttenuationDefinition(m_source.c_str());
    }
} // namespace SparkyStudios::Audio::Amplitude