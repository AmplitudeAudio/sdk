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

        const auto& listenerSpaceSourcePosition = listener.GetInverseMatrix() * AM_V4V(layer->GetLocation(), 1.0f);

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        _soundField.Configure(order, true, input->GetFrameCount());

        _source.SetPosition(SphericalPosition::ForHRTF(listenerSpaceSourcePosition.XYZ), 0.25f);
        _source.Process(input->GetChannel(0), input->GetFrameCount(), &_soundField);

        return _soundField.GetBuffer();
    }

    AmbisonicPanningNode::AmbisonicPanningNode()
        : Node("AmbisonicPanning")
    {}
} // namespace SparkyStudios::Audio::Amplitude
