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

#include <Ambisonics/BFormat.h>
#include <Core/EngineInternalState.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>

#include "Ambisonics/AmbisonicOrientationProcessor.h"
#include "engine_config_definition_generated.h"
#include "SparkyStudios/Audio/Amplitude/Math/Orientation.h"

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicPanningNodeInstance::AmbisonicPanningNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : ProcessorNodeInstance(id, pipeline)
    {}

    AudioBuffer AmbisonicPanningNodeInstance::Process(const AudioBuffer& input)
    {
        const auto* layer = GetLayer();
        if (layer == nullptr)
            return {};

        const auto& listener = layer->GetListener();
        if (!listener.Valid())
            return {};

        const auto& listenerSpaceSourcePosition = listener.GetInverseMatrix() * AM_V4V(layer->GetLocation(), 1.0f);

        const ePanningMode mode = Engine::GetInstance()->GetPanningMode();

        BFormat encodedOutput;
        encodedOutput.Configure(mode, true, input.GetFrameCount());

        if (!_sources.contains(layer->GetId()))
        {
            AmbisonicSource& source = _sources[layer->GetId()];
            source.Configure(mode, true);
        }

        AmbisonicSource& source = _sources[layer->GetId()];
        source.SetPosition(SphericalPosition::ForHRTF(listenerSpaceSourcePosition.XYZ), 0.25f);
        source.Process(input[0], input.GetFrameCount(), &encodedOutput);

        return encodedOutput.GetBuffer()->Clone();
    }

    AmbisonicPanningNode::AmbisonicPanningNode()
        : Node("AmbisonicPanning")
    {}
} // namespace SparkyStudios::Audio::Amplitude
