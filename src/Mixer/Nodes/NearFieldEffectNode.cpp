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
#include <Mixer/Nodes/NearFieldEffectNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kNearFieldMaxDistance = 1.0f; // 1 meter
    constexpr AmReal32 kNearFieldMinDistance = 0.1f; // 0.1 meter
    constexpr AmReal32 kNearFieldMaxGain = 9.0f;

    const AudioBuffer* NearFieldEffectNodeInstance::Process(const AudioBuffer* input)
    {
        // Only mono input is supported for this node.
        AMPLITUDE_ASSERT(input->GetChannelCount() == 1);

        const auto* layer = GetLayer();

        AmVec2 pannedGain = { 0.0f, 0.0f };

        AmReal32 nearFieldGain = layer->GetSound()->GetNearFieldGain().GetValue();
        if (nearFieldGain > 0.0f)
        {
            const auto& listener = layer->GetListener();

            const AmReal32 distance = AM_Len(listener.GetLocation() - layer->GetLocation());

            AmReal32 nearFieldFactor = 0.0f;
            if (distance < kNearFieldMaxDistance)
                nearFieldFactor = (1.0f / std::max(distance, kNearFieldMinDistance)) - 1.0f;

            pannedGain = Gain::CalculateStereoPannedGain(
                nearFieldFactor * nearFieldGain / kNearFieldMaxGain, layer->GetLocation(), listener.GetInverseMatrix());
        }

        const AmReal32 leftGainCurrent = _leftGainProcessor.GetGain();
        const AmReal32 rightGainCurrent = _rightGainProcessor.GetGain();

        const AmReal32 leftGainTarget = pannedGain.X;
        const AmReal32 rightGainTarget = pannedGain.Y;

        const bool isLeftGainZero = Gain::IsZero(leftGainCurrent) && Gain::IsZero(leftGainTarget);
        const bool isRightGainZero = Gain::IsZero(rightGainCurrent) && Gain::IsZero(rightGainTarget);

        if (isLeftGainZero && isRightGainZero)
        {
            _leftGainProcessor.SetGain(0.0f);
            _rightGainProcessor.SetGain(0.0f);

            return nullptr;
        }

        _output = AudioBuffer(input->GetFrameCount(), 2);

        {
            NearFieldProcessor processor(layer->GetSampleRate(), input->GetFrameCount());

            const auto& inChannel = input->GetChannel(0);
            auto& outChannelLeft = _output.GetChannel(0);
            auto& outChannelRight = _output.GetChannel(1);

            // Apply bass boost and delay compensation(if necessary) to the input signal
            // and place it temporarily in the right output channel. This way we avoid
            // allocating a temporary buffer.
            processor.Process(inChannel, outChannelRight, layer->GetSpatialization() == eSpatialization_HRTF);

            _leftGainProcessor.ApplyGain(leftGainTarget, outChannelRight, 0, outChannelLeft, 0, input->GetFrameCount(), false);
            _rightGainProcessor.ApplyGain(rightGainTarget, outChannelRight, 0, outChannelRight, 0, input->GetFrameCount(), false);
        }

        return &_output;
    }

    NearFieldEffectNode::NearFieldEffectNode()
        : Node("NearFieldEffect")
    {}
} // namespace SparkyStudios::Audio::Amplitude
