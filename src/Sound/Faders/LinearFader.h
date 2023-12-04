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

#ifndef SS_AMPLITUDE_AUDIO_LINEAR_FADER_H
#define SS_AMPLITUDE_AUDIO_LINEAR_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr BeizerCurveControlPoints gLinearFaderCurveControlPoints = { 0.0f, 0.0f, 1.0f, 1.0f };

    class LinearFaderInstance final : public FaderInstance
    {
    public:
        LinearFaderInstance()
        {
            m_curve = gLinearFaderCurveControlPoints;
        }
    };

    [[maybe_unused]] static class LinearFader final : public Fader
    {
    public:
        LinearFader()
            : Fader("Linear")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(LinearFaderInstance);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(LinearFaderInstance, (LinearFaderInstance*)instance);
        }

        // [[nodiscard]] BeizerCurveControlPoints GetControlPoints() const override
        // {
        //     return gLinearFaderCurveControlPoints;
        // }
    } gLinearFader; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_LINEAR_FADER_H
