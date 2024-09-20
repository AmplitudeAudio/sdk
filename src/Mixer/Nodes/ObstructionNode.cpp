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
#include <DSP/Filters/MonoPoleFilter.h>
#include <DSP/Gain.h>
#include <Mixer/Nodes/ObstructionNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    // Low pass filter coefficient for smoothing the applied obstruction. This avoids
    // sudden unrealistic changes in the volume of a sound object. Range [0, 1].
    // The value below has been calculated empirically.
    constexpr AmReal32 kObstructionSmoothingCoefficient = 0.75f;

    ObstructionNodeInstance::ObstructionNodeInstance()
        : _filter()
        , _currentObstruction(0)
        , _obstructionFilter(nullptr)
    {
        _obstructionFilter = _filter.CreateInstance();
    }

    ObstructionNodeInstance::~ObstructionNodeInstance()
    {
        _filter.DestroyInstance(_obstructionFilter);
        _obstructionFilter = nullptr;
    }

    const AudioBuffer* ObstructionNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const AmReal32 obstruction = layer->GetObstruction();

        const auto frames = input->GetFrameCount();
        const auto channels = input->GetChannelCount();
        const auto sampleRate = layer->GetSoundFormat().GetSampleRate();

        const Listener listener = layer->GetListener();
        const Entity entity = layer->GetEntity();

        _currentObstruction = AM_Lerp(_currentObstruction, kObstructionSmoothingCoefficient, obstruction);

        const auto& lpfCurve = Engine::GetInstance()->GetObstructionCoefficientCurve();
        const auto& gainCurve = Engine::GetInstance()->GetObstructionGainCurve();

        _output = AudioBuffer(frames, channels);

        if (const AmReal32 lpf = lpfCurve.Get(_currentObstruction); lpf > kEpsilon)
        {
            // Update the filter coefficients
            _obstructionFilter->SetParameter(MonoPoleFilter::ATTRIBUTE_COEFFICIENT, AM_CLAMP(lpf, 0.0f, 1.0f));

            // Apply Low Pass Filter
            _obstructionFilter->Process(*input, _output, frames, sampleRate);
        }
        else
        {
            _output = *input;
        }

        const AmReal32 gain = gainCurve.Get(_currentObstruction);

        // Apply Gain
        for (AmSize c = 0; c < channels; ++c)
            Gain::ApplyReplaceConstantGain(gain, _output[c], 0, _output[c], 0, frames);

        return &_output;
    }

    ObstructionNode::ObstructionNode()
        : Node("Obstruction")
    {}
} // namespace SparkyStudios::Audio::Amplitude
