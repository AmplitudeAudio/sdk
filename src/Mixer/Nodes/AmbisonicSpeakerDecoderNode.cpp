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
#include <atomic>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Mixer/Nodes/AmbisonicSpeakerDecoderNode.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicSpeakerDecoderNodeInstance::AmbisonicSpeakerDecoderNodeInstance()
        : ProcessorNodeInstance(false)
    {
        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        _ambisonicOrder = AM_MAX(static_cast<AmUInt32>(mode), 1u);
    }

    AmUInt16 AmbisonicSpeakerDecoderNodeInstance::GetOutputChannelCount() const
    {
        return GetPipelineOutputLayout().GetChannelCount();
    }

    void AmbisonicSpeakerDecoderNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _soundField.Configure(_ambisonicOrder, true, static_cast<AmUInt32>(frameCount));

        _ready = false;
        _speakerCount = 0;

        const ChannelLayout& output = GetPipelineOutputLayout();

        // A reconfigure that only changes the frame count keeps the matrix, so it never takes the engine's cache mutex.
        if (_matrix == nullptr || _matrix->GetLayout() != output.GetType() || _matrix->GetOrder() != _ambisonicOrder)
            _matrix = amEngine->GetSpeakerDecodingMatrix(_ambisonicOrder, output);

        if (_matrix == nullptr ||
            !_shelfFilter.Configure(_ambisonicOrder, true, static_cast<AmUInt32>(frameCount), m_layer->GetSampleRate()))
        {
            _matrix = nullptr;
            static std::atomic<bool> warned{ false };
            if (!warned.exchange(true))
                amLogWarning("AmbisonicSpeakerDecoder could not set up decoding for order %u: the node renders silence.", _ambisonicOrder);
            return;
        }

        _shelfFilter.SetHighFrequencyGains(_matrix->GetHighFrequencyGains());

        // Coefficients cached per speaker so Process reads a flat table.
        for (AmUInt16 c = 0; c < output.GetChannelCount(); ++c)
        {
            if (output.IsLfe(c))
                continue;

            _speakerChannel[_speakerCount] = c;
            for (AmUInt32 a = 0; a < _matrix->GetChannelCount(); ++a)
                _coefficients[_speakerCount][a] = _matrix->GetCoefficient(c, a);
            ++_speakerCount;
        }

        _ready = true;
    }

    const AudioBuffer* AmbisonicSpeakerDecoderNodeInstance::Process(const AudioBuffer* input)
    {
        if (input->IsEmpty())
            return nullptr;

        _output.Clear();
        if (!_ready)
            return &_output;

        const AmUInt32 frames = static_cast<AmUInt32>(AM_MIN(input->GetFrameCount(), _output.GetFrameCount()));

        for (AmUInt32 i = 0, l = input->GetChannelCount(); i < l; ++i)
            _soundField.CopyStream(input->GetChannel(i), i, frames);

        _shelfFilter.Process(&_soundField, frames);

        for (AmUInt16 s = 0; s < _speakerCount; ++s)
        {
            AmReal32* out = _output[_speakerChannel[s]].begin();
            for (AmUInt32 c = 0; c < _matrix->GetChannelCount(); ++c)
                ScalarMultiplyAccumulate(_soundField.GetBufferChannel(c).begin(), out, _coefficients[s][c], frames);
        }

        return &_output;
    }

    AmbisonicSpeakerDecoderNode::AmbisonicSpeakerDecoderNode()
        : Node("AmbisonicSpeakerDecoder")
    {}
} // namespace SparkyStudios::Audio::Amplitude
