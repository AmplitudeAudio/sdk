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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Nodes/AmbisonicSpeakerDecoderNode.h>

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

        const ChannelLayout& output = GetPipelineOutputLayout();
        _speakerCount = 0;
        for (AmUInt16 c = 0; c < output.GetChannelCount(); ++c)
            if (!output.IsLfe(c))
                _speakerChannel[_speakerCount++] = c;

        // A custom set-up from the layout itself: the encoder's azimuth is negative to the left.
        _decoder.Configure(_ambisonicOrder, true, eSpeakersPreset_Custom, _speakerCount);
        for (AmUInt16 s = 0; s < _speakerCount; ++s)
            _decoder.SetSpeakerPosition(s, SphericalPosition(-output.GetAzimuth(_speakerChannel[s]) * AM_DegToRad, 0.0f, 1.0f));
        _decoder.Refresh();

        _speakers = AudioBuffer(frameCount, _speakerCount);
    }

    const AudioBuffer* AmbisonicSpeakerDecoderNodeInstance::Process(const AudioBuffer* input)
    {
        if (input->IsEmpty())
            return nullptr;

        const AmUInt64 frames = AM_MIN(static_cast<AmUInt64>(input->GetFrameCount()), static_cast<AmUInt64>(_speakers.GetFrameCount()));

        for (AmUInt32 i = 0, l = input->GetChannelCount(); i < l; ++i)
            _soundField.CopyStream(input->GetChannel(i), i, static_cast<AmUInt32>(frames));

        _speakers.Clear();
        _decoder.Process(&_soundField, static_cast<AmUInt32>(frames), _speakers);

        _output.Clear();
        for (AmUInt16 s = 0; s < _speakerCount; ++s)
            std::copy_n(_speakers[s].begin(), frames, _output[_speakerChannel[s]].begin());

        return &_output;
    }

    AmbisonicSpeakerDecoderNode::AmbisonicSpeakerDecoderNode()
        : Node("AmbisonicSpeakerDecoder")
    {}
} // namespace SparkyStudios::Audio::Amplitude
