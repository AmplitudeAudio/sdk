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

#ifndef SS_AMPLITUDE_AUDIO_EXPONENTIAL_FADER_H
#define SS_AMPLITUDE_AUDIO_EXPONENTIAL_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ExponentialFaderInstance final : public FaderInstance
    {
    public:
        explicit ExponentialFaderInstance(AmReal64 k);

        AmReal64 GetFromPercentage(AmReal64 percentage) override;

    private:
        AmReal64 _k;
    };

    [[maybe_unused]] static class ExponentialSmoothFader final : public Fader
    {
    public:
        ExponentialSmoothFader()
            : Fader("ExponentialSmooth")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(ExponentialFaderInstance, 0.5);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(ExponentialFaderInstance, (ExponentialFaderInstance*)instance);
        }
    } gExponentialSmoothFader; // NOLINT(cert-err58-cpp)

    [[maybe_unused]] static class ExponentialSharpFader final : public Fader
    {
    public:
        ExponentialSharpFader()
            : Fader("ExponentialSharp")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(ExponentialFaderInstance, 0.9);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(ExponentialFaderInstance, (ExponentialFaderInstance*)instance);
        }
    } gExponentialSharpFader; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_EXPONENTIAL_FADER_H
