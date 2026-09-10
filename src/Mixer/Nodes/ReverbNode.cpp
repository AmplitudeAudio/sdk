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

#include <algorithm>
#include <numeric>

namespace SparkyStudios::Audio::Amplitude
{
    ReverbNodeInstance::ReverbNodeInstance()
        : ReverbNodeInstance("Freeverb")
    {}

    ReverbNodeInstance::ReverbNodeInstance(AmString algorithmName)
        : ProcessorNodeInstance(false)
        , _algorithmName(std::move(algorithmName))
        , _reverbInstance(nullptr)
        , _tempBuffer()
    {}

    void ReverbNodeInstance::Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline, AmSize paramCount)
    {
        ProcessorNodeInstance::Initialize(id, layer, pipeline, paramCount);

        if (const auto reverb = Reverb::Find(_algorithmName); reverb != nullptr)
        {
            _reverbInstance = reverb->CreateInstance();
            if (_reverbInstance != nullptr)
            {
                AmUInt32 sampleRate = layer != nullptr ? layer->GetSampleRate() : 0;
                if (sampleRate == 0)
                    sampleRate = 48000;

                _reverbInstance->Initialize(sampleRate);
            }
        }
        else
        {
            amLogWarning("Failed to find reverb algorithm '%s'.", _algorithmName.c_str());
        }

        Reset();
    }

    void ReverbNodeInstance::Reset()
    {
        ProcessorNodeInstance::Reset();

        if (_reverbInstance != nullptr)
            _reverbInstance->Reset();

        const auto* layer = GetLayer();
        if (layer == nullptr)
            return;

        const Room& room = layer->GetRoom();
        if (!room.Valid())
            return;

        // Set room size
        AmReal32 maxSurface = 0.0f;

        for (AmUInt32 i = 0; i < kAmRoomSurfaceCount; ++i)
            if (const AmReal32 surface = room.GetSurfaceArea(static_cast<eRoomWall>(i)); surface > maxSurface)
                maxSurface = surface;

        const AmReal32 roomSize = maxSurface > 0.0f ? room.GetVolume() / (maxSurface * std::sqrt(maxSurface)) : 0.0f;

        // Set room total absorption
        AmReal32 absorption = 0.0f;
        if (room.GetState() != nullptr)
        {
            const auto* roomCoefficients = room.GetState()->GetCoefficients();
            if (roomCoefficients != nullptr)
                absorption = std::accumulate(roomCoefficients, roomCoefficients + kAmRoomSurfaceCount, 0.0f) / kAmRoomSurfaceCount;
        }

        if (_reverbInstance != nullptr)
            _reverbInstance->SetRoomParameters(roomSize, absorption);
    }

    void ReverbNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        ProcessorNodeInstance::Configure(frameCount, channelCount);

        _tempBuffer = AudioBuffer(frameCount, channelCount);

        if (_reverbInstance != nullptr)
            _reverbInstance->Configure(frameCount);
    }

    const AudioBuffer* ReverbNodeInstance::Process(const AudioBuffer* input)
    {
        if (_reverbInstance == nullptr || input == nullptr || input->IsEmpty())
            return nullptr;

        const auto* layer = GetLayer();
        if (layer == nullptr)
            return nullptr;

        const Room& room = layer->GetRoom();
        if (!room.Valid())
            return nullptr;

        const Channel& channel = layer->GetChannel();
        if (!channel.Valid() || channel.GetState() == nullptr)
            return nullptr;

        const AmReal32 roomGain = channel.GetState()->GetRoomGain(room.GetId());

        if (roomGain < kEpsilon)
            return nullptr;

        _output.Clear();
        _tempBuffer.Clear();

        const AmUInt64 frames = input->GetFrameCount();
        const AmUInt32 sampleRate = layer->GetSampleRate();

        for (AmUInt16 c = 0; c < _tempBuffer.GetChannelCount(); ++c)
        {
            const auto& inChannel = input->GetChannel(std::min(c, static_cast<AmUInt16>(input->GetChannelCount() - 1)));
            Gain::ApplyReplaceConstantGain(roomGain, inChannel, 0, _tempBuffer[c], 0, frames);
        }

        _reverbInstance->Process(_tempBuffer, _output, frames, sampleRate);

        return &_output;
    }

    ReverbNode::ReverbNode()
        : Node("Reverb")
        , _algorithmName("Freeverb")
    {}

    ReverbNode::ReverbNode(AmString algorithmName)
        : Node(algorithmName)
        , _algorithmName(std::move(algorithmName))
    {}

    ReverbNode::ReverbNode(AmString name, AmString algorithmName)
        : Node(std::move(name))
        , _algorithmName(std::move(algorithmName))
    {}
} // namespace SparkyStudios::Audio::Amplitude
