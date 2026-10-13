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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <DSP/Gain.h>
#include <DSP/NearFieldProcessor.h>
#include <Math/LinearAlgebra.h>
#include <Mixer/Nodes/NearFieldEffectNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kNearFieldMaxDistance = 1.0f; // 1 meter
    constexpr AmReal32 kNearFieldMinDistance = 0.1f; // 0.1 meter
    constexpr AmReal32 kNearFieldMaxGain = 9.0f;

    const AudioBuffer* NearFieldEffectNodeInstance::Process(const AudioBuffer* input)
    {
        AMPLITUDE_ASSERT(_processor != nullptr);

        // Mono policy: before the panner, the pipeline hands this node the sound's mono fold-down.
        AMPLITUDE_ASSERT(input->GetChannelCount() == 1);

        const auto* layer = GetLayer();
        const ChannelLayout& output = GetPipelineOutputLayout();
        const AmUInt64 frames = input->GetFrameCount();

        SpeakerGains targets{};
        const Sound* sound = layer->GetSound();
        const Listener listener = layer->GetListener();
        if (sound != nullptr && listener.Valid())
        {
            const AmReal32 nearFieldGain = sound->GetNearFieldGain().GetValue();
            const AmReal32 distance = Length(Sub(listener.GetLocation(), layer->GetLocation()));

            AmReal32 nearFieldFactor = 0.0f;
            if (nearFieldGain > 0.0f && distance < kNearFieldMaxDistance)
                nearFieldFactor = (1.0f / std::max(distance, kNearFieldMinDistance)) - 1.0f;

            if (const AmReal32 gain = nearFieldFactor * nearFieldGain / kNearFieldMaxGain; !Gain::IsZero(gain))
            {
                AmReal32 azimuth = 0.0f;
                AmReal32 elevation = 0.0f;
                AM_UNUSED(ListenerRelativeDirection(layer->GetLocation(), listener.GetInverseMatrix(), azimuth, elevation));

                const SpeakerGains pan = SpeakerPanner::Pan(azimuth, elevation, output);
                for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
                    targets[o] = gain * pan[o];
            }
        }

        bool silent = true;
        for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
            silent &= Gain::IsZero(_speakerGains[o].GetGain()) && Gain::IsZero(targets[o]);

        if (silent)
        {
            for (auto& processor : _speakerGains)
                processor.SetGain(0.0f);

            return nullptr;
        }

        // Bass boost and delay compensation once, then the boost is panned like the sound itself.
        _processor->Process(input->GetChannel(0), _boosted[0], layer->GetSpatialization() == eSpatialization_HRTF);

        for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
            _speakerGains[o].ApplyGain(targets[o], _boosted[0], 0, _output[o], 0, frames, false);

        return &_output;
    }

    NearFieldEffectNodeInstance::~NearFieldEffectNodeInstance()
    {
        ampooldelete(eMemoryPoolKind_Amplimix, NearFieldProcessor, _processor);
        _processor = nullptr;
    }

    void NearFieldEffectNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        if (_processor != nullptr)
        {
            ampooldelete(eMemoryPoolKind_Amplimix, NearFieldProcessor, _processor);
            _processor = nullptr;
        }

        // The processor owns biquad filter states and a delay line: it must persist across audio blocks.
        _processor = ampoolnew(eMemoryPoolKind_Amplimix, NearFieldProcessor, GetLayer()->GetSampleRate(), frameCount);

        _boosted = AudioBuffer(frameCount, 1);

        for (auto& processor : _speakerGains)
            processor.Invalidate();
    }

    NearFieldEffectNode::NearFieldEffectNode()
        : Node("NearFieldEffect")
    {}
} // namespace SparkyStudios::Audio::Amplitude
