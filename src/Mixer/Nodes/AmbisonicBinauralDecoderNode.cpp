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

#include <Mixer/Nodes/AmbisonicBinauralDecoderNode.h>

#include <atomic>

#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/BFormat.h>
#include <Core/EngineInternalState.h>
#include <DSP/ChannelMap.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicBinauralDecoderNodeInstance::AmbisonicBinauralDecoderNodeInstance()
        : ProcessorNodeInstance(false)
    {
        _hrirSphere = Engine::GetInstance()->GetHRIRSphere();
        ePanningMode mode = Engine::GetInstance()->GetPanningMode();

        if (mode != ePanningMode_Stereo && _hrirSphere == nullptr)
            mode = ePanningMode_Stereo;

        _ambisonicOrder = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        if (mode == ePanningMode_Stereo)
            _decoder.Configure(_ambisonicOrder, true, eSpeakersPreset_Stereo);
    }

    const AudioBuffer* AmbisonicBinauralDecoderNodeInstance::Process(const AudioBuffer* input)
    {
        if (input->IsEmpty())
            return nullptr;

        for (AmUInt32 i = 0, l = input->GetChannelCount(); i < l; ++i)
            _soundField.CopyStream(input->GetChannel(i), i, input->GetFrameCount());

        _stereo.Clear();

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();

        if (mode == ePanningMode_Stereo)
            _decoder.Process(&_soundField, input->GetFrameCount(), _stereo);
        else
            _binauralizer.Process(&_soundField, input->GetFrameCount(), _stereo);

        ApplyChannelMap(_toOutput, _stereo, _output, input->GetFrameCount());

        return &_output;
    }

    void AmbisonicBinauralDecoderNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _soundField.Configure(_ambisonicOrder, true, frameCount);

        _stereo = AudioBuffer(frameCount, kAmStereoChannelCount);
        _toOutput = BuildChannelMap(ChannelLayout::Get(eChannelLayout_Stereo), GetPipelineOutputLayout());

        // Binaural is a headphone render: on any other output it feeds L and R only. Said once per run, on the game thread.
        static std::atomic<bool> warned{ false };
        if (GetPipelineOutputLayout().GetType() != eChannelLayout_Stereo && !warned.exchange(true))
            amLogWarning(
                "AmbisonicBinauralDecoder renders for headphones: on a %u-channel output it feeds L and R only. Use "
                "AmbisonicSpeakerDecoder for speakers.",
                static_cast<AmUInt32>(GetPipelineOutputLayout().GetChannelCount()));

        if (!_configured)
        {
            const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
            if (mode != ePanningMode_Stereo && _hrirSphere != nullptr)
                _binauralizer.Configure(
                    _ambisonicOrder, true, static_cast<AmUInt32>(frameCount), m_layer->GetSampleRate(), _hrirSphere.get());

            _configured = true;
        }
    }

    AmbisonicBinauralDecoderNode::AmbisonicBinauralDecoderNode()
        : Node("AmbisonicBinauralDecoder")
    {}
} // namespace SparkyStudios::Audio::Amplitude
