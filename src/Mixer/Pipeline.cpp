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

    void PipelineInstanceImpl::Execute(AudioBuffer& in, AudioBuffer& out)
    {
        // Configured on the game thread beforehand; this only re-runs when the caller's dimensions change.
        if (!Configure(in.GetFrameCount(), in.GetChannelCount(), out.GetFrameCount(), out.GetChannelCount()))
        {
            out.Clear();
            return;
        }

        _inputNode->SetInput(&in);
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
        return !_hasConfiguration || _configuredInputFrameCount != inFrames || _configuredInputChannelCount != inChannels ||
            _configuredOutputFrameCount != outFrames || _configuredOutputChannelCount != outChannels;
    }

    bool PipelineInstanceImpl::Configure(
        AmUInt64 inputFrameCount, AmUInt16 inputChannelCount, AmUInt64 outputFrameCount, AmUInt16 outputChannelCount)
    {
        if (!NeedsReconfiguration(inputFrameCount, inputChannelCount, outputFrameCount, outputChannelCount))
            return _isConfigured;

        _configuredInputFrameCount = inputFrameCount;
        _configuredInputChannelCount = inputChannelCount;
        _configuredOutputFrameCount = outputFrameCount;
        _configuredOutputChannelCount = outputChannelCount;
        _hasConfiguration = true;

        _isConfigured = ConfigureNodeGraph();
        return _isConfigured;
    }

    bool PipelineInstanceImpl::ConfigureNodeGraph()
    {
        const ChannelLayout* outputLayout = ChannelLayout::FromChannelCount(_configuredOutputChannelCount);
        if (outputLayout == nullptr)
        {
            amLogError(
                "A pipeline cannot render %u channels: the output must be mono, stereo, quad, 5.1 or 7.1.",
                static_cast<AmUInt32>(_configuredOutputChannelCount));
            return false;
        }

        _inputNode->SetPipelineOutputLayout(outputLayout->GetType());
        _inputNode->Configure(_configuredInputFrameCount, _configuredInputChannelCount);

        // Build a map of node ID to its consumers for topological traversal
        std::unordered_map<AmObjectID, std::vector<AmObjectID>> nodeConsumers;
        for (const auto& [id, pair] : _nodeInstances)
        {
            const auto& node = pair.second;

            if (const auto* processor = dynamic_cast<ProcessorNodeInstance*>(node.get()))
                nodeConsumers[processor->GetProvider()].push_back(id);
            else if (const auto* mixer = dynamic_cast<MixerNodeInstance*>(node.get()))
                for (const auto& providerId : mixer->GetProviders())
                    nodeConsumers[providerId].push_back(id);
        }

        std::unordered_set<AmObjectID> configured;
        std::queue<AmObjectID> toProcess;

        // Nodes whose output still carries the sound's own channels: no Custom node upstream.
        std::unordered_set<AmObjectID> sourceLayout;

        const AmObjectID inputNodeId = _inputNode->GetId();
        configured.insert(inputNodeId);
        sourceLayout.insert(inputNodeId);

        for (const auto& consumerId : nodeConsumers[inputNodeId])
            toProcess.push(consumerId);

        while (!toProcess.empty())
        {
            const AmObjectID currentId = toProcess.front();
            toProcess.pop();

            if (configured.contains(currentId))
                continue;

            auto nodeIt = _nodeInstances.find(currentId);
            if (nodeIt == _nodeInstances.end())
                continue;

            const auto& node = nodeIt->second.second;
            const auto policyIt = _policies.find(currentId);
            const eNodeChannelPolicy policy = policyIt != _policies.end() ? policyIt->second : eNodeChannelPolicy_Mono;

            node->SetPipelineOutputLayout(outputLayout->GetType());

            if (auto* processor = dynamic_cast<ProcessorNodeInstance*>(node.get()))
            {
                const AmObjectID providerId = processor->GetProvider();
                const auto provider = GetNode(providerId);

                const AmUInt64 inputFrames = provider->GetOutputFrameCount();
                AmUInt16 inputChannels = provider->GetOutputChannelCount();
                const bool fromSource = sourceLayout.contains(providerId);

                if (policy == eNodeChannelPolicy_Mono && fromSource && inputChannels > 1)
                {
                    if (!processor->EnableFoldDown(inputFrames, inputChannels))
                    {
                        amLogError(
                            "Cannot fold %u channels down to mono for node " AM_ID_CHAR_FMT ": no channel layout has that many channels.",
                            static_cast<AmUInt32>(inputChannels), currentId);
                        return false;
                    }

                    inputChannels = 1;
                }
                else
                {
                    processor->DisableFoldDown();
                }

                node->Configure(inputFrames, inputChannels);

                if (fromSource && policy != eNodeChannelPolicy_Custom)
                    sourceLayout.insert(currentId);
            }
            else if (const auto* mixer = dynamic_cast<MixerNodeInstance*>(node.get()))
            {
                // A mixer sums its providers sample by sample: it waits for all of them, and they must agree.
                AmUInt64 inputFrames = 0;
                AmUInt16 inputChannels = 0;
                bool ready = true;
                bool allFromSource = true;

                for (const auto& providerId : mixer->GetProviders())
                {
                    if (!configured.contains(providerId))
                    {
                        ready = false;
                        break;
                    }

                    const auto provider = GetNode(providerId);
                    if (inputChannels == 0)
                    {
                        inputFrames = provider->GetOutputFrameCount();
                        inputChannels = provider->GetOutputChannelCount();
                    }
                    else if (provider->GetOutputChannelCount() != inputChannels)
                    {
                        amLogError(
                            "The mixer node " AM_ID_CHAR_FMT " sums providers of %u and %u channels: its providers must produce the same "
                            "layout.",
                            currentId, static_cast<AmUInt32>(inputChannels), static_cast<AmUInt32>(provider->GetOutputChannelCount()));
                        return false;
                    }

                    allFromSource &= sourceLayout.contains(providerId);
                }

                // Pushed again when its next provider is configured.
                if (!ready)
                    continue;

                node->Configure(inputFrames, inputChannels);

                if (allFromSource)
                    sourceLayout.insert(currentId);
            }

            configured.insert(currentId);

            for (const auto& consumerId : nodeConsumers[currentId])
                if (!configured.contains(consumerId))
                    toProcess.push(consumerId);
        }

        // The last node must produce what the caller's output buffer holds.
        const AmObjectID lastId = _outputNode->GetProvider();
        const auto last = GetNode(lastId);
        if (last == nullptr || !configured.contains(lastId))
        {
            amLogError("The pipeline's output node is not reachable from its input node.");
            return false;
        }

        if (last->GetOutputChannelCount() != _configuredOutputChannelCount)
        {
            amLogError(
                "The pipeline produces %u channels but its output takes %u: pan or mix to the output layout before the output node.",
                static_cast<AmUInt32>(last->GetOutputChannelCount()), static_cast<AmUInt32>(_configuredOutputChannelCount));
            return false;
        }

        _outputNode->SetPipelineOutputLayout(outputLayout->GetType());
        _outputNode->Configure(_configuredOutputFrameCount, _configuredOutputChannelCount);
        return true;
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
                instance->_policies[nodeId] = node->GetChannelPolicy();
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
