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

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>
#include <SparkyStudios/Audio/Amplitude/Core/Log.h>

#include <Core/EngineInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    AttenuationShape::AttenuationShape()
        : m_maxAttenuationFactor(1.0f)
    {}

    float AttenuationShape::GetAttenuationFactor(const Attenuation*, const hmm_vec3&, const ListenerInternalState*)
    {
        return m_maxAttenuationFactor;
    }

    float AttenuationShape::GetAttenuationFactor(const Attenuation*, const EntityInternalState*, const ListenerInternalState*)
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

    float ConeAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const ListenerInternalState* listener)
    {
        if (const hmm_vec3& soundToListener = listener->GetLocation() - soundLocation;
            AM_Length(soundToListener) >= attenuation->GetMaxDistance())
            return 0.0f;

        // Cone attenuation works only with PositionOrientation spatial sounds
        return 1.0f;
    }

    float ConeAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const EntityInternalState* entity, const ListenerInternalState* listener)
    {
        const hmm_vec3& soundToListener = listener->GetLocation() - entity->GetLocation();
        const float distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const float gain = attenuation->GetGainCurve().Get(distance);
        const float coneDist = AM_Dot(soundToListener, entity->GetDirection());

        if (coneDist < 0.0f || coneDist > _outerHeight)
            return gain * m_maxAttenuationFactor;

        if (coneDist > _innerHeight)
            return gain * AM_Lerp(1.0f, (coneDist - _innerHeight) / (_outerHeight - _innerHeight), m_maxAttenuationFactor);

        const float innerConeRadius = coneDist / _innerHeight * _innerRadius;
        const float outerConeRadius = coneDist / _outerHeight * _outerRadius;

        const float d = AM_Length(soundToListener - coneDist * entity->GetDirection());

        if (d <= innerConeRadius)
            return gain * 1.0f;
        if (d >= outerConeRadius)
            return gain * m_maxAttenuationFactor;

        const float delta = (distance - innerConeRadius) / (outerConeRadius - innerConeRadius);

        return gain * AM_Lerp(1.0f, AM_CLAMP(delta, 0.0f, 1.0f), m_maxAttenuationFactor);
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

    float SphereAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const ListenerInternalState* listener)
    {
        const hmm_vec3& soundToListener = listener->GetLocation() - soundLocation;
        const float distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const float gain = attenuation->GetGainCurve().Get(distance);
        const float delta = (distance - _innerRadius) / (_outerRadius - _innerRadius);

        return gain * AM_Lerp(1.0f, AM_CLAMP(delta, 0.0f, 1.0f), m_maxAttenuationFactor);
    }

    float SphereAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const EntityInternalState* entity, const ListenerInternalState* listener)
    {
        return GetAttenuationFactor(attenuation, entity->GetLocation(), listener);
    }

    float BoxAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const ListenerInternalState* listener)
    {
        const hmm_vec3& soundToListener = listener->GetLocation() - soundLocation;
        const float distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const float gain = attenuation->GetGainCurve().Get(distance);

        return gain * _getFactor(soundLocation, listener, AM_Mat4d(1.0f));
    }

    float BoxAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const EntityInternalState* entity, const ListenerInternalState* listener)
    {
        const hmm_vec3& soundToListener = listener->GetLocation() - entity->GetLocation();
        const float distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const float gain = attenuation->GetGainCurve().Get(distance);

        return gain * _getFactor(entity->GetLocation(), listener, AM_LookAt(AM_Vec3(0, 0, 0), entity->GetDirection(), entity->GetUp()));
    }

    float BoxAttenuationShape::_getFactor(const hmm_vec3& soundLocation, const ListenerInternalState* listener, hmm_mat4 lookAt)
    {
        lookAt = AM_Multiply(AM_Translate(soundLocation), lookAt);

        const hmm_vec3& x = listener->GetLocation();

        hmm_vec3 iP1, iP2, iP3, iP4, oP1, oP2, oP3, oP4;

        switch (amEngine->GetState()->up_axis)
        {
        default:
        case GameEngineUpAxis_Y:
            iP1 = AM_Multiply(lookAt, AM_Vec4(-_innerHalfWidth, -_innerHalfHeight, -_innerHalfDepth, 1.0f)).XYZ;
            iP2 = AM_Multiply(lookAt, AM_Vec4(-_innerHalfWidth, -_innerHalfHeight, _innerHalfDepth, 1.0f)).XYZ;
            iP3 = AM_Multiply(lookAt, AM_Vec4(_innerHalfWidth, -_innerHalfHeight, -_innerHalfDepth, 1.0f)).XYZ;
            iP4 = AM_Multiply(lookAt, AM_Vec4(-_innerHalfWidth, _innerHalfHeight, -_innerHalfDepth, 1.0f)).XYZ;

            oP1 = AM_Multiply(lookAt, AM_Vec4(-_outerHalfWidth, -_outerHalfHeight, -_outerHalfDepth, 1.0f)).XYZ;
            oP2 = AM_Multiply(lookAt, AM_Vec4(-_outerHalfWidth, -_outerHalfHeight, _outerHalfDepth, 1.0f)).XYZ;
            oP3 = AM_Multiply(lookAt, AM_Vec4(_outerHalfWidth, -_outerHalfHeight, -_outerHalfDepth, 1.0f)).XYZ;
            oP4 = AM_Multiply(lookAt, AM_Vec4(-_outerHalfWidth, _outerHalfHeight, -_outerHalfDepth, 1.0f)).XYZ;
            break;

        case GameEngineUpAxis_Z:
            iP1 = AM_Multiply(lookAt, AM_Vec4(-_innerHalfWidth, -_innerHalfDepth, -_innerHalfHeight, 1.0f)).XYZ;
            iP2 = AM_Multiply(lookAt, AM_Vec4(-_innerHalfWidth, _innerHalfDepth, -_innerHalfHeight, 1.0f)).XYZ;
            iP3 = AM_Multiply(lookAt, AM_Vec4(_innerHalfWidth, -_innerHalfDepth, -_innerHalfHeight, 1.0f)).XYZ;
            iP4 = AM_Multiply(lookAt, AM_Vec4(-_innerHalfWidth, -_innerHalfDepth, _innerHalfHeight, 1.0f)).XYZ;

            oP1 = AM_Multiply(lookAt, AM_Vec4(-_outerHalfWidth, -_outerHalfDepth, -_outerHalfHeight, 1.0f)).XYZ;
            oP2 = AM_Multiply(lookAt, AM_Vec4(-_outerHalfWidth, _outerHalfDepth, -_outerHalfHeight, 1.0f)).XYZ;
            oP3 = AM_Multiply(lookAt, AM_Vec4(_outerHalfWidth, -_outerHalfDepth, -_outerHalfHeight, 1.0f)).XYZ;
            oP4 = AM_Multiply(lookAt, AM_Vec4(-_outerHalfWidth, -_outerHalfDepth, _outerHalfHeight, 1.0f)).XYZ;
            break;
        }

        hmm_vec3 iU = AM_Normalize(iP2 - iP1);
        hmm_vec3 iV = AM_Normalize(iP3 - iP1);
        hmm_vec3 iW = AM_Normalize(iP4 - iP1);

        hmm_vec3 oU = AM_Normalize(oP2 - oP1);
        hmm_vec3 oV = AM_Normalize(oP3 - oP1);
        hmm_vec3 oW = AM_Normalize(oP4 - oP1);

        const float iUX = AM_Dot(iU, x);
        const float iVX = AM_Dot(iV, x);
        const float iWX = AM_Dot(iW, x);

        const float oUX = AM_Dot(oU, x);
        const float oVX = AM_Dot(oV, x);
        const float oWX = AM_Dot(oW, x);

        const float iUP1 = AM_Dot(iU, iP1);
        const float iVP1 = AM_Dot(iV, iP1);
        const float iWP1 = AM_Dot(iW, iP1);

        const float iUP2 = AM_Dot(iU, iP2);
        const float iVP3 = AM_Dot(iV, iP3);
        const float iWP4 = AM_Dot(iW, iP4);

        const float oUP1 = AM_Dot(oU, oP1);
        const float oVP1 = AM_Dot(oV, oP1);
        const float oWP1 = AM_Dot(oW, oP1);

        const float oUP2 = AM_Dot(oU, oP2);
        const float oVP3 = AM_Dot(oV, oP3);
        const float oWP4 = AM_Dot(oW, oP4);

        if (AM_BETWEEN(iUX, iUP1, iUP2) && AM_BETWEEN(iVX, iVP1, iVP3) && AM_BETWEEN(iWX, iWP1, iWP4))
            return 1.0f;

        if (!(AM_BETWEEN(oUX, oUP1, oUP2) && AM_BETWEEN(oVX, oVP1, oVP3) && AM_BETWEEN(oWX, oWP1, oWP4)))
            return m_maxAttenuationFactor;

        const float dP1 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP2 - oP1))) / (_outerHalfDepth - _innerHalfDepth);
        const float dP2 = HMM_ABS(AM_Dot(x - oP2, AM_Normalize(oP1 - oP2))) / (_outerHalfDepth - _innerHalfDepth);
        const float dP3 = HMM_ABS(AM_Dot(x - oP3, AM_Normalize(oP1 - oP3))) / (_outerHalfWidth - _innerHalfWidth);
        const float dP4 = HMM_ABS(AM_Dot(x - oP4, AM_Normalize(oP1 - oP4))) / (_outerHalfHeight - _innerHalfHeight);
        const float dP5 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP3 - oP1))) / (_outerHalfWidth - _innerHalfWidth);
        const float dP6 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP4 - oP1))) / (_outerHalfHeight - _innerHalfHeight);

        const float shortestRoad = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

        return AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(shortestRoad, 0.0f, 1.0f), 1.0f);
    }

    float CapsuleAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const ListenerInternalState* listener)
    {
        return _getFactor(attenuation, soundLocation, listener, AM_Mat4d(1.0f));
    }

    float CapsuleAttenuationShape::GetAttenuationFactor(
        const Attenuation* attenuation, const EntityInternalState* entity, const ListenerInternalState* listener)
    {
        return _getFactor(
            attenuation, entity->GetLocation(), listener, AM_LookAt(AM_Vec3(0, 0, 0), entity->GetDirection(), entity->GetUp()));
    }

    float CapsuleAttenuationShape::_getFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const ListenerInternalState* listener, hmm_mat4 lookAt)
    {
        lookAt = AM_Multiply(AM_Translate(soundLocation), lookAt);

        const hmm_vec3& x = listener->GetLocation();

        const float distanceToOrigin = AM_Length(x - soundLocation);

        const float innerHalfHeight = _innerHalfHeight - _innerRadius;
        const float outerHalfHeight = _outerHalfHeight - _outerRadius;

        hmm_vec3 iA, iB, oA, oB;

        switch (amEngine->GetState()->up_axis)
        {
        default:
        case GameEngineUpAxis_Y:
            iA = AM_Multiply(lookAt, AM_Vec4(0.0f, innerHalfHeight, 0.0f, 1.0f)).XYZ;
            iB = AM_Multiply(lookAt, AM_Vec4(0.0f, -innerHalfHeight, 0.0f, 1.0f)).XYZ;

            oA = AM_Multiply(lookAt, AM_Vec4(0.0f, outerHalfHeight, 0.0f, 1.0f)).XYZ;
            oB = AM_Multiply(lookAt, AM_Vec4(0.0f, -outerHalfHeight, 0.0f, 1.0f)).XYZ;
            break;

        case GameEngineUpAxis_Z:
            iA = AM_Multiply(lookAt, AM_Vec4(0.0f, 0.0f, innerHalfHeight, 1.0f)).XYZ;
            iB = AM_Multiply(lookAt, AM_Vec4(0.0f, 0.0f, -innerHalfHeight, 1.0f)).XYZ;

            oA = AM_Multiply(lookAt, AM_Vec4(0.0f, 0.0f, outerHalfHeight, 1.0f)).XYZ;
            oB = AM_Multiply(lookAt, AM_Vec4(0.0f, 0.0f, -outerHalfHeight, 1.0f)).XYZ;
            break;
        }

        hmm_vec3 iE = iB - iA;
        hmm_vec3 iM = AM_Cross(iA, iB);

        hmm_vec3 oE = oB - oA;
        hmm_vec3 oM = AM_Cross(oA, oB);

        const float iDistanceToAxis = AM_Length(iM + AM_Cross(iE, x)) / AM_Length(iE);
        const float oDistanceToAxis = AM_Length(oM + AM_Cross(oE, x)) / AM_Length(oE);

        if (oDistanceToAxis >= attenuation->GetMaxDistance())
            return 0.0f;

        const float gain = attenuation->GetGainCurve().Get(iDistanceToAxis);

        const float iDistanceToA = AM_Length(x - iA);
        const float iDistanceToB = AM_Length(x - iB);

        const float oDistanceToA = AM_Length(x - oA);
        const float oDistanceToB = AM_Length(x - oB);

        if (iDistanceToAxis <= _innerRadius && distanceToOrigin <= innerHalfHeight)
            return gain * 1.0f;

        if (oDistanceToAxis >= _outerRadius && distanceToOrigin >= outerHalfHeight)
            return gain * m_maxAttenuationFactor;

        const float rDelta = 1.0f - (oDistanceToAxis - _innerRadius) / (_outerRadius - _innerRadius);
        const float hDelta = 1.0f - (distanceToOrigin - _innerHalfHeight) / (_outerHalfHeight - _innerHalfHeight);

        const float delta = AM_MIN(rDelta, hDelta);

        return gain * AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(delta, 0.0f, 1.0f), 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude
