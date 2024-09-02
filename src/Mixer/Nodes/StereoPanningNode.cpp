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

    StereoPanningNodeInstance::StereoPanningNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : ProcessorNodeInstance(id, pipeline)
    {}

    AudioBuffer StereoPanningNodeInstance::Process(const AudioBuffer& input)
    {
        const auto* layer = GetLayer();
        if (layer == nullptr)
            return input.Clone();

        // Mono channels required for input
        AMPLITUDE_ASSERT(input.GetChannelCount() == 1);

        // Stereo channels for output
        AudioBuffer output(input.GetFrameCount(), 2);

        // Apply panning
        {
            const auto& listener = layer->GetListener();
            const AmReal32 gain = layer->GetGain();
            const AmVec2 pannedGain = Gain::CalculateStereoPannedGain(gain, layer->GetLocation(), listener.GetInverseMatrix());

            Gain::ApplyReplaceConstantGain(pannedGain.Left, input[0], 0, output[0], 0, output.GetFrameCount());
            Gain::ApplyReplaceConstantGain(pannedGain.Right, input[0], 0, output[1], 0, output.GetFrameCount());
        }

        return output;
    }

    StereoPanningNode::StereoPanningNode()
        : Node("StereoPanning")
    {}
} // namespace SparkyStudios::Audio::Amplitude
