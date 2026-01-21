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

#include <Core/Engine.h>
#include <DSP/Gain.h>
#include <Mixer/Nodes/ReverbNode.h>

#include <numeric>

namespace SparkyStudios::Audio::Amplitude
{
    ReverbNodeInstance::ReverbNodeInstance()
        : ProcessorNodeInstance(false)
    {}

    void ReverbNodeInstance::Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline, AmSize paramCount)
    {
        ProcessorNodeInstance::Initialize(id, layer, pipeline, paramCount);
        Reset();

        _model.SetWidth(1);
        _model.SetWet(1);
        _model.SetDry(0);
        _model.SetMode(0);
    }

    void ReverbNodeInstance::Reset()
    {
        ProcessorNodeInstance::Reset();

        const Room& room = GetLayer()->GetRoom();
        if (!room.Valid() || !room.GetState()->WasUpdated())
            return;

        // Set room size
        {
            AmReal32 maxSurface = 0.0f;

            for (AmUInt32 i = 0; i < kAmRoomSurfaceCount; ++i)
                if (const AmReal32 surface = room.GetSurfaceArea(static_cast<eRoomWall>(i)); surface > maxSurface)
                    maxSurface = surface;

            const AmReal32 roomSize = room.GetVolume() / (maxSurface * std::sqrt(maxSurface));

            _model.SetRoomSize(roomSize);
        }

        // Set room total absorption
        {
            const auto* roomCoefficients = room.GetState()->GetCoefficients();
            const AmReal32 absorption =
                std::accumulate(roomCoefficients, roomCoefficients + kAmRoomSurfaceCount, 0.0f) / kAmRoomSurfaceCount;

            _model.SetDamp(absorption);
        }
    }

    void ReverbNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _tempBuffer = AudioBuffer(frameCount, channelCount);
    }

    const AudioBuffer* ReverbNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const Room& room = layer->GetRoom();
        if (!room.Valid())
            return nullptr;

        const AmReal32 roomGain = layer->GetChannel().GetState()->GetRoomGain(room.GetId());

        if (roomGain < kEpsilon)
            return nullptr;

        _output.Clear();
        _tempBuffer.Clear();

        // Apply reflections gain
        Gain::ApplyReplaceConstantGain(roomGain, input->GetChannel(0), 0, _tempBuffer[0], 0, _output.GetFrameCount());

        // Apply reverberation
        _model.ProcessReplace(
            _tempBuffer[0].begin(), _tempBuffer[0].begin(), _output[0].begin(), _output[1].begin(), _tempBuffer.GetFrameCount(), 1);

        return &_output;
    }

    ReverbNode::ReverbNode()
        : Node("Reverb")
    {}
} // namespace SparkyStudios::Audio::Amplitude
