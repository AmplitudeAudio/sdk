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

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <DSP/ChannelMap.h>
#include <Math/LinearAlgebra.h>
#include <Mixer/Nodes/SpeakerPanningNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    void SpeakerPanningNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        // The voice only plays named layouts; anything else keeps the channels a named layout can address.
        const ChannelLayout* source = ChannelLayout::FromChannelCount(channelCount);
        AMPLITUDE_ASSERT(source != nullptr);
        _sourceLayout = source != nullptr ? source : &ChannelLayout::LargestFitting(channelCount);

        _map2D = BuildChannelMap(*_sourceLayout, GetPipelineOutputLayout());

        // A new configuration starts from silence: the first block snaps to its gains.
        for (auto& row : _gains)
            for (auto& processor : row)
                processor.Invalidate();
    }

    AmUInt16 SpeakerPanningNodeInstance::GetOutputChannelCount() const
    {
        return GetPipelineOutputLayout().GetChannelCount();
    }

    bool SpeakerPanningNodeInstance::ShouldSkip() const
    {
        const auto* layer = GetLayer();
        if (layer->GetSpatialization() == eSpatialization_None)
            return false;

        return !layer->GetListener().Valid();
    }

    AmReal32 SpeakerPanningNodeInstance::EffectiveSpread(const Listener& listener) const
    {
        const auto* layer = GetLayer();

        AmReal32 setting = kAmSpreadNative;
        if (const Sound* sound = layer->GetSound(); sound != nullptr)
            setting = sound->GetSpread().GetValue();

        AmReal32 spread = SpreadEmitters::Resolve(setting, *_sourceLayout);

        if (const Attenuation* attenuation = layer->GetAttenuation(); attenuation != nullptr)
            spread *= attenuation->GetSpreadFactor(Length(Sub(layer->GetLocation(), listener.GetLocation())));

        return spread;
    }

    SpreadEmitterSet SpeakerPanningNodeInstance::SingleSourceGains(
        const AmVector3& location, const AmMatrix4& view, AmReal32 spread, EmitterGains& gains) const
    {
        // A source on the listener has no direction: it plays straight ahead.
        AmReal32 azimuth = 0.0f;
        AmReal32 elevation = 0.0f;
        AM_UNUSED(ListenerRelativeDirection(location, view, azimuth, elevation));

        const ChannelLayout& output = GetPipelineOutputLayout();
        const SpreadEmitterSet emitters = SpreadEmitters::Compute(*_sourceLayout, azimuth, elevation, spread);

        for (AmUInt16 e = 0; e < emitters.count; ++e)
        {
            const SpeakerGains pan = SpeakerPanner::Pan(emitters.emitters[e].azimuth, emitters.emitters[e].elevation, output);
            for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
                gains[e][o] = emitters.emitters[e].weight * pan[o];
        }

        return emitters;
    }

    SpreadEmitterSet SpeakerPanningNodeInstance::MultiSourceGains(const AmMatrix4& view, AmReal32 spread, EmitterGains& gains) const
    {
        const auto* layer = GetLayer();
        const ChannelLayout& output = GetPipelineOutputLayout();

        // Weighted for power: G = sqrt(sum(w * (a * g)^2) / sum(w)). One instance gives a * g.
        EmitterGains power{};
        AmReal32 totalWeight = 0.0f;
        SpreadEmitterSet emitters = SpreadEmitters::Compute(*_sourceLayout, 0.0f, 0.0f, spread);

        for (AmSize n = 0, count = layer->GetInstanceCount(); n < count; ++n)
        {
            const AmReal32 weight = layer->GetInstanceWeight(n);
            const AmReal32 instanceGain = layer->GetInstanceGain(n);

            EmitterGains instance{};
            emitters = SingleSourceGains(layer->GetInstanceLocation(n), view, spread, instance);

            for (AmUInt16 e = 0; e < emitters.count; ++e)
                for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
                    power[e][o] += weight * instanceGain * instanceGain * instance[e][o] * instance[e][o];

            totalWeight += weight;
        }

        for (AmUInt16 e = 0; e < emitters.count; ++e)
            for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
                gains[e][o] = totalWeight > kEpsilon ? std::sqrt(power[e][o] / totalWeight) : 0.0f;

        return emitters;
    }

    void SpeakerPanningNodeInstance::Render(const AudioBuffer* input, const SpreadEmitterSet& emitters, const EmitterGains& gains, AmUInt64 frames)
    {
        const ChannelLayout& output = GetPipelineOutputLayout();

        for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
            std::fill_n(_output.GetChannel(o).begin(), frames, 0.0f);

        for (AmUInt16 e = 0; e < emitters.count; ++e)
        {
            const auto& channel = input->GetChannel(emitters.emitters[e].channel);

            for (AmUInt16 o = 0; o < output.GetChannelCount(); ++o)
            {
                GainProcessor& processor = _gains[e][o];
                const AmReal32 target = gains[e][o];

                // Nothing to hear on this pair: keep the ramp state without touching the audio.
                if (processor.IsInitialized() && !processor.IsRamping() && processor.GetGain() == 0.0f && target == 0.0f)
                {
                    processor.Advance(target, frames);
                    continue;
                }

                processor.ApplyGain(target, channel, 0, _output.GetChannel(o), 0, frames, true);
            }
        }

        // The LFE is never panned: it reaches the output LFE, or nothing when the output has none.
        const AmInt32 sourceLfe = _sourceLayout->GetLfeChannel();
        const AmInt32 outputLfe = output.GetLfeChannel();
        if (sourceLfe >= 0 && outputLfe >= 0)
            Gain::ApplyAccumulateConstantGain(1.0f, input->GetChannel(sourceLfe), 0, _output.GetChannel(outputLfe), 0, frames);
    }

    const AudioBuffer* SpeakerPanningNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();
        const AmUInt64 frames = AM_MIN(static_cast<AmUInt64>(input->GetFrameCount()), static_cast<AmUInt64>(_output.GetFrameCount()));

        if (layer->GetSpatialization() == eSpatialization_None)
        {
            ApplyChannelMap(_map2D, *input, _output, frames);
            return &_output;
        }

        if (const AmUInt32 sampleRate = layer->GetSampleRate(); sampleRate > 0)
            for (auto& row : _gains)
                for (auto& processor : row)
                    processor.SetMinRampFrames(GainRampMinFrames(sampleRate));

        // Instances sharing one pipeline within a block must not ramp from each other's pan.
        if (layer->IsSharingPipelineAcrossInstances())
            for (auto& row : _gains)
                for (auto& processor : row)
                    processor.Invalidate();

        const Listener listener = layer->GetListener();
        const AmMatrix4& view = listener.GetInverseMatrix();
        const AmReal32 spread = EffectiveSpread(listener);

        EmitterGains gains{};
        const SpreadEmitterSet emitters =
            layer->IsMultiPosition() ? MultiSourceGains(view, spread, gains) : SingleSourceGains(layer->GetLocation(), view, spread, gains);

        Render(input, emitters, gains, frames);
        return &_output;
    }

    SpeakerPanningNode::SpeakerPanningNode(AmString name)
        : Node(std::move(name))
    {}
} // namespace SparkyStudios::Audio::Amplitude
