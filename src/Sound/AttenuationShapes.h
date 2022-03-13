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
    class ConeAttenuationZone
        : public AttenuationZone
        , public ConeZone
    {
    public:
        explicit ConeAttenuationZone(const ConeZoneSettings* settings)
            : AttenuationZone()
            , ConeZone(
                  new ConeShape(settings->inner()->radius(), settings->inner()->height()),
                  new ConeShape(settings->outer()->radius(), settings->outer()->height()))
        {}

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class SphereAttenuationZone
        : public AttenuationZone
        , public SphereZone
    {
    public:
        explicit SphereAttenuationZone(const SphereZoneSettings* settings)
            : AttenuationZone()
            , SphereZone(new SphereShape(settings->inner()->radius()), new SphereShape(settings->outer()->radius()))
        {}

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class BoxAttenuationZone
        : public AttenuationZone
        , public BoxZone
    {
    public:
        explicit BoxAttenuationZone(const BoxZoneSettings* settings)
            : AttenuationZone()
            , BoxZone(
                  new BoxShape(settings->inner()->half_width(), settings->inner()->half_height(), settings->inner()->half_depth()),
                  new BoxShape(settings->outer()->half_width(), settings->outer()->half_height(), settings->outer()->half_depth()))
        {}

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;
    };

    class CapsuleAttenuationZone
        : public AttenuationZone
        , public CapsuleZone
    {
    public:
        explicit CapsuleAttenuationZone(const CapsuleZoneSettings* settings)
            : AttenuationZone()
            , CapsuleZone(
                  new CapsuleShape(settings->inner()->radius(), settings->inner()->half_height()),
                  new CapsuleShape(settings->outer()->radius(), settings->outer()->half_height()))
        {}

        float GetAttenuationFactor(const Attenuation*, const hmm_vec3& soundLocation, const Listener& listener) override;
        float GetAttenuationFactor(const Attenuation*, const Entity& entity, const Listener& listener) override;

        float GetFactor(const Attenuation* attenuation, const hmm_vec3& soundLocation, const Listener& listener, hmm_mat4 lookAt);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ATTENUATION_INTERNAL_STATE_H
