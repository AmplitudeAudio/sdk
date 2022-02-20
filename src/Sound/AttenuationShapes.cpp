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
    AttenuationZone::AttenuationZone()
        : m_maxAttenuationFactor(1.0f)
    {}

    AmReal32 AttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener)
    {
        return m_maxAttenuationFactor;
    }

    AmReal32 AttenuationZone::GetAttenuationFactor(const Attenuation*, const Entity&, const Listener&)
    {
        return m_maxAttenuationFactor;
    }

    AttenuationZone* AttenuationZone::Create(const AttenuationShapeDefinition* definition)
    {
        if (definition == nullptr)
            return nullptr;

        AttenuationZone* shape = nullptr;

        switch (definition->zone_type())
        {
        default:
        case ZoneDefinition_Cone:
            shape = new ConeAttenuationZone(definition->zone_as_Cone());
            break;
        case ZoneDefinition_Sphere:
            shape = new SphereAttenuationZone(definition->zone_as_Sphere());
            break;
        case ZoneDefinition_Box:
            shape = new BoxAttenuationZone(definition->zone_as_Box());
            break;
        case ZoneDefinition_Capsule:
            shape = new CapsuleAttenuationZone(definition->zone_as_Capsule());
            break;
        }

        shape->m_maxAttenuationFactor = definition->max_attenuation_factor();

        return shape;
    }

    AmReal32 ConeAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetPosition(soundLocation);
        m_innerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        m_outerShape->SetPosition(soundLocation);
        m_outerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        if (const hmm_vec3& soundToListener = listener.GetLocation() - soundLocation;
            AM_Length(soundToListener) >= attenuation->GetMaxDistance())
            return 0.0f;

        // Cone attenuation works only with PositionOrientation spatial sounds
        return 1.0f;
    }

    AmReal32 ConeAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetPosition(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetDirection(), entity.GetUp());

        m_outerShape->SetPosition(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetDirection(), entity.GetUp());

        const hmm_vec3& soundToListener = listener.GetLocation() - entity.GetLocation();
        const AmReal32 distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        if (!m_outerShape->Contains(listener.GetLocation()))
            return gain * m_maxAttenuationFactor;

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    AmReal32 SphereAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetPosition(soundLocation);
        m_innerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        m_outerShape->SetPosition(soundLocation);
        m_outerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        const hmm_vec3& soundToListener = listener.GetLocation() - soundLocation;
        const AmReal32 distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    AmReal32 SphereAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        return GetAttenuationFactor(attenuation, entity.GetLocation(), listener);
    }

    AmReal32 BoxAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetPosition(soundLocation);
        m_innerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        m_outerShape->SetPosition(soundLocation);
        m_outerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        const hmm_vec3& soundToListener = listener.GetLocation() - soundLocation;
        const AmReal32 distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    AmReal32 BoxAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetPosition(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetDirection(), entity.GetUp());

        m_outerShape->SetPosition(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetDirection(), entity.GetUp());

        const hmm_vec3& soundToListener = listener.GetLocation() - entity.GetLocation();
        const AmReal32 distance = AM_Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    // AmReal32 BoxAttenuationZone::GetFactor(const hmm_vec3& soundLocation, const Listener& listener, hmm_mat4 lookAt)
    // {
    //     const auto* inner = static_cast<BoxShape*>(m_innerShape);
    //     const auto* outer = static_cast<BoxShape*>(m_outerShape);
    //     const GameEngineUpAxis upAxis = amEngine->GetState()->up_axis;

    //     lookAt = AM_Multiply(AM_Translate(soundLocation), lookAt);

    //     const hmm_vec3& x = listener.GetLocation();

    //     hmm_vec3 iP1, iP2, iP3, iP4, oP1, oP2, oP3, oP4;

    //     switch (upAxis)
    //     {
    //     default:
    //     case GameEngineUpAxis_Y:
    //         iP1 = AM_Multiply(lookAt, AM_Vec4(-inner->GetHalfWidth(), -inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP2 = AM_Multiply(lookAt, AM_Vec4(-inner->GetHalfWidth(), -inner->GetHalfHeight(), inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP3 = AM_Multiply(lookAt, AM_Vec4(inner->GetHalfWidth(), -inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP4 = AM_Multiply(lookAt, AM_Vec4(-inner->GetHalfWidth(), inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;

    //         oP1 = AM_Multiply(lookAt, AM_Vec4(-outer->GetHalfWidth(), -outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP2 = AM_Multiply(lookAt, AM_Vec4(-outer->GetHalfWidth(), -outer->GetHalfHeight(), outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP3 = AM_Multiply(lookAt, AM_Vec4(outer->GetHalfWidth(), -outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP4 = AM_Multiply(lookAt, AM_Vec4(-outer->GetHalfWidth(), outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         break;

    //     case GameEngineUpAxis_Z:
    //         iP1 = AM_Multiply(lookAt, AM_Vec4(-inner->GetHalfWidth(), -inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP2 = AM_Multiply(lookAt, AM_Vec4(-inner->GetHalfWidth(), inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP3 = AM_Multiply(lookAt, AM_Vec4(inner->GetHalfWidth(), -inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP4 = AM_Multiply(lookAt, AM_Vec4(-inner->GetHalfWidth(), -inner->GetHalfDepth(), inner->GetHalfHeight(), 1.0f)).XYZ;

    //         oP1 = AM_Multiply(lookAt, AM_Vec4(-outer->GetHalfWidth(), -outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP2 = AM_Multiply(lookAt, AM_Vec4(-outer->GetHalfWidth(), outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP3 = AM_Multiply(lookAt, AM_Vec4(outer->GetHalfWidth(), -outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP4 = AM_Multiply(lookAt, AM_Vec4(-outer->GetHalfWidth(), -outer->GetHalfDepth(), outer->GetHalfHeight(), 1.0f)).XYZ;
    //         break;
    //     }

    //     hmm_vec3 iU = AM_Normalize(iP2 - iP1);
    //     hmm_vec3 iV = AM_Normalize(iP3 - iP1);
    //     hmm_vec3 iW = AM_Normalize(iP4 - iP1);

    //     hmm_vec3 oU = AM_Normalize(oP2 - oP1);
    //     hmm_vec3 oV = AM_Normalize(oP3 - oP1);
    //     hmm_vec3 oW = AM_Normalize(oP4 - oP1);

    //     const AmReal32 iUX = AM_Dot(iU, x);
    //     const AmReal32 iVX = AM_Dot(iV, x);
    //     const AmReal32 iWX = AM_Dot(iW, x);

    //     const AmReal32 oUX = AM_Dot(oU, x);
    //     const AmReal32 oVX = AM_Dot(oV, x);
    //     const AmReal32 oWX = AM_Dot(oW, x);

    //     const AmReal32 iUP1 = AM_Dot(iU, iP1);
    //     const AmReal32 iVP1 = AM_Dot(iV, iP1);
    //     const AmReal32 iWP1 = AM_Dot(iW, iP1);

    //     const AmReal32 iUP2 = AM_Dot(iU, iP2);
    //     const AmReal32 iVP3 = AM_Dot(iV, iP3);
    //     const AmReal32 iWP4 = AM_Dot(iW, iP4);

    //     const AmReal32 oUP1 = AM_Dot(oU, oP1);
    //     const AmReal32 oVP1 = AM_Dot(oV, oP1);
    //     const AmReal32 oWP1 = AM_Dot(oW, oP1);

    //     const AmReal32 oUP2 = AM_Dot(oU, oP2);
    //     const AmReal32 oVP3 = AM_Dot(oV, oP3);
    //     const AmReal32 oWP4 = AM_Dot(oW, oP4);

    //     if (AM_BETWEEN(iUX, iUP1, iUP2) && AM_BETWEEN(iVX, iVP1, iVP3) && AM_BETWEEN(iWX, iWP1, iWP4))
    //         return 1.0f;

    //     if (!(AM_BETWEEN(oUX, oUP1, oUP2) && AM_BETWEEN(oVX, oVP1, oVP3) && AM_BETWEEN(oWX, oWP1, oWP4)))
    //         return m_maxAttenuationFactor;

    //     switch (upAxis)
    //     {
    //     default:
    //     case GameEngineUpAxis_Y:
    //         {
    //             const AmReal32 dP1 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP2 - oP1))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
    //             const AmReal32 dP2 = HMM_ABS(AM_Dot(x - oP2, AM_Normalize(oP1 - oP2))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
    //             const AmReal32 dP3 = HMM_ABS(AM_Dot(x - oP3, AM_Normalize(oP1 - oP3))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
    //             const AmReal32 dP4 = HMM_ABS(AM_Dot(x - oP4, AM_Normalize(oP1 - oP4))) / (outer->GetHalfHeight() -
    //             inner->GetHalfHeight()); const AmReal32 dP5 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP3 - oP1))) / (outer->GetHalfWidth()
    //             - inner->GetHalfWidth()); const AmReal32 dP6 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP4 - oP1))) /
    //             (outer->GetHalfHeight() - inner->GetHalfHeight());

    //             const AmReal32 shortestRoad = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

    //             return AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(shortestRoad, 0.0f, 1.0f), 1.0f);
    //         }

    //     case GameEngineUpAxis_Z:
    //         {
    //             const AmReal32 dP1 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP2 - oP1))) / (outer->GetHalfHeight() -
    //             inner->GetHalfHeight()); const AmReal32 dP2 = HMM_ABS(AM_Dot(x - oP2, AM_Normalize(oP1 - oP2))) / (outer->GetHalfHeight()
    //             - inner->GetHalfHeight()); const AmReal32 dP3 = HMM_ABS(AM_Dot(x - oP3, AM_Normalize(oP1 - oP3))) /
    //             (outer->GetHalfWidth() - inner->GetHalfWidth()); const AmReal32 dP4 = HMM_ABS(AM_Dot(x - oP4, AM_Normalize(oP1 - oP4))) /
    //             (outer->GetHalfDepth() - inner->GetHalfDepth()); const AmReal32 dP5 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP3 - oP1))) /
    //             (outer->GetHalfWidth() - inner->GetHalfWidth()); const AmReal32 dP6 = HMM_ABS(AM_Dot(x - oP1, AM_Normalize(oP4 - oP1))) /
    //             (outer->GetHalfDepth() - inner->GetHalfDepth());

    //             const AmReal32 shortestRoad = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

    //             return AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(shortestRoad, 0.0f, 1.0f), 1.0f);
    //         }
    //     }
    // }

    AmReal32 CapsuleAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetPosition(soundLocation);
        m_innerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        m_outerShape->SetPosition(soundLocation);
        m_outerShape->SetOrientation(AM_Vec3(0, 0, 0), AM_Vec3(0, 0, 0));

        return GetFactor(attenuation, soundLocation, listener, AM_Mat4d(1.0f));
    }

    AmReal32 CapsuleAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetPosition(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetDirection(), entity.GetUp());

        m_outerShape->SetPosition(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetDirection(), entity.GetUp());

        return GetFactor(attenuation, entity.GetLocation(), listener, AM_LookAt(AM_Vec3(0, 0, 0), entity.GetDirection(), entity.GetUp()));
    }

    AmReal32 CapsuleAttenuationZone::GetFactor(
        const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener, hmm_mat4 lookAt)
    {
        const auto* inner = dynamic_cast<CapsuleShape*>(m_innerShape);
        const auto* outer = dynamic_cast<CapsuleShape*>(m_outerShape);

        lookAt = AM_Multiply(AM_Translate(soundLocation), lookAt);

        const hmm_vec3& x = listener.GetLocation();

        const AmReal32 distanceToOrigin = AM_Length(x - soundLocation);

        const AmReal32 innerHalfHeight = inner->GetHalfHeight() - inner->GetRadius();
        const AmReal32 outerHalfHeight = outer->GetHalfHeight() - outer->GetRadius();

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

        const AmReal32 iDistanceToAxis = AM_Length(iM + AM_Cross(iE, x)) / AM_Length(iE);
        const AmReal32 oDistanceToAxis = AM_Length(oM + AM_Cross(oE, x)) / AM_Length(oE);

        if (oDistanceToAxis >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(iDistanceToAxis);

        const AmReal32 iDistanceToA = AM_Length(x - iA);
        const AmReal32 iDistanceToB = AM_Length(x - iB);

        if (iDistanceToAxis <= inner->GetRadius() && distanceToOrigin <= innerHalfHeight)
            return gain * 1.0f;

        if (iDistanceToA <= inner->GetRadius() || iDistanceToB <= inner->GetRadius())
            return gain * 1.0f;

        if (oDistanceToAxis >= outer->GetRadius() && distanceToOrigin >= outerHalfHeight)
            return gain * m_maxAttenuationFactor;

        const AmReal32 rDelta = 1.0f - (oDistanceToAxis - inner->GetRadius()) / (outer->GetRadius() - inner->GetRadius());
        const AmReal32 hDelta = 1.0f - (distanceToOrigin - inner->GetHalfHeight()) / (outer->GetHalfHeight() - inner->GetHalfHeight());

        const AmReal32 delta = AM_MIN(rDelta, hDelta);

        return gain * AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(delta, 0.0f, 1.0f), 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude
