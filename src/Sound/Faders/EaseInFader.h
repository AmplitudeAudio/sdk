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

#ifndef SS_AMPLITUDE_AUDIO_EASE_IN_FADER_H
#define SS_AMPLITUDE_AUDIO_EASE_IN_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr BeizerCurveControlPoints gEaseInFaderCurveControlPoints = { 0.42f, 0.0f, 1.0f, 1.0f };

    class EaseInFaderInstance final : public FaderInstance
    {
    public:
        EaseInFaderInstance()
        {
            m_curve = gEaseInFaderCurveControlPoints;
        }
    };

    [[maybe_unused]] static class EaseInFader final : public Fader
    {
    public:
        EaseInFader()
            : Fader("EaseIn")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(EaseInFaderInstance);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(EaseInFaderInstance, (EaseInFaderInstance*)instance);
        }

        [[nodiscard]] BeizerCurveControlPoints GetControlPoints() const override
        {
            return gEaseInFaderCurveControlPoints;
        }
    } gEaseInFader; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_EASE_IN_FADER_H
