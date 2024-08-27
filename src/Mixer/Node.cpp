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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    NodeInstance::NodeInstance(AmObjectID id)
        : m_id(id)
    {}

    AmObjectID NodeInstance::GetId() const
    {
        return m_id;
    }

    const AmplimixLayer* NodeInstance::GetLayer() const
    {
        return _layer;
    }

    ProcessorNodeInstance::ProcessorNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : NodeInstance(id)
        , m_pipeline(std::move(pipeline))
    {}

    void ProcessorNodeInstance::Consume()
    {
        AMPLITUDE_ASSERT(m_provider != kAmInvalidObjectId);

        NodeInstance* node = m_pipeline->GetNode(m_provider);
        AMPLITUDE_ASSERT(node != nullptr);

        ProviderNodeInstance* provider = dynamic_cast<ProviderNodeInstance*>(node);
        AMPLITUDE_ASSERT(provider != nullptr);

        _processingBuffer = provider->Provide();
    }

    void ProcessorNodeInstance::Connect(AmObjectID provider)
    {
        m_provider = provider;
    }

    AudioBuffer ProcessorNodeInstance::Provide()
    {
        Consume();
        return Process(_processingBuffer);
    }

    MixerNodeInstance::MixerNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : NodeInstance(id)
        , m_pipeline(pipeline)
    {}

    void MixerNodeInstance::Consume()
    {
        if (m_providers.empty())
            return;

        _processingBuffers.clear();
        for (const auto& providerId : m_providers)
        {
            AMPLITUDE_ASSERT(providerId != kAmInvalidObjectId);

            NodeInstance* node = m_pipeline->GetNode(providerId);
            AMPLITUDE_ASSERT(node != nullptr);

            ProviderNodeInstance* provider = dynamic_cast<ProviderNodeInstance*>(node);
            AMPLITUDE_ASSERT(provider != nullptr);

            _processingBuffers.push_back(provider->Provide());
        }
    }

    void MixerNodeInstance::Connect(AmObjectID provider)
    {
        m_providers.push_back(provider);
    }

    void MixerNodeInstance::In(const std::vector<AmObjectID>& providers)
    {
        m_providers = providers;
    }

    AudioBuffer MixerNodeInstance::Provide()
    {
        Consume();
        return Mix(_processingBuffers);
    }

    InputNodeInstance::InputNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : NodeInstance(id)
    {}

    void InputNodeInstance::SetInput(const AudioBuffer* buffer)
    {
        _buffer = buffer;
    }

    AudioBuffer InputNodeInstance::Provide()
    {
        AudioBuffer temp;
        temp = *_buffer;

        return std::move(temp);
    }

    OutputNodeInstance::OutputNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : NodeInstance(id)
        , _pipeline(pipeline)
    {}

    void OutputNodeInstance::SetOutput(AudioBuffer* buffer)
    {
        _buffer = buffer;
    }

    void OutputNodeInstance::Consume()
    {
        AMPLITUDE_ASSERT(_provider != kAmInvalidObjectId);

        NodeInstance* node = _pipeline->GetNode(_provider);
        AMPLITUDE_ASSERT(node != nullptr);

        ProviderNodeInstance* provider = dynamic_cast<ProviderNodeInstance*>(node);
        AMPLITUDE_ASSERT(provider != nullptr);

        *_buffer = provider->Provide();
    }

    void OutputNodeInstance::Connect(AmObjectID provider)
    {
        _provider = provider;
    }

    Node::Node(AmString name)
        : _name(std::move(name))
    {}

    const AmString& Node::GetName() const
    {
        return _name;
    }
} // namespace SparkyStudios::Audio::Amplitude