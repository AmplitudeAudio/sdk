// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <queue>
#include <ranges>
#include <unordered_set>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <Mixer/Pipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    PipelineInstanceImpl::PipelineInstanceImpl(const Pipeline* parent, const AmplimixLayerImpl* layer)
        : _nodeInstances()
        , _inputNode(nullptr)
        , _outputNode(nullptr)
        , _layer(layer)
    {}

    PipelineInstanceImpl::~PipelineInstanceImpl()
    {
        for (auto& [name, node] : _nodeInstances | std::views::values)
            node = nullptr; // Clear the node instance

        _nodeInstances.clear();

        if (_outputNode != nullptr)
            _outputNode = nullptr;

        if (_inputNode != nullptr)
            _inputNode = nullptr;
    }

    void PipelineInstanceImpl::Execute(const AudioBuffer& in, AudioBuffer& out)
    {
        // Auto-configure if buffer dimensions changed
        Configure(in.GetFrameCount(), in.GetChannelCount(), out.GetFrameCount(), out.GetChannelCount());

        // Copy the input buffer content
        _inputBuffer = in;

        // Set the input and output buffers for the pipeline
        _inputNode->SetInput(&_inputBuffer);
        _outputNode->SetOutput(&out);

        // Consume data from the output node.
        // This will propagate the data from the input node to the output node,
        // executing all nodes in between.
        _outputNode->Consume();
    }

    std::shared_ptr<NodeInstance> PipelineInstanceImpl::GetNode(AmObjectID id) const
    {
        if (const auto it = _nodeInstances.find(id); it != _nodeInstances.end())
            return it->second.second;

        if (_inputNode != nullptr && _inputNode->GetId() == id)
            return _inputNode;

        if (_outputNode != nullptr && _outputNode->GetId() == id)
            return _outputNode;

        return nullptr;
    }

    void PipelineInstanceImpl::Reset()
    {
        _inputNode->Reset();

        for (const auto& [name, node] : _nodeInstances | std::views::values)
            node->Reset();

        _outputNode->Reset();
    }

    void PipelineInstanceImpl::AddNode(AmObjectID id, AmString nodeName, std::shared_ptr<NodeInstance> nodeInstance)
    {
        if (_nodeInstances.contains(id))
            return;

        _nodeInstances[id] = std::make_pair(nodeName, nodeInstance);
    }

    bool PipelineInstanceImpl::NeedsReconfiguration(AmUInt64 inFrames, AmUInt16 inChannels, AmUInt64 outFrames, AmUInt16 outChannels) const
    {
        return !_isConfigured || _configuredInputFrameCount != inFrames || _configuredInputChannelCount != inChannels ||
            _configuredOutputFrameCount != outFrames || _configuredOutputChannelCount != outChannels;
    }

    void PipelineInstanceImpl::Configure(
        AmUInt64 inputFrameCount, AmUInt16 inputChannelCount, AmUInt64 outputFrameCount, AmUInt16 outputChannelCount)
    {
        if (!NeedsReconfiguration(inputFrameCount, inputChannelCount, outputFrameCount, outputChannelCount))
            return;

        _configuredInputFrameCount = inputFrameCount;
        _configuredInputChannelCount = inputChannelCount;
        _configuredOutputFrameCount = outputFrameCount;
        _configuredOutputChannelCount = outputChannelCount;

        ConfigureNodeGraph();

        _isConfigured = true;
    }

    void PipelineInstanceImpl::ConfigureNodeGraph()
    {
        // Configure input node first
        _inputNode->Configure(_configuredInputFrameCount, _configuredInputChannelCount);

        // Build a map of node ID to its consumers for topological traversal
        std::unordered_map<AmObjectID, std::vector<AmObjectID>> nodeConsumers;
        std::unordered_map<AmObjectID, AmObjectID> nodeProvider; // For single-input processors
        std::unordered_map<AmObjectID, std::vector<AmObjectID>> nodeProviders; // For multi-input mixers

        // Collect connection information
        for (const auto& [id, pair] : _nodeInstances)
        {
            const auto& node = pair.second;

            if (const auto* processor = dynamic_cast<ProcessorNodeInstance*>(node.get()))
            {
                nodeProvider[id] = processor->GetProvider();
                nodeConsumers[processor->GetProvider()].push_back(id);
            }
            else if (const auto* mixer = dynamic_cast<MixerNodeInstance*>(node.get()))
            {
                nodeProviders[id] = mixer->GetProviders();
                for (const auto& providerId : mixer->GetProviders())
                    nodeConsumers[providerId].push_back(id);
            }
        }

        // Use BFS to configure nodes in topological order (from input to output)
        std::unordered_set<AmObjectID> configured;
        std::queue<AmObjectID> toProcess;

        // Start with input node (ID from _inputNode)
        const AmObjectID inputNodeId = _inputNode->GetId();
        configured.insert(inputNodeId);

        // Add all nodes that consume from input node
        for (const auto& consumerId : nodeConsumers[inputNodeId])
            toProcess.push(consumerId);

        while (!toProcess.empty())
        {
            const AmObjectID currentId = toProcess.front();
            toProcess.pop();

            if (configured.contains(currentId))
                continue;

            // Get the node
            auto nodeIt = _nodeInstances.find(currentId);
            if (nodeIt == _nodeInstances.end())
                continue;

            const auto& node = nodeIt->second.second;

            // Determine input dimensions from provider(s)
            AmUInt64 inputFrames = 0;
            AmUInt16 inputChannels = 0;

            if (const auto* processor = dynamic_cast<ProcessorNodeInstance*>(node.get()))
            {
                // Single provider - get its output dimensions
                const AmObjectID providerId = processor->GetProvider();
                if (configured.contains(providerId))
                {
                    auto providerNode = GetNode(providerId);

                    inputFrames = providerNode->GetOutputFrameCount();
                    inputChannels = providerNode->GetOutputChannelCount();
                }
            }
            else if (const auto* mixer = dynamic_cast<MixerNodeInstance*>(node.get()))
            {
                // Multiple providers - use first non-empty provider's dimensions
                // (all providers should have matching dimensions for mixing)
                for (const auto& providerId : mixer->GetProviders())
                {
                    if (configured.contains(providerId))
                    {
                        auto providerNode = GetNode(providerId);

                        inputFrames = providerNode->GetOutputFrameCount();
                        inputChannels = providerNode->GetOutputChannelCount();
                        break;
                    }
                }
            }

            // Configure this node
            node->Configure(inputFrames, inputChannels);
            configured.insert(currentId);

            // Add consumers of this node to the queue
            for (const auto& consumerId : nodeConsumers[currentId])
                if (!configured.contains(consumerId))
                    toProcess.push(consumerId);
        }

        // Configure output node last
        if (_outputNode != nullptr)
            _outputNode->Configure(_configuredOutputFrameCount, _configuredOutputChannelCount);
    }

    PipelineImpl::~PipelineImpl()
    {}

    std::shared_ptr<PipelineInstance> PipelineImpl::CreateInstance(const AmplimixLayer* layer) const
    {
        auto instance = ampoolshared(eMemoryPoolKind_Amplimix, PipelineInstanceImpl, this, static_cast<const AmplimixLayerImpl*>(layer));

        const auto* definition = GetDefinition();
        const auto* nodes = definition->nodes();

        // Create node instances based on the pipeline definition
        for (flatbuffers::uoffset_t i = 0, l = nodes->size(); i < l; ++i)
        {
            const auto* nodeDef = nodes->Get(i);
            const auto& nodeName = nodeDef->name()->str();
            const auto& nodeId = nodeDef->id();
            const auto* inputs = nodeDef->consume();

            auto node = Node::Find(nodeName);
            std::shared_ptr<NodeInstance> nodeInstance = nullptr;

            if (nodeName == "Input")
            {
                if (instance->_inputNode != nullptr)
                {
                    amLogError("More than one input node was found in the pipeline.");
                    return nullptr;
                }

                nodeInstance = node->CreateInstance();
                instance->_inputNode = std::static_pointer_cast<InputNodeInstance>(nodeInstance);
            }
            else if (nodeName == "Output")
            {
                if (instance->_outputNode != nullptr)
                {
                    amLogError("More than one output node was found in the pipeline.");
                    return nullptr;
                }

                nodeInstance = node->CreateInstance();
                instance->_outputNode = std::static_pointer_cast<OutputNodeInstance>(nodeInstance);
            }
            else
            {
                if (node != nullptr)
                    nodeInstance = node->CreateInstance();

                if (nodeInstance == nullptr)
                {
                    amLogError(
                        "Pipeline node not found: %s. Make sure it is registered. If the node is provided by a plugin, make sure to load "
                        "the "
                        "plugin before Amplitude.",
                        nodeName.c_str());
                    return nullptr;
                }

                instance->AddNode(nodeId, nodeName, nodeInstance);
            }

            const flatbuffers::uoffset_t paramCount = nodeDef->parameters() ? nodeDef->parameters()->size() : 0;

            // Initialize the node with the provided parameters
            nodeInstance->Initialize(nodeId, layer, instance.get(), paramCount);

            // Initialize the node parameters
            for (flatbuffers::uoffset_t i = 0; i < paramCount; ++i)
                nodeInstance->SetParameter(i, nodeDef->parameters()->Get(i));

            // Connect the node inputs
            if (node->CanConsume())
            {
                auto consumerNode = std::dynamic_pointer_cast<ConsumerNodeInstance>(nodeInstance);
                if (consumerNode == nullptr)
                {
                    amLogError(
                        "The node '%s' can consume, but it doesn't inherits ConsumerNodeInstance. This is a programming error.",
                        nodeName.c_str());
                    return nullptr;
                }

                if (!AM_BETWEEN(inputs->size(), node->GetMinInputCount(), node->GetMaxInputCount()))
                {
                    amLogError(
                        "The node '%s' requires %zu to %zu input(s), but %d were provided.", nodeName.c_str(), node->GetMinInputCount(),
                        node->GetMaxInputCount(), inputs->size());
                    return nullptr;
                }

                std::vector<AmObjectID> connectedNodes;
                for (flatbuffers::uoffset_t j = 0, m = inputs->size(); j < m; ++j)
                {
                    const AmObjectID producerNodeId = inputs->Get(j);

                    if (producerNodeId == nodeId)
                    {
                        amLogError("A node cannot consume itself: %s", nodeName.c_str());
                        return nullptr;
                    }

                    if (std::ranges::find(connectedNodes, producerNodeId) != connectedNodes.end())
                    {
                        amLogWarning(
                            "The node with ID '" AM_ID_CHAR_FMT "' is already connected to %s, skipping.", producerNodeId,
                            nodeName.c_str());
                        continue;
                    }

                    consumerNode->Connect(producerNodeId);
                    connectedNodes.push_back(producerNodeId);
                }
            }
        }

        if (instance->_inputNode == nullptr || instance->_outputNode == nullptr)
        {
            amLogError("The pipeline must have an input and an output node.");
            return nullptr;
        }

        return instance;
    }

    bool PipelineImpl::LoadDefinition(const PipelineDefinition* definition, std::shared_ptr<EngineInternalState> state)
    {
        m_id = definition->id();
        m_name = definition->name()->str();

        return true;
    }

    const PipelineDefinition* PipelineImpl::GetDefinition() const
    {
        return GetPipelineDefinition(m_source.c_str());
    }
} // namespace SparkyStudios::Audio::Amplitude
