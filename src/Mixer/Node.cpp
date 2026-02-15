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

#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Sound/Effect.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<std::string, std::shared_ptr<Node>> NodeRegistry;
    typedef NodeRegistry::value_type NodeImpl;

    static NodeRegistry& nodeRegistry()
    {
        static NodeRegistry r;
        return r;
    }

    static bool& lockNodes()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& nodesCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    void NodeInstance::Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline, AmSize paramCount)
    {
        AMPLITUDE_ASSERT(layer != nullptr);

        m_id = id;
        m_layer = layer;
        m_pipeline = pipeline;

        m_parameters.clear();
        m_parameters.assign(paramCount, 0);
    }

    AmObjectID NodeInstance::GetId() const
    {
        return m_id;
    }

    const AmplimixLayer* NodeInstance::GetLayer() const
    {
        return m_layer;
    }

    AmReal32 NodeInstance::GetParameter(AmSize index)
    {
        if (index >= m_parameters.size())
            return 0;

        return m_parameters[index];
    }

    void NodeInstance::SetParameter(AmSize index, AmReal32 value)
    {
        if (index >= m_parameters.size())
            return;

        if (m_parameters[index] == value)
            return;

        m_parameters[index] = value;
        m_numParamsChanged |= 1 << index;
    }

    void NodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        m_inputFrameCount = frameCount;
        m_inputChannelCount = channelCount;
    }

    AmUInt64 NodeInstance::GetOutputFrameCount() const
    {
        return m_inputFrameCount;
    }

    AmUInt16 NodeInstance::GetOutputChannelCount() const
    {
        return m_inputChannelCount;
    }

    bool NodeInstance::ShouldSkip() const
    {
        return false;
    }

    ProcessorNodeInstance::ProcessorNodeInstance(bool processOnEmptyInputBuffer)
        : m_provider(0)
        , _processingBuffer(nullptr)
        , _lastOutputBuffer(nullptr)
        , _processOnEmptyInputBuffer(processOnEmptyInputBuffer)
        , _cachedProvider(nullptr)
    {}

    void ProcessorNodeInstance::Consume()
    {
        if (m_layer == nullptr)
            return;

        AMPLITUDE_ASSERT(m_provider != kAmInvalidObjectId);

        if (_cachedProvider == nullptr)
        {
            const auto node = m_pipeline->GetNode(m_provider);
            AMPLITUDE_ASSERT(node != nullptr);

            _cachedProvider = dynamic_cast<ProviderNodeInstance*>(node.get());
            AMPLITUDE_ASSERT(_cachedProvider != nullptr);
        }

        _processingBuffer = _cachedProvider->Provide();
    }

    void ProcessorNodeInstance::Connect(AmObjectID provider)
    {
        m_provider = provider;
        _cachedProvider = nullptr;
    }

    const AudioBuffer* ProcessorNodeInstance::Provide()
    {
        if (m_layer == nullptr)
            return nullptr;

        if (_lastOutputBuffer != nullptr)
            return _lastOutputBuffer;

        // Check if node should be skipped, so we avoid triggering upstream processing in dead paths.
        if (ShouldSkip())
            return nullptr;

        if (_processingBuffer == nullptr)
            Consume();

        if (_processingBuffer == nullptr && !_processOnEmptyInputBuffer)
            return nullptr;

        return _lastOutputBuffer = Process(_processingBuffer);
    }

    void ProcessorNodeInstance::Reset()
    {
        _processingBuffer = nullptr;
        _lastOutputBuffer = nullptr;
    }

    void ProcessorNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        NodeInstance::Configure(frameCount, channelCount);

        const AmUInt64 outFrames = GetOutputFrameCount();
        const AmUInt16 outChannels = GetOutputChannelCount();

        if (_output.IsEmpty() || _output.GetFrameCount() != outFrames || _output.GetChannelCount() != outChannels)
            _output = AudioBuffer(outFrames, outChannels);
    }

    MixerNodeInstance::MixerNodeInstance()
        : _processingBuffers()
        , _mixBuffer()
        , _processed(false)
        , _cachedProviders()
    {}

    void MixerNodeInstance::Consume()
    {
        if (m_layer == nullptr)
            return;

        if (m_providers.empty())
            return;

        if (_cachedProviders.empty())
        {
            _cachedProviders.reserve(m_providers.size());
            for (const auto& providerId : m_providers)
            {
                AMPLITUDE_ASSERT(providerId != kAmInvalidObjectId);

                auto node = m_pipeline->GetNode(providerId);
                AMPLITUDE_ASSERT(node != nullptr);

                auto* provider = dynamic_cast<ProviderNodeInstance*>(node.get());
                AMPLITUDE_ASSERT(provider != nullptr);

                _cachedProviders.push_back(provider);
            }
        }

        _processingBuffers.clear();
        for (auto* provider : _cachedProviders)
            _processingBuffers.push_back(provider->Provide());
    }

    void MixerNodeInstance::Connect(AmObjectID provider)
    {
        m_providers.push_back(provider);
        _cachedProviders.clear();
    }

    void MixerNodeInstance::Connect(const std::vector<AmObjectID>& providers)
    {
        m_providers = providers;
        _cachedProviders.clear();
    }

    const AudioBuffer* MixerNodeInstance::Provide()
    {
        if (m_layer == nullptr)
            return nullptr;

        if (_processed)
            return &_mixBuffer;

        if (_processingBuffers.empty())
            Consume();

        if (_processingBuffers.empty())
            return nullptr;

        auto buffers = _processingBuffers |
            std::ranges::views::filter(
                           [](const AudioBuffer* buffer)
                           {
                               return buffer != nullptr && !buffer->IsEmpty();
                           });

        if (buffers.empty())
            return nullptr;

        const auto& first = buffers.begin();
        _mixBuffer.Clear();

        for (const auto& input : buffers)
        {
            AMPLITUDE_ASSERT(input->GetFrameCount() == _mixBuffer.GetFrameCount());
            AMPLITUDE_ASSERT(input->GetChannelCount() == _mixBuffer.GetChannelCount());

            _mixBuffer += *input;
        }

        _processed = true;
        return &_mixBuffer;
    }

    void MixerNodeInstance::Reset()
    {
        _processingBuffers.clear();
        _mixBuffer.Clear();

        _processed = false;
    }

    void MixerNodeInstance::Configure(AmUInt64 frameCount, AmUInt16 channelCount)
    {
        NodeInstance::Configure(frameCount, channelCount);

        const AmUInt64 outFrames = GetOutputFrameCount();
        const AmUInt16 outChannels = GetOutputChannelCount();

        if (_mixBuffer.IsEmpty() || _mixBuffer.GetFrameCount() != outFrames || _mixBuffer.GetChannelCount() != outChannels)
            _mixBuffer = AudioBuffer(outFrames, outChannels);
    }

    InputNodeInstance::InputNodeInstance()
        : _buffer(nullptr)
        , _filter(nullptr)
    {}

    void InputNodeInstance::SetInput(AudioBuffer* buffer)
    {
        _buffer = buffer;

        if (const auto effect = GetLayer()->GetEffect(); effect != nullptr)
            _filter = effect->GetFilter();
    }

    const AudioBuffer* InputNodeInstance::Provide()
    {
        if (_buffer == nullptr)
            return nullptr;

        if (_filter != nullptr)
            _filter->Process(*_buffer, *_buffer, _buffer->GetFrameCount(), GetLayer()->GetSampleRate());

        return _buffer;
    }

    void InputNodeInstance::Reset()
    {
        _buffer = nullptr;
        _filter = nullptr;
    }

    OutputNodeInstance::OutputNodeInstance()
        : _provider(0)
        , _buffer(nullptr)
        , _cachedProvider(nullptr)
    {}

    void OutputNodeInstance::SetOutput(AudioBuffer* buffer)
    {
        _buffer = buffer;
    }

    void OutputNodeInstance::Connect(AmObjectID provider)
    {
        _provider = provider;
        _cachedProvider = nullptr;
    }

    void OutputNodeInstance::Consume()
    {
        if (m_layer == nullptr)
            return;

        if (_buffer == nullptr)
            return;

        AMPLITUDE_ASSERT(_provider != kAmInvalidObjectId);

        if (_cachedProvider == nullptr)
        {
            auto node = m_pipeline->GetNode(_provider);
            AMPLITUDE_ASSERT(node != nullptr);

            _cachedProvider = dynamic_cast<ProviderNodeInstance*>(node.get());
            AMPLITUDE_ASSERT(_cachedProvider != nullptr);
        }

        const auto* output = _cachedProvider->Provide();
        if (output == nullptr)
            return;

        *_buffer = *output;
    }

    void OutputNodeInstance::Reset()
    {
        _buffer = nullptr;
    }

    Node::Node(AmString name)
        : m_name(std::move(name))
    {}

    Node::~Node()
    {}

    const AmString& Node::GetName() const
    {
        return m_name;
    }

    AmSize Node::GetParameterCount() const
    {
        return 0;
    }

    AmString Node::GetParameterName(AmSize index) const
    {
        return "";
    }

    eParameterType Node::GetParameterType(AmSize index) const
    {
        return eParameterType_Float;
    }

    AmReal32 Node::GetParameterMax(AmSize index) const
    {
        return 0;
    }

    AmReal32 Node::GetParameterMin(AmSize index) const
    {
        return 0;
    }

    void Node::Register(std::shared_ptr<Node> node)
    {
        if (lockNodes() || node == nullptr)
            return;

        if (Find(node->GetName()) != nullptr)
        {
            amLogWarning("Failed to register node '%s' as it is already registered", node->GetName().c_str());
            return;
        }

        NodeRegistry& nodes = nodeRegistry();
        nodes.insert(NodeImpl(node->GetName(), node));
        nodesCount()++;
    }

    void Node::Unregister(std::shared_ptr<const Node> node)
    {
        if (lockNodes() || node == nullptr)
            return;

        NodeRegistry& nodes = nodeRegistry();
        if (const auto& it = nodes.find(node->GetName()); it != nodes.end())
        {
            nodes.erase(it);
            nodesCount()--;
        }
    }

    std::shared_ptr<Node> Node::Find(const std::string& name)
    {
        NodeRegistry& nodes = nodeRegistry();
        if (const auto& it = nodes.find(name); it != nodes.end())
            return it->second;

        return nullptr;
    }

    std::shared_ptr<NodeInstance> Node::Construct(const std::string& name)
    {
        std::shared_ptr<Node> node = Find(name);
        if (node == nullptr)
            return nullptr;

        return node->CreateInstance();
    }

    void Node::LockRegistry()
    {
        lockNodes() = true;
    }

    void Node::UnlockRegistry()
    {
        lockNodes() = false;
    }

    const std::map<AmString, std::shared_ptr<Node>>& Node::GetRegistry()
    {
        return nodeRegistry();
    }
} // namespace SparkyStudios::Audio::Amplitude
