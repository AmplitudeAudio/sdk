// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_MIXER_LAYER_GAIN_MIXER_H
#define _AM_IMPLEMENTATION_MIXER_LAYER_GAIN_MIXER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Gain.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Maximum number of output channels the mixer renders (mono or stereo).
     */
    constexpr AmUInt16 kAmplimixMaxOutputChannels = 2;

    /**
     * @brief Accumulates @p frames frames of @p in into @p out with a per-channel ramped gain.
     *
     * Advances @p processors: call once per layer per block.
     */
    AM_INLINE void MixLayerWithGain(
        GainProcessor* processors, AmUInt16 channelCount, AmReal32 gain, const AudioBuffer& in, AudioBuffer& out, AmSize frames)
    {
        AMPLITUDE_ASSERT(channelCount <= kAmplimixMaxOutputChannels);

        for (AmUInt16 c = 0; c < channelCount; ++c)
            processors[c].ApplyGain(gain, in.GetChannel(c), 0, out.GetChannel(c), 0, frames, true);
    }

    /**
     * @brief Accumulates one instance of a separate-mode layer using copies of @p processors.
     *
     * Every instance of the layer sees the same ramp for the block; @p processors are not modified.
     * Call @c AdvanceLayerGain() once after all instances.
     */
    AM_INLINE void MixLayerInstanceWithGain(
        const GainProcessor* processors, AmUInt16 channelCount, AmReal32 gain, const AudioBuffer& in, AudioBuffer& out, AmSize frames)
    {
        AMPLITUDE_ASSERT(channelCount <= kAmplimixMaxOutputChannels);

        for (AmUInt16 c = 0; c < channelCount; ++c)
        {
            GainProcessor instanceProcessor = processors[c];
            instanceProcessor.ApplyGain(gain, in.GetChannel(c), 0, out.GetChannel(c), 0, frames, true);
        }
    }

    /**
     * @brief Advances the layer's gain ramps by one block without processing audio.
     */
    AM_INLINE void AdvanceLayerGain(GainProcessor* processors, AmUInt16 channelCount, AmReal32 gain, AmSize frames)
    {
        AMPLITUDE_ASSERT(channelCount <= kAmplimixMaxOutputChannels);

        for (AmUInt16 c = 0; c < channelCount; ++c)
            processors[c].Advance(gain, frames);
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_LAYER_GAIN_MIXER_H
