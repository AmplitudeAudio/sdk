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
    class ObstructionProcessorInstance final : public SoundProcessorInstance
    {
    public:
        ObstructionProcessorInstance()
            : _lpfCurve()
            , _filter()
            , _obstructionFilters()
        {
            _lpfCurve.SetFader("Exponential");
        }

        ~ObstructionProcessorInstance() override
        {
            for (auto& [soundId, filter] : _obstructionFilters)
                _filter.DestroyInstance(filter);
        }

        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            const AmplimixLayer* layer) override
        {
            const float obstruction = layer->GetObstruction();

            if (out != in)
                std::memcpy(out, in, bufferSize);

            // Nothing to do if no obstruction
            if (obstruction < kEpsilon)
                return;

            _lpfCurve.SetStart({ 0, sampleRate / 2.0f });
            _lpfCurve.SetEnd({ 1, sampleRate / 2000.0f });

            const auto& lpfCurve = amEngine->GetObstructionLowPassCurve();
            const auto& gainCurve = amEngine->GetObstructionGainCurve();

            if (const AmReal32 lpf = lpfCurve.Get(obstruction); lpf > 0)
            {
                const AmUInt64 id = layer->GetId();

                if (!_obstructionFilters.contains(id))
                {
                    _filter.InitLowPass(std::ceil(_lpfCurve.Get(lpf)), 0.5f);
                    _obstructionFilters[id] = _filter.CreateInstance();
                }

                // Update the filter coefficients
                _obstructionFilters[id]->SetFilterParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY, _lpfCurve.Get(lpf));

                // Apply Low Pass Filter
                _obstructionFilters[id]->Process(out, frames, bufferSize, channels, sampleRate);
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

        void Cleanup(const AmplimixLayer* layer) override
        {
            if (!_obstructionFilters.contains(layer->GetId()))
                return;

            _filter.DestroyInstance(_obstructionFilters[layer->GetId()]);
            _obstructionFilters.erase(layer->GetId());
        }

    private:
        CurvePart _lpfCurve;
        BiquadResonantFilter _filter;

        std::map<AmObjectID, FilterInstance*> _obstructionFilters;
    };

    class ObstructionProcessor final : public SoundProcessor
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
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_OBSTRUCTION_PROCESSOR_H
