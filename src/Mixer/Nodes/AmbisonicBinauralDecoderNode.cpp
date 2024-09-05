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
    AmbisonicBinauralDecoderNodeInstance::AmbisonicBinauralDecoderNodeInstance(const HRIRSphere* hrirSphere)
        : _hrirSphere(hrirSphere)
    {
        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1);

        if (mode == ePanningMode_Stereo)
            _decoder.Configure(order, true, eSpeakersPreset_Stereo);
        else
            _binauralizer.Configure(order, true, _hrirSphere);
    }

    const AudioBuffer* AmbisonicBinauralDecoderNodeInstance::Process(const AudioBuffer* input)
    {
        if (input->IsEmpty())
            return nullptr;

        const auto* layer = GetLayer();

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1);

        BFormat soundField;
        soundField.Configure(order, true, input->GetFrameCount());

        for (AmUInt32 i = 0, l = input->GetChannelCount(); i < l; ++i)
            soundField.CopyStream(input->GetChannel(i), i, input->GetFrameCount());

        _output = AudioBuffer(input->GetFrameCount(), 2);

        if (mode == ePanningMode_Stereo)
            _decoder.Process(&soundField, input->GetFrameCount(), _output);
        else
            _binauralizer.Process(&soundField, input->GetFrameCount(), _output);

        return &_output;
    }

    AmbisonicBinauralDecoderNode::AmbisonicBinauralDecoderNode()
        : Node("AmbisonicBinauralDecoder")
    {
        if (!_hrirSphere.IsLoaded())
        {
            // TODO: Get the HRIR file to load from settings.
            _hrirSphere.SetResource(AM_OS_STRING("./data/mit.amir"));
            _hrirSphere.Load(Engine::GetInstance()->GetFileSystem());
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
