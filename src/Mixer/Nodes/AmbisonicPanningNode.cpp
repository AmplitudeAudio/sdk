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

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <Core/EngineInternalState.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicPanningNodeInstance::AmbisonicPanningNodeInstance()
    {
        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        _ambisonicOrder = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        for (auto& source : _sources)
            source.Configure(_ambisonicOrder, true);
    }

    bool AmbisonicPanningNodeInstance::ShouldSkip() const
    {
        const auto* layer = GetLayer();

        if (layer->GetSpatialization() != eSpatialization_HRTF)
            return true;

        const auto& listener = layer->GetListener();
        return !listener.Valid();
    }

    void AmbisonicPanningNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        NodeInstance::Configure(frameCount, channelCount);

        const ChannelLayout* source = ChannelLayout::FromChannelCount(channelCount);
        _sourceLayout = source != nullptr ? source : &ChannelLayout::LargestFitting(channelCount);

        _soundField.Configure(_ambisonicOrder, true, static_cast<AmUInt32>(frameCount));
        _emitterField.Configure(_ambisonicOrder, true, static_cast<AmUInt32>(frameCount));
        _scaledInput = AudioBuffer(frameCount, 1);
    }

    AmUInt16 AmbisonicPanningNodeInstance::GetOutputChannelCount() const
    {
        return static_cast<AmUInt16>(OrderToComponents(_ambisonicOrder, true));
    }

    void AmbisonicPanningNodeInstance::EncodeEmitters(
        const AudioBuffer* input, const AmVector3& location, const AmVector3& listenerPosition, AmReal32 spread, AmReal32 gain)
    {
        // World-locked encode: direction from listener to source, in world space; the rotator applies the listener's
        // orientation afterwards.
        const SphericalPosition centre = SphericalPosition::ForHRTF(Sub(location, listenerPosition));
        const SpreadEmitterSet emitters = SpreadEmitters::Compute(*_sourceLayout, 0.0f, 0.0f, spread);
        const AmUInt64 frames = input->GetFrameCount();

        for (AmUInt16 e = 0; e < emitters.count; ++e)
        {
            const SpreadEmitter& emitter = emitters.emitters[e];

            // Offsets turn around the vertical axis. The encoder's azimuth is negative to the left, the emitter's positive.
            const SphericalPosition position(
                centre.GetAzimuth() - emitter.azimuth * AM_DegToRad, centre.GetElevation(), centre.GetRadius());
            _sources[e].SetPosition(position, 0.25f);

            ScalarMultiply(input->GetChannel(emitter.channel).begin(), _scaledInput[0].begin(), gain * emitter.weight, frames);

            _emitterField.Reset();
            _sources[e].Process(_scaledInput.GetChannel(0), static_cast<AmUInt32>(frames), &_emitterField);
            _soundField += _emitterField;
        }
    }

    const AudioBuffer* AmbisonicPanningNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const auto& listener = layer->GetListener();
        const AmVector3 listenerPosition = listener.GetLocation();

        AmReal32 setting = kAmSpreadNative;
        if (const Sound* sound = layer->GetSound(); sound != nullptr)
            setting = sound->GetSpread().GetValue();

        AmReal32 spread = SpreadEmitters::Resolve(setting, *_sourceLayout);
        if (const Attenuation* attenuation = layer->GetAttenuation(); attenuation != nullptr)
            spread *= attenuation->GetSpreadFactor(Length(Sub(layer->GetLocation(), listenerPosition)));

        _soundField.Reset();

        if (layer->IsMultiPosition())
        {
            AmReal32 totalWeight = 0.0f;
            for (AmSize i = 0, count = layer->GetInstanceCount(); i < count; ++i)
            {
                const AmReal32 weight = layer->GetInstanceWeight(i);
                EncodeEmitters(input, layer->GetInstanceLocation(i), listenerPosition, spread, weight * layer->GetInstanceGain(i));
                totalWeight += weight;
            }

            // Normalize by total weight if needed
            if (totalWeight > kEpsilon && totalWeight != 1.0f)
                _soundField *= 1.0f / totalWeight;
        }
        else
        {
            EncodeEmitters(input, layer->GetLocation(), listenerPosition, spread, 1.0f);
        }

        return _soundField.GetBuffer();
    }

    AmbisonicPanningNode::AmbisonicPanningNode()
        : Node("AmbisonicPanning")
    {}
} // namespace SparkyStudios::Audio::Amplitude
