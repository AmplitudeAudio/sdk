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

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <Core/EngineInternalState.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicPanningNodeInstance::AmbisonicPanningNodeInstance()
    {
        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        _ambisonicOrder = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        _source.Configure(_ambisonicOrder, true);
    }

    bool AmbisonicPanningNodeInstance::ShouldSkip() const
    {
        const auto* layer = GetLayer();

        if (layer->GetSpatialization() != eSpatialization_HRTF)
            return true;

        const auto& listener = layer->GetListener();
        return !listener.Valid();
    }

    void AmbisonicPanningNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        NodeInstance::Configure(frameCount, channelCount);

        _soundField.Configure(_ambisonicOrder, true, static_cast<AmUInt32>(frameCount));

        _instanceSoundField.Configure(_ambisonicOrder, true, static_cast<AmUInt32>(frameCount));
        _instanceScaledInput = AudioBuffer(frameCount, 1);
    }

    AmUInt16 AmbisonicPanningNodeInstance::GetOutputChannelCount() const
    {
        return static_cast<AmUInt16>(OrderToComponents(_ambisonicOrder, true));
    }

    const AudioBuffer* AmbisonicPanningNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const auto& listener = layer->GetListener();
        const auto& listenerInvMatrix = listener.GetInverseMatrix();

        _soundField.Reset();

        if (layer->IsMultiPosition())
        {
            const AmSize instanceCount = layer->GetInstanceCount();
            AmReal32 totalWeight = 0.0f;

            for (AmSize i = 0; i < instanceCount; ++i)
            {
                const AmVector3 location = layer->GetInstanceLocation(i);
                const AmReal32 weight = layer->GetInstanceWeight(i);
                const AmReal32 instanceGain = layer->GetInstanceGain(i);

                const auto& listenerSpacePosition = Transform(listenerInvMatrix, { .xyz = location, ._pad2 = 1.0f });
                _source.SetPosition(SphericalPosition::ForHRTF(listenerSpacePosition.xyz), 0.25f);

                // Reset temporary buffers
                _instanceSoundField.Reset();
                _instanceScaledInput.Clear();

                // Scale input by weight and gain for this instance
                const AmReal32 scaleFactor = weight * instanceGain;
                ScalarMultiply(input->GetChannel(0).begin(), _instanceScaledInput[0].begin(), scaleFactor, input->GetFrameCount());

                _source.Process(_instanceScaledInput.GetChannel(0), input->GetFrameCount(), &_instanceSoundField);

                // Accumulate into main soundfield
                _soundField += _instanceSoundField;
                totalWeight += weight;
            }

            // Normalize by total weight if needed
            if (totalWeight > kEpsilon && totalWeight != 1.0f)
            {
                const AmReal32 normFactor = 1.0f / totalWeight;
                _soundField *= normFactor;
            }
        }
        else
        {
            const auto& listenerSpaceSourcePosition = Transform(listenerInvMatrix, { .xyz = layer->GetLocation(), ._pad2 = 1.0f });
            _source.SetPosition(SphericalPosition::ForHRTF(listenerSpaceSourcePosition.xyz), 0.25f);
            _source.Process(input->GetChannel(0), input->GetFrameCount(), &_soundField);
        }

        return _soundField.GetBuffer();
    }

    AmbisonicPanningNode::AmbisonicPanningNode()
        : Node("AmbisonicPanning")
    {}
} // namespace SparkyStudios::Audio::Amplitude
