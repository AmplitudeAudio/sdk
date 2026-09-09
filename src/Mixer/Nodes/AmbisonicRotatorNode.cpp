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

#include <SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h>

#include <Core/Engine.h>
#include <Mixer/Nodes/AmbisonicRotatorNode.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicRotatorNodeInstance::AmbisonicRotatorNodeInstance()
        : ProcessorNodeInstance()
    {
        const ePanningMode mode = amEngine->GetPanningMode();
        _ambisonicOrder = AM_MAX(static_cast<AmUInt32>(mode), 1u);

        _rotator.Configure(_ambisonicOrder, true);
    }

    bool AmbisonicRotatorNodeInstance::ShouldSkip() const
    {
        const auto* layer = GetLayer();
        const auto& listener = layer->GetListener();
        return !listener.Valid();
    }

    void AmbisonicRotatorNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        NodeInstance::Configure(frameCount, channelCount);

        _soundField.Configure(_ambisonicOrder, true, static_cast<AmUInt32>(frameCount));
    }

    AmUInt16 AmbisonicRotatorNodeInstance::GetOutputChannelCount() const
    {
        return static_cast<AmUInt16>(OrderToComponents(_ambisonicOrder, true));
    }

    const AudioBuffer* AmbisonicRotatorNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const auto& listener = layer->GetListener();

        // The orientation processor moves scene directions by the inverse of the supplied orientation,
        // and the SDK's ears-centered field (ForHRTF +90 deg azimuth) reverses the rotation sense,
        // so the inverse of the AmbiX-frame listener rotation is supplied.
        const CartesianCoordinateSystem::Converter engineToAmbiX(
            CartesianCoordinateSystem::Default(), CartesianCoordinateSystem::AmbiX());
        _rotator.SetOrientation(Orientation(Inverse(engineToAmbiX.Forward(listener.GetOrientation().GetQuaternion()))));

        _soundField.Reset();

        for (AmUInt32 i = 0, l = input->GetChannelCount(); i < l; ++i)
            _soundField.CopyStream(input->GetChannel(i), i, input->GetFrameCount());

        _rotator.Process(&_soundField, input->GetFrameCount());

        return _soundField.GetBuffer();
    }

    AmbisonicRotatorNode::AmbisonicRotatorNode()
        : Node("AmbisonicRotator")
    {}
} // namespace SparkyStudios::Audio::Amplitude
