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

#include <Core/Engine.h>
#include <Mixer/Nodes/LimiterNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    LimiterNodeInstance::LimiterNodeInstance()
        : ProcessorNodeInstance(false)
    {}

    const AudioBuffer* LimiterNodeInstance::Process(const AudioBuffer* input)
    {
        const AmReal32 sampleRate = m_layer->GetSampleRate();
        if (sampleRate != _sampleRate)
            UpdateCoefficients(sampleRate);

        AudioBuffer::Copy(*input, 0, _output, 0, input->GetFrameCount());

        for (AmSize i = 0, l = _output.GetFrameCount(); i < l; i++)
        {
            AmReal32 peak = 0.0f;

            for (AmSize c = 0, m = _output.GetChannelCount(); c < m; c++)
                if (const AmReal32 absSample = std::abs(_output[c][i]); absSample > peak)
                    peak = absSample;

            if (peak > _enveloppe)
                _enveloppe = peak + (_enveloppe - peak) * _attackCoeff;
            else
                _enveloppe = peak + (_enveloppe - peak) * _releaseCoeff;

            AmReal32 gain = _enveloppe > GetParameter(ATTRIBUTE_THRESHOLD_DB) ? GetParameter(ATTRIBUTE_THRESHOLD_DB) / _enveloppe : 1.0f;

            for (AmSize c = 0, m = _output.GetChannelCount(); c < m; c++)
                _output[c][i] *= gain;
        }

        return &_output;
    }

    void LimiterNodeInstance::Reset()
    {
        ProcessorNodeInstance::Reset();

        _sampleRate = 0.0f;
        _attackCoeff = 0.0f;
        _releaseCoeff = 0.0f;
        _enveloppe = 0.0f;
    }

    void LimiterNodeInstance::SetParameter(AmSize index, AmReal32 value)
    {
        if (index == ATTRIBUTE_THRESHOLD_DB)
            value = std::pow(10.0f, value / 20.0f);

        ProcessorNodeInstance::SetParameter(index, value);
    }

    void LimiterNodeInstance::UpdateCoefficients(AmReal32 sampleRate)
    {
        _sampleRate = sampleRate;
        _attackCoeff = std::exp(-1.0f / (GetParameter(ATTRIBUTE_ATTACK_MS) * 0.001f * sampleRate));
        _releaseCoeff = std::exp(-1.0f / (GetParameter(ATTRIBUTE_RELEASE_MS) * 0.001f * sampleRate));
    }

    LimiterNode::LimiterNode()
        : Node("Limiter")
    {}
} // namespace SparkyStudios::Audio::Amplitude
