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

#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/BFormat.h>
#include <Core/EngineInternalState.h>

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
        else
            _binauralizer.Configure(_ambisonicOrder, true, _hrirSphere.get());
    }

    const AudioBuffer* AmbisonicBinauralDecoderNodeInstance::Process(const AudioBuffer* input)
    {
        if (input->IsEmpty())
            return nullptr;

        for (AmUInt32 i = 0, l = input->GetChannelCount(); i < l; ++i)
            _soundField.CopyStream(input->GetChannel(i), i, input->GetFrameCount());

        _output.Clear();

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();

        if (mode == ePanningMode_Stereo)
            _decoder.Process(&_soundField, input->GetFrameCount(), _output);
        else
            _binauralizer.Process(&_soundField, input->GetFrameCount(), _output);

        return &_output;
    }

    void AmbisonicBinauralDecoderNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _soundField.Configure(_ambisonicOrder, true, frameCount);
    }

    AmbisonicBinauralDecoderNode::AmbisonicBinauralDecoderNode()
        : Node("AmbisonicBinauralDecoder")
    {}
} // namespace SparkyStudios::Audio::Amplitude
