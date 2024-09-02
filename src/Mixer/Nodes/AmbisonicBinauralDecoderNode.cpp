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
    AmbisonicBinauralDecoderNodeInstance::AmbisonicBinauralDecoderNodeInstance(
        AmObjectID id, const Pipeline* pipeline, const HRIRSphere* hrirSphere)
        : ProcessorNodeInstance(id, pipeline)
        , _hrirSphere(hrirSphere)
    {}

    AudioBuffer AmbisonicBinauralDecoderNodeInstance::Process(const AudioBuffer& input)
    {
        const auto* layer = GetLayer();
        if (layer == nullptr)
            return {};

        if (input.IsEmpty())
            return {};

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();

        BFormat soundField;
        soundField.Configure(mode, true, input.GetFrameCount());

        for (AmUInt32 i = 0, l = input.GetChannelCount(); i < l; ++i)
            soundField.CopyStream(input[i], i, input.GetFrameCount());

        AudioBuffer output(input.GetFrameCount(), 2);

        if (mode == ePanningMode_Stereo)
        {
            if (!_binauralizers.contains(layer->GetId()))
            {
                AmbisonicDecoder& decoder = _decoders[layer->GetId()];
                decoder.Configure(mode, true, eSpeakersPreset_Stereo);
            }

            AmbisonicDecoder& decoder = _decoders[layer->GetId()];
            decoder.Process(&soundField, input.GetFrameCount(), output);
        }
        else
        {
            if (!_binauralizers.contains(layer->GetId()))
            {
                AmbisonicBinauralizer& decoder = _binauralizers[layer->GetId()];
                decoder.Configure(mode, true, _hrirSphere);
            }

            AmbisonicBinauralizer& decoder = _binauralizers[layer->GetId()];
            decoder.Process(&soundField, input.GetFrameCount(), output);
        }

        return output;
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
