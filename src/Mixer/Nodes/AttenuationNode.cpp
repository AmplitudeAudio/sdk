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
#include <DSP/Gain.h>
#include <Mixer/Nodes/AttenuationNode.h>

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    AttenuationNodeInstance::AttenuationNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : ProcessorNodeInstance(id, pipeline)
    {}

    AudioBuffer AttenuationNodeInstance::Process(const AudioBuffer& input)
    {
        AudioBuffer output = input.Clone();

        const auto* layer = GetLayer();
        if (layer == nullptr)
            return output;

        const Attenuation* attenuation = layer->GetAttenuation();

        AmReal32 targetGain = 1.0f;

        if (attenuation != nullptr)
        {
            const eSpatialization spatialization = layer->GetSpatialization();
            const Listener& listener = layer->GetListener();

            if (listener.Valid())
            {
                if (spatialization != eSpatialization_None)
                {
                    const Entity& entity = layer->GetEntity();

                    if (spatialization == eSpatialization_PositionOrientation)
                    {
                        AMPLITUDE_ASSERT(entity.Valid());
                        targetGain *= attenuation->GetGain(entity, listener);
                    }
                    else if (spatialization == eSpatialization_HRTF && entity.Valid())
                    {
                        targetGain *= attenuation->GetGain(entity, listener);
                    }
                    else
                    {
                        const AmVec3& location = layer->GetLocation();

                        // Position-based spatialization, or HRTF-based spatialization without entity
                        targetGain *= attenuation->GetGain(location, listener);
                    }
                }
            }
            else
            {
                // No sound without listener on an attenuated source
                targetGain = 0.0f;
            }
        }

        if (Gain::IsZero(targetGain))
        {
            output.Clear();
            return output;
        }

        if (!Gain::IsOne(targetGain))
        {
            for (AmSize c = 0; c < output.GetChannelCount(); ++c)
                Gain::ApplyReplaceConstantGain(targetGain, input[c], 0, output[c], 0, input.GetFrameCount());
        }

        return output;
    }

    AttenuationNode::AttenuationNode()
        : Node("Attenuation")
    {}

    NodeInstance* AttenuationNode::CreateInstance(AmObjectID id, const Pipeline* pipeline) const
    {
        return ampoolnew(MemoryPoolKind::Amplimix, AttenuationNodeInstance, id, pipeline);
    }

    void AttenuationNode::DestroyInstance(NodeInstance* instance) const
    {
        ampooldelete(MemoryPoolKind::Amplimix, AttenuationNodeInstance, (AttenuationNodeInstance*)instance);
    }
} // namespace SparkyStudios::Audio::Amplitude
