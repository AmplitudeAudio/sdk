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
#include <Mixer/Nodes/AmbisonicRotatorNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicRotatorNodeInstance::AmbisonicRotatorNodeInstance()
        : ProcessorNodeInstance()
    {
        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        _rotator.Configure(order, true);
    }

    const AudioBuffer* AmbisonicRotatorNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        if (const eSpatialization spatialization = layer->GetSpatialization(); spatialization != eSpatialization_HRTF)
            return nullptr;

        const auto& listener = layer->GetListener();
        if (!listener.Valid())
            return nullptr;

        const AmQuat listenerRotation = listener.GetOrientation().GetQuaternion();
        const AmQuat inverseListenerRotation = AM_InvQ(listenerRotation);

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();
        const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        _soundField.Configure(order, true, input->GetFrameCount());

        for (AmUInt32 i = 0, l = input->GetChannelCount(); i < l; ++i)
            _soundField.CopyStream(input->GetChannel(i), i, input->GetFrameCount());

        _rotator.SetOrientation(Orientation(inverseListenerRotation));
        _rotator.Process(&_soundField, input->GetFrameCount());

        return _soundField.GetBuffer();
    }

    AmbisonicRotatorNode::AmbisonicRotatorNode()
        : Node("AmbisonicRotator")
    {}
} // namespace SparkyStudios::Audio::Amplitude
