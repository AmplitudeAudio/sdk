// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#include <Mixer/Nodes/RoundoffClipNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    RoundoffClipNodeInstance::RoundoffClipNodeInstance()
        : ProcessorNodeInstance(false)
    {}

    const AudioBuffer* RoundoffClipNodeInstance::Process(const AudioBuffer* input)
    {
        AudioBuffer::Copy(*input, 0, _output, 0, input->GetFrameCount());

        for (AmSize c = 0; c < input->GetChannelCount(); c++)
        {
            const auto& inChannel = input->GetChannel(c);
            /* */ auto& outChannel = _output[c];

            for (AmSize i = 0, l = input->GetFrameCount(); i < l; i++)
            {
                const AmReal32& x = inChannel[i];
                /* */ AmReal32& y = outChannel[i];

                y = x <= -1.65f ? -0.9862875f : x >= 1.65f ? 0.9862875f : 0.87f * x - 0.1f * x * x * x;
            }
        }

        return &_output;
    }

    RoundoffClipNode::RoundoffClipNode()
        : Node("RoundoffClip")
    {}
} // namespace SparkyStudios::Audio::Amplitude
