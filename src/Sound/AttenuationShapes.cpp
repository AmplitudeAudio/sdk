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

#include <Sound/AttenuationShapes.h>

namespace SparkyStudios::Audio::Amplitude
{
    AttenuationShape::AttenuationShape()
        : m_maxAttenuationFactor(1.0f)
    {}

    float AttenuationShape::GetAttenuationFactor(const hmm_vec3& soundLocation, const ListenerInternalState* listener)
    {
        return m_maxAttenuationFactor;
    }

    float AttenuationShape::GetAttenuationFactor(const EntityInternalState* entity, const ListenerInternalState* listener)
    {
        return m_maxAttenuationFactor;
    }

    AttenuationShape* AttenuationShape::Create(const AttenuationShapeDefinition* definition)
    {
        if (definition == nullptr)
            return nullptr;

        AttenuationShape* shape = nullptr;

        switch (definition->model())
        {
        default:
        case AttenuationShapeModel_Default:
            shape = new AttenuationShape();
            break;
        case AttenuationShapeModel_Cone:
            shape = new ConeAttenuationShape(definition->settings_as_Cone());
            break;
        case AttenuationShapeModel_Sphere:
            shape = new SphereAttenuationShape(definition->settings_as_Sphere());
            break;
        case AttenuationShapeModel_Box:
            shape = new BoxAttenuationShape(definition->settings_as_Box());
            break;
        case AttenuationShapeModel_Capsule:
            shape = new CapsuleAttenuationShape(definition->settings_as_Capsule());
            break;
        }

        shape->m_maxAttenuationFactor = definition->max_attenuation_factor();

        return shape;
    }

    float ConeAttenuationShape::GetAttenuationFactor(const hmm_vec3& soundLocation, const ListenerInternalState* listener)
    {
        // Cone attenuation works only with PositionOrientation spatial sounds
        return 1.0f;
    }

    float ConeAttenuationShape::GetAttenuationFactor(const EntityInternalState* entity, const ListenerInternalState* listener)
    {
        const hmm_vec3& soundToListener = listener->GetLocation() - entity->GetLocation();
        float cone_dist = AM_Dot(soundToListener, entity->GetDirection());

        if (cone_dist < 0.0f || cone_dist > _outerHeight)
            return m_maxAttenuationFactor;

        if (cone_dist > _innerHeight)
            return AM_Lerp(1.0f, (cone_dist - _innerHeight) / (_outerHeight - _innerHeight), m_maxAttenuationFactor);

        float innerConeRadius = (cone_dist / _innerHeight) * _innerRadius;
        float outerConeRadius = (cone_dist / _outerHeight) * _outerRadius;

        float distance = AM_Length(soundToListener - cone_dist * entity->GetDirection());

        if (distance <= innerConeRadius)
            return 1.0f;
        if (distance >= outerConeRadius)
            return m_maxAttenuationFactor;

        return AM_Lerp(1.0f, (distance - innerConeRadius) / (outerConeRadius - innerConeRadius), m_maxAttenuationFactor);
    }

    float ConeAttenuationShape::GetOuterRadius() const
    {
        return _outerRadius;
    }

    float ConeAttenuationShape::GetInnerRadius() const
    {
        return _innerRadius;
    }

    float ConeAttenuationShape::GetInnerHeight() const
    {
        return _innerHeight;
    }

    float ConeAttenuationShape::GetOuterHeight() const
    {
        return _outerHeight;
    }

    float SphereAttenuationShape::GetAttenuationFactor(const hmm_vec3& soundLocation, const ListenerInternalState* listener)
    {
        const hmm_vec3& soundToListener = listener->GetLocation() - soundLocation;
        float delta = (AM_Length(soundToListener) - _innerRadius) / (_outerRadius - _innerRadius);

        return AM_Lerp(1.0f, AM_CLAMP(delta, 0.0f, 1.0f), m_maxAttenuationFactor);
    }

    float SphereAttenuationShape::GetAttenuationFactor(const EntityInternalState* entity, const ListenerInternalState* listener)
    {
        return GetAttenuationFactor(entity->GetLocation(), listener);
    }
} // namespace SparkyStudios::Audio::Amplitude
