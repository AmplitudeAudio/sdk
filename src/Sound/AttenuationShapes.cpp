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

#include <Core/EngineInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    AttenuationZoneImpl::AttenuationZoneImpl()
        : m_maxAttenuationFactor(1.0f)
    {}

    AmReal32 AttenuationZoneImpl::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVector3& soundLocation, const Listener& listener)
    {
        return m_maxAttenuationFactor;
    }

    AmReal32 AttenuationZoneImpl::GetAttenuationFactor(const Attenuation*, const Entity&, const Listener&)
    {
        return m_maxAttenuationFactor;
    }

    AttenuationZone* AttenuationZoneImpl::Create(const AttenuationShapeDefinition* definition)
    {
        if (definition == nullptr)
            return nullptr;

        AttenuationZoneImpl* shape = nullptr;

        switch (definition->zone_type())
        {
        default:
        case ZoneDefinition_Cone:
            shape = ampoolnew(eMemoryPoolKind_Engine, ConeAttenuationZone, definition->zone_as_Cone());
            break;
        case ZoneDefinition_Sphere:
            shape = ampoolnew(eMemoryPoolKind_Engine, SphereAttenuationZone, definition->zone_as_Sphere());
            break;
        case ZoneDefinition_Box:
            shape = ampoolnew(eMemoryPoolKind_Engine, BoxAttenuationZone, definition->zone_as_Box());
            break;
        case ZoneDefinition_Capsule:
            shape = ampoolnew(eMemoryPoolKind_Engine, CapsuleAttenuationZone, definition->zone_as_Capsule());
            break;
        }

        shape->m_maxAttenuationFactor = definition->max_attenuation_factor();

        return shape;
    }

    AmReal32 ConeAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVector3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        if (const AmVector3& soundToListener = Sub(listener.GetLocation(), soundLocation);
            Length(soundToListener) >= attenuation->GetMaxDistance())
            return 0.0f;

        // Cone attenuation works only with PositionOrientation spatial sounds
        return 1.0f;
    }

    AmReal32 ConeAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetLocation(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetOrientation());

        m_outerShape->SetLocation(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetOrientation());

        const AmVector3& soundToListener = Sub(listener.GetLocation(), entity.GetLocation());
        const AmReal32 distance = Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        if (!m_outerShape->Contains(listener.GetLocation()))
            return gain * m_maxAttenuationFactor;

        return gain * Lerp(Zone::GetFactor(listener), m_maxAttenuationFactor, 1.0f);
    }

    AmReal32 SphereAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVector3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        const AmVector3& soundToListener = Sub(listener.GetLocation(), soundLocation);
        const AmReal32 distance = Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * Lerp(Zone::GetFactor(listener), m_maxAttenuationFactor, 1.0f);
    }

    AmReal32 SphereAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        return GetAttenuationFactor(attenuation, entity.GetLocation(), listener);
    }

    AmReal32 BoxAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVector3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        const AmVector3& soundToListener = Sub(listener.GetLocation(), soundLocation);
        const AmReal32 distance = Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * Lerp(Zone::GetFactor(listener), m_maxAttenuationFactor, 1.0f);
    }

    AmReal32 BoxAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetLocation(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetOrientation());

        m_outerShape->SetLocation(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetOrientation());

        const AmVector3& soundToListener = Sub(listener.GetLocation(), entity.GetLocation());
        const AmReal32 distance = Length(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * Lerp(Zone::GetFactor(listener), m_maxAttenuationFactor, 1.0f);
    }

    // AmReal32 BoxAttenuationZone::GetFactor(const AmVector3& soundLocation, const Listener& listener, AmMatrix4 lookAt)
    // {
    //     const auto* inner = static_cast<BoxShape*>(m_innerShape);
    //     const auto* outer = static_cast<BoxShape*>(m_outerShape);
    //     const eGameEngineUpAxis upAxis = amEngine->GetState()->up_axis;

    //     lookAt = AM_Mul(AM_Translate(soundLocation), lookAt);

    //     const AmVector3& x = listener.GetLocation();

    //     AmVector3 iP1, iP2, iP3, iP4, oP1, oP2, oP3, oP4;

    //     switch (upAxis)
    //     {
    //     default:
    //     case eGameEngineUpAxis_Y:
    //         iP1 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP2 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfHeight(), inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP3 = AM_Mul(lookAt, AM_V4(inner->GetHalfWidth(), -inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP4 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;

    //         oP1 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP2 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfHeight(), outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP3 = AM_Mul(lookAt, AM_V4(outer->GetHalfWidth(), -outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP4 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         break;

    //     case eGameEngineUpAxis_Z:
    //         iP1 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP2 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP3 = AM_Mul(lookAt, AM_V4(inner->GetHalfWidth(), -inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP4 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfDepth(), inner->GetHalfHeight(), 1.0f)).XYZ;

    //         oP1 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP2 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP3 = AM_Mul(lookAt, AM_V4(outer->GetHalfWidth(), -outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP4 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfDepth(), outer->GetHalfHeight(), 1.0f)).XYZ;
    //         break;
    //     }

    //     AmVector3 iU = Normalize(iP2 - iP1);
    //     AmVector3 iV = Normalize(iP3 - iP1);
    //     AmVector3 iW = Normalize(iP4 - iP1);

    //     AmVector3 oU = Normalize(oP2 - oP1);
    //     AmVector3 oV = Normalize(oP3 - oP1);
    //     AmVector3 oW = Normalize(oP4 - oP1);

    //     const AmReal32 iUX = Dot(iU, x);
    //     const AmReal32 iVX = Dot(iV, x);
    //     const AmReal32 iWX = Dot(iW, x);

    //     const AmReal32 oUX = Dot(oU, x);
    //     const AmReal32 oVX = Dot(oV, x);
    //     const AmReal32 oWX = Dot(oW, x);

    //     const AmReal32 iUP1 = Dot(iU, iP1);
    //     const AmReal32 iVP1 = Dot(iV, iP1);
    //     const AmReal32 iWP1 = Dot(iW, iP1);

    //     const AmReal32 iUP2 = Dot(iU, iP2);
    //     const AmReal32 iVP3 = Dot(iV, iP3);
    //     const AmReal32 iWP4 = Dot(iW, iP4);

    //     const AmReal32 oUP1 = Dot(oU, oP1);
    //     const AmReal32 oVP1 = Dot(oV, oP1);
    //     const AmReal32 oWP1 = Dot(oW, oP1);

    //     const AmReal32 oUP2 = Dot(oU, oP2);
    //     const AmReal32 oVP3 = Dot(oV, oP3);
    //     const AmReal32 oWP4 = Dot(oW, oP4);

    //     if (AM_BETWEEN(iUX, iUP1, iUP2) && AM_BETWEEN(iVX, iVP1, iVP3) && AM_BETWEEN(iWX, iWP1, iWP4))
    //         return 1.0f;

    //     if (!(AM_BETWEEN(oUX, oUP1, oUP2) && AM_BETWEEN(oVX, oVP1, oVP3) && AM_BETWEEN(oWX, oWP1, oWP4)))
    //         return m_maxAttenuationFactor;

    //     switch (upAxis)
    //     {
    //     default:
    //     case eGameEngineUpAxis_Y:
    //         {
    //             const AmReal32 dP1 = AM_ABS(Dot(x - oP1, Normalize(oP2 - oP1))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
    //             const AmReal32 dP2 = AM_ABS(Dot(x - oP2, Normalize(oP1 - oP2))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
    //             const AmReal32 dP3 = AM_ABS(Dot(x - oP3, Normalize(oP1 - oP3))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
    //             const AmReal32 dP4 = AM_ABS(Dot(x - oP4, Normalize(oP1 - oP4))) / (outer->GetHalfHeight() -
    //             inner->GetHalfHeight()); const AmReal32 dP5 = AM_ABS(Dot(x - oP1, Normalize(oP3 - oP1))) / (outer->GetHalfWidth()
    //             - inner->GetHalfWidth()); const AmReal32 dP6 = AM_ABS(Dot(x - oP1, Normalize(oP4 - oP1))) /
    //             (outer->GetHalfHeight() - inner->GetHalfHeight());

    //             const AmReal32 shortestRoad = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

    //             return AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(shortestRoad, 0.0f, 1.0f), 1.0f);
    //         }

    //     case eGameEngineUpAxis_Z:
    //         {
    //             const AmReal32 dP1 = AM_ABS(Dot(x - oP1, Normalize(oP2 - oP1))) / (outer->GetHalfHeight() -
    //             inner->GetHalfHeight()); const AmReal32 dP2 = AM_ABS(Dot(x - oP2, Normalize(oP1 - oP2))) / (outer->GetHalfHeight()
    //             - inner->GetHalfHeight()); const AmReal32 dP3 = AM_ABS(Dot(x - oP3, Normalize(oP1 - oP3))) /
    //             (outer->GetHalfWidth() - inner->GetHalfWidth()); const AmReal32 dP4 = AM_ABS(Dot(x - oP4, Normalize(oP1 - oP4))) /
    //             (outer->GetHalfDepth() - inner->GetHalfDepth()); const AmReal32 dP5 = AM_ABS(Dot(x - oP1, Normalize(oP3 - oP1))) /
    //             (outer->GetHalfWidth() - inner->GetHalfWidth()); const AmReal32 dP6 = AM_ABS(Dot(x - oP1, Normalize(oP4 - oP1))) /
    //             (outer->GetHalfDepth() - inner->GetHalfDepth());

    //             const AmReal32 shortestRoad = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

    //             return AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(shortestRoad, 0.0f, 1.0f), 1.0f);
    //         }
    //     }
    // }

    AmReal32 CapsuleAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVector3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        return GetFactor(attenuation, soundLocation, listener, kMatrix4Identity);
    }

    AmReal32 CapsuleAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetLocation(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetOrientation());

        m_outerShape->SetLocation(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetOrientation());

        return GetFactor(attenuation, entity.GetLocation(), listener, entity.GetOrientation().GetLookAtMatrix(kVector3Zero));
    }

    AmReal32 CapsuleAttenuationZone::GetFactor(
        const Attenuation* attenuation, const AmVector3& soundLocation, const Listener& listener, AmMatrix4 lookAt)
    {
        const auto* inner = dynamic_cast<CapsuleShape*>(m_innerShape.get());
        const auto* outer = dynamic_cast<CapsuleShape*>(m_outerShape.get());

        lookAt = Mul(Translation(soundLocation), lookAt);

        const AmVector3& x = listener.GetLocation();

        const AmReal32 distanceToOrigin = Length(Sub(x, soundLocation));

        const AmReal32 innerHalfHeight = inner->GetHalfHeight() - inner->GetRadius();
        const AmReal32 outerHalfHeight = outer->GetHalfHeight() - outer->GetRadius();

        const AmVector3 iA = Transform(lookAt, { 0.0f, 0.0f, innerHalfHeight, 1.0f }).xyz;
        const AmVector3 iB = Transform(lookAt, { 0.0f, 0.0f, -innerHalfHeight, 1.0f }).xyz;

        const AmVector3 oA = Transform(lookAt, { 0.0f, 0.0f, outerHalfHeight, 1.0f }).xyz;
        const AmVector3 oB = Transform(lookAt, { 0.0f, 0.0f, -outerHalfHeight, 1.0f }).xyz;

        const AmVector3 iE = Sub(iB, iA);
        const AmVector3 iM = Cross(iA, iB);

        const AmVector3 oE = Sub(oB, oA);
        const AmVector3 oM = Cross(oA, oB);

        const AmReal32 iDistanceToAxis = Length(Add(iM, Cross(iE, x))) / Length(iE);
        const AmReal32 oDistanceToAxis = Length(Add(oM, Cross(oE, x))) / Length(oE);

        if (oDistanceToAxis >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(iDistanceToAxis);

        const AmReal32 iDistanceToA = Length(Sub(x, iA));
        const AmReal32 iDistanceToB = Length(Sub(x, iB));

        if (iDistanceToAxis <= inner->GetRadius() && distanceToOrigin <= innerHalfHeight)
            return gain * 1.0f;

        if (iDistanceToA <= inner->GetRadius() || iDistanceToB <= inner->GetRadius())
            return gain * 1.0f;

        if (oDistanceToAxis >= outer->GetRadius() && distanceToOrigin >= outerHalfHeight)
            return gain * m_maxAttenuationFactor;

        const AmReal32 rDelta = 1.0f - (oDistanceToAxis - inner->GetRadius()) / (outer->GetRadius() - inner->GetRadius());
        const AmReal32 hDelta = 1.0f - (distanceToOrigin - inner->GetHalfHeight()) / (outer->GetHalfHeight() - inner->GetHalfHeight());

        const AmReal32 delta = AM_MIN(rDelta, hDelta);

        return gain * Lerp(AM_CLAMP(delta, 0.0f, 1.0f), m_maxAttenuationFactor, 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude
