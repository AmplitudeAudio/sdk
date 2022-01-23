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
#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

#include "attenuation_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class ConeAttenuationShape : public AttenuationShape
    {
    public:
        explicit ConeAttenuationShape(const ConeAttenuationSettings* settings)
            : AttenuationShape()
        {
            const auto* inner = settings->inner();
            const auto* outer = settings->outer();

            m_innerShape = new ConeShape(inner->radius(), inner->height());
            m_outerShape = new ConeShape(outer->radius(), outer->height());
        }

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class SphereAttenuationShape : public AttenuationShape
    {
    public:
        explicit SphereAttenuationShape(const SphereAttenuationSettings* settings)
            : AttenuationShape()
        {
            const auto* inner = settings->inner();
            const auto* outer = settings->outer();

            m_innerShape = new SphereShape(inner->radius());
            m_outerShape = new SphereShape(outer->radius());
        }

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class BoxAttenuationShape : public AttenuationShape
    {
    public:
        explicit BoxAttenuationShape(const BoxAttenuationSettings* settings)
            : AttenuationShape()
        {
            const auto* inner = settings->inner();
            const auto* outer = settings->outer();

            m_innerShape = new BoxShape(inner->half_width(), inner->half_height(), inner->half_depth());
            m_outerShape = new BoxShape(outer->half_width(), outer->half_height(), outer->half_depth());
        }

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;

        float GetFactor(const hmm_vec3& soundLocation, const Listener& listener, hmm_mat4& lookAt);
    };

    class CapsuleAttenuationShape : public AttenuationShape
    {
    public:
        explicit CapsuleAttenuationShape(const CapsuleAttenuationSettings* settings)
            : AttenuationShape()
        {
            const auto* inner = settings->inner();
            const auto* outer = settings->outer();

            m_innerShape = new CapsuleShape(inner->radius(), inner->half_height());
            m_outerShape = new CapsuleShape(outer->radius(), outer->half_height());
        }

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;

        float GetFactor(const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener, hmm_mat4& lookAt);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ATTENUATION_INTERNAL_STATE_H
