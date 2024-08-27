// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace Gain
    {
        void ApplyReplaceConstantGain(
            AmReal32 gain, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames);
        void ApplyAdditiveConstantGain(
            AmReal32 gain, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames);

        void ApplyReplaceLinearGain(
            AmReal32 startGain,
            AmReal32 endGain,
            const AudioBuffer& in,
            AmSize inOffset,
            AudioBuffer& out,
            AmSize outOffset,
            AmSize frames);
        void ApplyAdditiveLinearGain(
            AmReal32 startGain,
            AmReal32 endGain,
            const AudioBuffer& in,
            AmSize inOffset,
            AudioBuffer& out,
            AmSize outOffset,
            AmSize frames);

        void ApplyReplaceGain(Curve gainCurve, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames);
        void ApplyAdditiveGain(Curve gainCurve, const AudioBuffer& in, AmSize inOffset, AudioBuffer& out, AmSize outOffset, AmSize frames);

        bool IsZero(AmReal32 gain);
        bool IsOne(AmReal32 gain);
    } // namespace Gain
} // namespace SparkyStudios::Audio::Amplitude