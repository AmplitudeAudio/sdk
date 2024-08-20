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

#ifndef _AM_IMPLEMENTATION_MIXER_PIPELINE_OBSTRUCTION_PROCESSOR_H
#define _AM_IMPLEMENTATION_MIXER_PIPELINE_OBSTRUCTION_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <DSP/Filters/BiquadResonantFilter.h>

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

        void Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out) override
        {
            const float obstruction = layer->GetObstruction();

            if (&out != &in)
                AudioBuffer::Copy(in, 0, out, 0, out.GetFrameCount());

            const auto frames = out.GetFrameCount();
            const auto channels = out.GetChannelCount();
            const auto sampleRate = layer->GetSoundFormat().GetSampleRate();

            // Nothing to do if no obstruction
            if (obstruction < kEpsilon)
                return;

            _lpfCurve.SetStart({ 0, sampleRate / 2.0f });
            _lpfCurve.SetEnd({ 1, sampleRate / 2000.0f });

            const auto& lpfCurve = Engine::GetInstance()->GetObstructionLowPassCurve();
            const auto& gainCurve = Engine::GetInstance()->GetObstructionGainCurve();

            if (const AmReal32 lpf = lpfCurve.Get(obstruction); lpf > 0)
            {
                const AmUInt64 id = layer->GetId();

                if (!_obstructionFilters.contains(id))
                {
                    _filter.InitializeLowPass(std::ceil(_lpfCurve.Get(lpf)), 0.5f);
                    _obstructionFilters[id] = _filter.CreateInstance();
                }

                // Update the filter coefficients
                _obstructionFilters[id]->SetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY, _lpfCurve.Get(lpf));

                // Apply Low Pass Filter
                _obstructionFilters[id]->Process(in, out, frames, sampleRate);
            }

            const AmReal32 gain = gainCurve.Get(obstruction);

            // Apply Gain
            for (AmSize c = 0; c < channels; c++)
            {
                auto& outChannel = out[c];
                ScalarMultiply(outChannel.begin(), outChannel.begin(), gain, frames);
            }
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

#endif // _AM_IMPLEMENTATION_MIXER_PIPELINE_OBSTRUCTION_PROCESSOR_H
