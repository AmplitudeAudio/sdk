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

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicPanningNodeInstance::AmbisonicPanningNodeInstance()
    {
        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        _source.Configure(order, true);
    }

    const AudioBuffer* AmbisonicPanningNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        if (const eSpatialization spatialization = layer->GetSpatialization(); spatialization != eSpatialization_HRTF)
            return nullptr;

        const auto& listener = layer->GetListener();
        if (!listener.Valid())
            return nullptr;

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        _soundField.Configure(order, true, input->GetFrameCount());
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

                const auto& listenerSpacePosition = Transform(listener.GetInverseMatrix(), { .xyz = location, ._pad2 = 1.0f });
                _source.SetPosition(SphericalPosition::ForHRTF(listenerSpacePosition.xyz), 0.25f);

                // Create a temporary soundfield for this instance
                BFormat tempField;
                tempField.Configure(order, true, input->GetFrameCount());
                tempField.Reset();

                // Scale input by weight and gain for this instance
                AudioBuffer scaledInput(input->GetFrameCount(), 1);
                const AmReal32 scaleFactor = weight * instanceGain;
                ScalarMultiply(input->GetChannel(0).begin(), scaledInput[0].begin(), scaleFactor, input->GetFrameCount());

                _source.Process(scaledInput.GetChannel(0), input->GetFrameCount(), &tempField);

                // Accumulate into main soundfield
                _soundField += tempField;
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
            const auto& listenerSpaceSourcePosition = Transform(listener.GetInverseMatrix(), { .xyz = layer->GetLocation(), ._pad2 = 1.0f });
            _source.SetPosition(SphericalPosition::ForHRTF(listenerSpaceSourcePosition.xyz), 0.25f);
            _source.Process(input->GetChannel(0), input->GetFrameCount(), &_soundField);
        }

        return _soundField.GetBuffer();
    }

    AmbisonicPanningNode::AmbisonicPanningNode()
        : Node("AmbisonicPanning")
    {}
} // namespace SparkyStudios::Audio::Amplitude
