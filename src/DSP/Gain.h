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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace Gain
    {
        void ApplyReplaceConstantGain(
            AmReal32 gain, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);
        void ApplyAccumulateConstantGain(
            AmReal32 gain, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);

        void ApplyReplaceLinearGain(
            AmReal32 startGain,
            AmReal32 endGain,
            const AudioBufferChannel& in,
            AmSize inOffset,
            AudioBufferChannel& out,
            AmSize outOffset,
            AmSize frames);
        void ApplyAccumulateLinearGain(
            AmReal32 startGain,
            AmReal32 endGain,
            const AudioBufferChannel& in,
            AmSize inOffset,
            AudioBufferChannel& out,
            AmSize outOffset,
            AmSize frames);

        void ApplyReplaceGain(
            Curve gainCurve, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);
        void ApplyAccumulateGain(
            Curve gainCurve, const AudioBufferChannel& in, AmSize inOffset, AudioBufferChannel& out, AmSize outOffset, AmSize frames);

        bool IsZero(AmReal32 gain);
        bool IsOne(AmReal32 gain);

        AmVec2 CalculateStereoPannedGain(AmReal32 gain, AmVec3 sourcePosition, AmMat4 listenerViewMatrix);
        AmVec2 CalculateStereoPannedGain(AmReal32 gain, SphericalPosition sourcePosition);
        AmVec2 CalculateStereoPannedGain(AmReal32 gain, AmReal32 pan);
    } // namespace Gain

    class GainProcessor
    {
    public:
        GainProcessor();
        GainProcessor(AmReal32 initialGain);

        void ApplyGain(
            AmReal32 gain,
            const AudioBufferChannel& in,
            AmSize inOffset,
            AudioBufferChannel& out,
            AmSize outOffset,
            AmSize frames,
            bool accumulate);

        void SetGain(AmReal32 gain);

        [[nodiscard]] AM_INLINE AmReal32 GetGain() const
        {
            return _currentGain;
        }

    private:
        AmReal32 LinearGainRamp(
            AmSize rampLength, AmReal32 startGain, AmReal32 endGain, const AmReal32* in, AmReal32* out, AmSize frames, bool accumulate);

        AmReal32 _currentGain;
        bool _isInitialized;
    };
} // namespace SparkyStudios::Audio::Amplitude