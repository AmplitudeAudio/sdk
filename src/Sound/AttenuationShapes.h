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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_ATTENUATION_INTERNAL_STATE_H
#define SS_AMPLITUDE_AUDIO_ATTENUATION_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>

#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

#include "attenuation_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class ConeAttenuationZone final
        : public AttenuationZone
        , public ConeZone
    {
    public:
        explicit ConeAttenuationZone(const ConeZoneSettings* settings)
            : AttenuationZone()
            , ConeZone(ConeShape::Create(settings->inner()), ConeShape::Create(settings->outer()))
        {}

        ~ConeAttenuationZone() override
        {
            amdelete(ConeShape, (ConeShape*)m_innerShape);
            amdelete(ConeShape, (ConeShape*)m_outerShape);
        }

        float GetAttenuationFactor(const Attenuation*, const AmVec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class SphereAttenuationZone final
        : public AttenuationZone
        , public SphereZone
    {
    public:
        explicit SphereAttenuationZone(const SphereZoneSettings* settings)
            : AttenuationZone()
            , SphereZone(SphereShape::Create(settings->inner()), SphereShape::Create(settings->outer()))
        {}

        ~SphereAttenuationZone() override
        {
            amdelete(SphereShape, (SphereShape*)m_innerShape);
            amdelete(SphereShape, (SphereShape*)m_outerShape);
        }

        float GetAttenuationFactor(const Attenuation*, const AmVec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class BoxAttenuationZone final
        : public AttenuationZone
        , public BoxZone
    {
    public:
        explicit BoxAttenuationZone(const BoxZoneSettings* settings)
            : AttenuationZone()
            , BoxZone(BoxShape::Create(settings->inner()), BoxShape::Create(settings->outer()))
        {}

        ~BoxAttenuationZone() override
        {
            amdelete(BoxShape, (BoxShape*)m_innerShape);
            amdelete(BoxShape, (BoxShape*)m_outerShape);
        }

        float GetAttenuationFactor(const Attenuation*, const AmVec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class CapsuleAttenuationZone final
        : public AttenuationZone
        , public CapsuleZone
    {
    public:
        explicit CapsuleAttenuationZone(const CapsuleZoneSettings* settings)
            : AttenuationZone()
            , CapsuleZone(CapsuleShape::Create(settings->inner()), CapsuleShape::Create(settings->outer()))
        {}

        ~CapsuleAttenuationZone() override
        {
            amdelete(CapsuleShape, (CapsuleShape*)m_innerShape);
            amdelete(CapsuleShape, (CapsuleShape*)m_outerShape);
        }

        float GetAttenuationFactor(const Attenuation*, const AmVec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;

        float GetFactor(const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener, AmMat4 lookAt);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ATTENUATION_INTERNAL_STATE_H
