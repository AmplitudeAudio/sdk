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

#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

#include <Core/EntityInternalState.h>
#include <Core/ListenerInternalState.h>
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
            _innerRadius = settings->inner_radius();
            _outerRadius = settings->outer_radius();
            _innerHeight = settings->inner_height();
            _outerHeight = settings->outer_height();
        }

        float GetAttenuationFactor(const hmm_vec3& soundLocation, const ListenerInternalState* listener) override;
        float GetAttenuationFactor(const EntityInternalState* entity, const ListenerInternalState* listener) override;
        [[nodiscard]] float GetInnerRadius() const;
        [[nodiscard]] float GetOuterRadius() const;
        [[nodiscard]] float GetInnerHeight() const;
        [[nodiscard]] float GetOuterHeight() const;

    private:
        float _innerRadius;
        float _outerRadius;
        float _innerHeight;
        float _outerHeight;
    };

    class SphereAttenuationShape : public AttenuationShape
    {
    public:
        explicit SphereAttenuationShape(const SphereAttenuationSettings* settings)
            : AttenuationShape()
        {
            _innerRadius = settings->inner_radius();
            _outerRadius = settings->outer_radius();
        }

        float GetAttenuationFactor(const hmm_vec3& soundLocation, const ListenerInternalState* listener) override;
        float GetAttenuationFactor(const EntityInternalState* entity, const ListenerInternalState* listener) override;

        [[nodiscard]] float GetInnerRadius() const
        {
            return _innerRadius;
        }

        [[nodiscard]] float GetOuterRadius() const
        {
            return _outerRadius;
        }

    private:
        float _innerRadius;
        float _outerRadius;
    };

    class BoxAttenuationShape : public AttenuationShape
    {
    public:
        explicit BoxAttenuationShape(const BoxAttenuationSettings* settings)
            : AttenuationShape()
        {
            _innerHeight = settings->inner_height();
            _outerHeight = settings->outer_height();
            _innerWidth = settings->inner_width();
            _outerWidth = settings->outer_width();
        }

        [[nodiscard]] float GetInnerHeight() const
        {
            return _innerHeight;
        }

        [[nodiscard]] float GetOuterHeight() const
        {
            return _outerHeight;
        }

        [[nodiscard]] float GetInnerWidth() const
        {
            return _innerWidth;
        }

        [[nodiscard]] float GetOuterWidth() const
        {
            return _outerWidth;
        }

    private:
        float _innerHeight;
        float _outerHeight;
        float _innerWidth;
        float _outerWidth;
    };

    class CapsuleAttenuationShape : public AttenuationShape
    {
    public:
        explicit CapsuleAttenuationShape(const CapsuleAttenuationSettings* settings)
            : AttenuationShape()
        {
            _innerRadius = settings->inner_radius();
            _outerRadius = settings->outer_radius();
            _innerHeight = settings->inner_height();
            _outerHeight = settings->outer_height();
        }

        [[nodiscard]] float GetInnerRadius() const
        {
            return _innerRadius;
        }

        [[nodiscard]] float GetOuterRadius() const
        {
            return _outerRadius;
        }

        [[nodiscard]] float GetInnerHeight() const
        {
            return _innerHeight;
        }

        [[nodiscard]] float GetOuterHeight() const
        {
            return _outerHeight;
        }

    private:
        float _innerRadius;
        float _outerRadius;
        float _innerHeight;
        float _outerHeight;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ATTENUATION_INTERNAL_STATE_H
