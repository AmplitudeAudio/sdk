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

#ifndef SS_AMPLITUDE_AUDIO_OBSTRUCTION_PROCESSOR_H
#define SS_AMPLITUDE_AUDIO_OBSTRUCTION_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <Sound/Filters/BiquadResonantFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    static std::map<AmObjectID, FilterInstance*> gObstructionFilters = {};

    class ObstructionProcessorInstance final : public SoundProcessorInstance
    {
    public:
        ObstructionProcessorInstance()
            : _lpfCurve()
            , _lpFilter()
        {
            _lpfCurve.SetFader("Exponential");
        }

        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override
        {
            const float obstruction = sound->GetObstruction();

            if (out != in)
                std::memcpy(out, in, bufferSize);

            // Nothing to do if no obstruction
            if (obstruction < kEpsilon)
                return;

            _lpfCurve.SetStart({ 0, sampleRate / 2.0f });
            _lpfCurve.SetEnd({ 1, sampleRate / 2000.0f });

            const auto& lpfCurve = amEngine->GetState()->obstruction_config.lpf;
            const auto& gainCurve = amEngine->GetState()->obstruction_config.gain;

            if (const AmReal32 lpf = lpfCurve.Get(obstruction); lpf > 0)
            {
                if (!gObstructionFilters.contains(sound->GetId()))
                {
                    _lpFilter.InitLowPass(std::ceil(_lpfCurve.Get(lpf)), 0.5f);
                    gObstructionFilters[sound->GetId()] = _lpFilter.CreateInstance();
                }

                // Update the filter coefficients
                gObstructionFilters[sound->GetId()]->SetFilterParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY, _lpfCurve.Get(lpf));

                // Apply Low Pass Filter
                gObstructionFilters[sound->GetId()]->Process(out, frames, bufferSize, channels, sampleRate);
            }

            const AmReal32 gain = gainCurve.Get(obstruction);

            // Apply Gain
            const AmSize length = frames * channels;

#if defined(AM_SIMD_INTRINSICS)
            const AmSize end = AmAudioFrame::size * (length / AmAudioFrame::size);
            const auto factor = xsimd::batch(gain);

            for (AmSize i = 0; i < end; i += AmAudioFrame::size)
            {
                const auto bin = xsimd::load_aligned(&out[i]);
                xsimd::store_aligned(&out[i], xsimd::mul(bin, factor));
            }

            for (AmSize i = end; i < length; i++)
            {
                out[i] = out[i] * gain;
            }
#else
            for (AmSize i = 0; i < length; i++)
            {
                out[i] = out[i] * gain;
            }
#endif // AM_SIMD_INTRINSICS
        }

        void Cleanup(SoundInstance* sound) override
        {
            if (!gObstructionFilters.contains(sound->GetId()))
                return;

            _lpFilter.DestroyInstance(gObstructionFilters[sound->GetId()]);
            gObstructionFilters.erase(sound->GetId());
        }

    private:
        CurvePart _lpfCurve;
        BiquadResonantFilter _lpFilter;
    };

    [[maybe_unused]] static class ObstructionProcessor final : public SoundProcessor
    {
    public:
        ObstructionProcessor()
            : SoundProcessor("ObstructionProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, ObstructionProcessorInstance);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Amplimix, ObstructionProcessorInstance, (ObstructionProcessorInstance*)instance);
        }
    } gObstructionProcessor; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_OBSTRUCTION_PROCESSOR_H
