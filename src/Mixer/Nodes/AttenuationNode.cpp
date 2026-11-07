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

#include <Core/EngineInternalState.h>
#include <DSP/Gain.h>
#include <Mixer/InstanceBlending.h>
#include <Mixer/Nodes/AttenuationNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    AttenuationNodeInstance::AttenuationNodeInstance()
        : ProcessorNodeInstance(false)
    {}

    const AudioBuffer* AttenuationNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const Attenuation* attenuation = layer->GetAttenuation();
        if (attenuation == nullptr)
            return input;

        const Listener& listener = layer->GetListener();

        AmReal32 targetGain = 1.0f;

        // Compute attenuated gain based on spatialization
        {
            const eSpatialization spatialization = layer->GetSpatialization();

            if (listener.Valid())
            {
                if (layer->IsMultiPosition())
                {
                    const AmSize instanceCount = layer->GetInstanceCount();
                    AmReal32 totalWeight = 0.0f;
                    AmReal32 blendedGain = 0.0f;

                    for (AmSize i = 0; i < instanceCount; ++i)
                    {
                        const AmReal32 weight = layer->GetInstanceWeight(i);

                        blendedGain += layer->GetInstanceAttenuationGain(i) * weight;
                        totalWeight += weight;
                    }

                    // Normalize by total weight
                    constexpr AmReal32 kMinTotalWeight = 1e-3f;

                    targetGain = totalWeight > kMinTotalWeight ? blendedGain / totalWeight : 0.0f;
                }
                else
                {
                    const Entity& entity = layer->GetEntity();
                    const AmVector3 location = layer->GetLocation();

                    if (spatialization == eSpatialization_PositionOrientation)
                    {
                        AMPLITUDE_ASSERT(entity.Valid());
                        targetGain *= attenuation->GetGain(location, layer->GetOrientation(), listener);
                    }
                    else if (spatialization == eSpatialization_HRTF && entity.Valid())
                    {
                        targetGain *= attenuation->GetGain(location, layer->GetOrientation(), listener);
                    }
                    else if (spatialization == eSpatialization_Position)
                    {
                        // Position-based spatialization, or HRTF-based spatialization without entity
                        targetGain *= attenuation->GetGain(location, listener);
                    }
                }
            }
            else
            {
                // No sound without listener on an attenuated source
                targetGain = 0.0f;
            }
        }

        const AmUInt64 frames = input->GetFrameCount();
        const AmUInt16 channels = _output.GetChannelCount();

        // Instances sharing one pipeline within a block must not ramp from each other's gain.
        if (layer->IsSharingPipelineAcrossInstances())
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].Invalidate();

        if (const AmUInt32 sampleRate = layer->GetSampleRate(); sampleRate > 0)
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].SetMinRampFrames(GainRampMinFrames(sampleRate));

        // Cull only once the gain has fully ramped down (or was never audible). Leaving the processors
        // initialized at 0 makes a later re-entry fade in instead of jumping.
        const bool silent = !_gain[0].IsInitialized() || (Gain::IsZero(_gain[0].GetGain()) && !_gain[0].IsRamping());
        if (Gain::IsZero(targetGain) && silent)
        {
            for (AmUInt16 c = 0; c < channels; ++c)
                _gain[c].Reset(0.0f);

            return nullptr;
        }

        for (AmUInt16 c = 0; c < channels; ++c)
            _gain[c].ApplyGain(targetGain, input->GetChannel(c), 0, _output[c], 0, frames, false);

        return &_output;
    }

    AttenuationNode::AttenuationNode()
        : Node("Attenuation")
    {}
} // namespace SparkyStudios::Audio::Amplitude
