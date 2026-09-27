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
#include <SparkyStudios/Audio/Amplitude/DSP/AudioConverter.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <DSP/Gain.h>
#include <Mixer/Nodes/StereoPanningNode.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool StereoPanningNodeInstance::ShouldSkip() const
    {
        const auto* layer = GetLayer();
        const auto& listener = layer->GetListener();
        return !listener.Valid();
    }

    const AudioBuffer* StereoPanningNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();
        const auto& listener = layer->GetListener();
        const auto& listenerInvMatrix = listener.GetInverseMatrix();

        // Mono channels required for input
        AMPLITUDE_ASSERT(input->GetChannelCount() == 1);

        if (const AmUInt32 sampleRate = layer->GetSampleRate(); sampleRate > 0)
            for (auto& processor : _panGain)
                processor.SetMinRampFrames(GainRampMinFrames(sampleRate));

        // Instances sharing one pipeline within a block must not ramp from each other's pan.
        if (layer->IsSharingPipelineAcrossInstances())
            for (auto& processor : _panGain)
                processor.Invalidate();

        constexpr AmReal32 kGain = 1.0f;

        if (layer->GetSpatialization() == eSpatialization_None)
        {
            ApplyPan(input, Gain::CalculateStereoPannedGain(kGain, 0.0f));
        }
        else if (layer->IsMultiPosition())
        {
            const AmSize instanceCount = layer->GetInstanceCount();
            AmReal32 totalWeight = 0.0f;
            AmVector2 blendedPan = kVector2Zero;

            for (AmSize i = 0; i < instanceCount; ++i)
            {
                const AmVector3 location = layer->GetInstanceLocation(i);
                const AmReal32 weight = layer->GetInstanceWeight(i);
                const AmReal32 instanceGain = layer->GetInstanceGain(i);

                const AmVector2 pannedGain = Gain::CalculateStereoPannedGain(instanceGain, location, listenerInvMatrix);

                blendedPan.x += pannedGain.x * weight;
                blendedPan.y += pannedGain.y * weight;
                totalWeight += weight;
            }

            // Normalize by total weight
            if (totalWeight > kEpsilon)
            {
                blendedPan.x /= totalWeight;
                blendedPan.y /= totalWeight;
            }

            ApplyPan(input, blendedPan);
        }
        else
        {
            ApplyPan(input, Gain::CalculateStereoPannedGain(kGain, layer->GetLocation(), listenerInvMatrix));
        }

        return &_output;
    }

    void StereoPanningNodeInstance::ApplyPan(const AudioBuffer* input, const AmVector2& pannedGain)
    {
        const AmSize frames = _output.GetFrameCount();

        _panGain[0].ApplyGain(pannedGain.x, input->GetChannel(0), 0, _output[0], 0, frames, false);
        _panGain[1].ApplyGain(pannedGain.y, input->GetChannel(0), 0, _output[1], 0, frames, false);
    }

    StereoPanningNode::StereoPanningNode()
        : Node("StereoPanning")
    {}
} // namespace SparkyStudios::Audio::Amplitude
