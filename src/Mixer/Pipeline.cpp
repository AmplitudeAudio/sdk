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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Mixer/Nodes/AmbisonicBinauralDecoderNode.h>
#include <Mixer/Nodes/AmbisonicMixerNode.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Mixer/Nodes/AttenuationNode.h>
#include <Mixer/Nodes/NearFieldEffectNode.h>
#include <Mixer/Nodes/OcclusionNode.h>
#include <Mixer/Nodes/StereoMixerNode.h>
#include <Mixer/Nodes/StereoPanningNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    Pipeline::Pipeline()
        : _processors()
    {
        _inputNode = ampoolnew(MemoryPoolKind::Amplimix, InputNodeInstance, 1, this);
        _nodeInstances[_inputNode->GetId()] = _inputNode;

        _outputNode = ampoolnew(MemoryPoolKind::Amplimix, OutputNodeInstance, 2, this);
        _nodeInstances[_outputNode->GetId()] = _outputNode;

        auto attenuationNode = ampoolnew(MemoryPoolKind::Amplimix, AttenuationNodeInstance, 3, this);
        _nodeInstances[attenuationNode->GetId()] = attenuationNode;

        auto occlusionNode = ampoolnew(MemoryPoolKind::Amplimix, OcclusionNodeInstance, 4, this);
        _nodeInstances[occlusionNode->GetId()] = occlusionNode;

        auto nearFieldEffectNode = ampoolnew(MemoryPoolKind::Amplimix, NearFieldEffectNodeInstance, 5, this);
        _nodeInstances[nearFieldEffectNode->GetId()] = nearFieldEffectNode;

        auto stereoPanningNode = ampoolnew(MemoryPoolKind::Amplimix, StereoPanningNodeInstance, 6, this);
        _nodeInstances[stereoPanningNode->GetId()] = stereoPanningNode;

        auto ambisonicPanningNode = ampoolnew(MemoryPoolKind::Amplimix, AmbisonicPanningNodeInstance, 7, this);
        _nodeInstances[ambisonicPanningNode->GetId()] = ambisonicPanningNode;

        auto ambisonicMixerNode = ampoolnew(MemoryPoolKind::Amplimix, AmbisonicMixerNodeInstance, 8, this);
        _nodeInstances[ambisonicMixerNode->GetId()] = ambisonicMixerNode;

        _nodes.push_back(ampoolnew(MemoryPoolKind::Amplimix, AmbisonicBinauralDecoderNode));
        auto ambisonicBinauralDecoder = (AmbisonicBinauralDecoderNodeInstance*)_nodes[0]->CreateInstance(9, this);
        _nodeInstances[ambisonicBinauralDecoder->GetId()] = ambisonicBinauralDecoder;

        auto stereoMixerNode = ampoolnew(MemoryPoolKind::Amplimix, StereoMixerNodeInstance, 10, this);
        _nodeInstances[stereoMixerNode->GetId()] = stereoMixerNode;

        // Direct Sound Path
        {
            attenuationNode->Connect(_inputNode->GetId());
            occlusionNode->Connect(attenuationNode->GetId());
            nearFieldEffectNode->Connect(occlusionNode->GetId());
            stereoMixerNode->Connect(nearFieldEffectNode->GetId());

            // Stereo Panning
            {
                stereoPanningNode->Connect(occlusionNode->GetId());
                stereoMixerNode->Connect(stereoPanningNode->GetId());
            }

            // HRTF Panning
            {
                ambisonicPanningNode->Connect(occlusionNode->GetId());
                ambisonicMixerNode->Connect(ambisonicPanningNode->GetId());
                ambisonicBinauralDecoder->Connect(ambisonicMixerNode->GetId());
                stereoMixerNode->Connect(ambisonicBinauralDecoder->GetId());
            }
        }

        _outputNode->Connect(stereoMixerNode->GetId());
    }

    Pipeline::~Pipeline()
    {
        for (const auto& processor : _processors)
            ampooldelete(MemoryPoolKind::Amplimix, SoundProcessorInstance, processor);

        _processors.clear();

        for (const auto& node : _nodeInstances)
            ampooldelete(MemoryPoolKind::Amplimix, NodeInstance, node.second);

        for (const auto& node : _nodes)
            ampooldelete(MemoryPoolKind::Amplimix, Node, node);
    }

    void Pipeline::Append(SoundProcessorInstance* processor)
    {
        _processors.push_back(processor);
    }

    void Pipeline::Insert(SoundProcessorInstance* processor, AmSize index)
    {
        if (index >= _processors.size())
            _processors.push_back(processor);
        else
            _processors.insert(_processors.begin() + index, processor);
    }

    void Pipeline::Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() == out.GetFrameCount());

        const AudioBuffer* cIn = &in;

        for (auto&& p : _processors)
        {
            p->Process(layer, *cIn, out);
            cIn = &out;
        }
    }

    void Pipeline::Cleanup(const AmplimixLayer* layer)
    {
        for (auto&& p : _processors)
            p->Cleanup(layer);
    }

    void Pipeline::Execute(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out)
    {
        // Update the layer for all nodes in the pipeline
        for (auto& node : _nodeInstances)
            node.second->_layer = layer;

        // Set the input and output buffers for the pipeline
        _inputNode->SetInput(&in);
        _outputNode->SetOutput(&out);

        // Consume data from the output node.
        // This will propagate the data from the input node to the output node,
        // executing all nodes in between.
        _outputNode->Consume();
    }

    InputNodeInstance* Pipeline::GetInputNode() const
    {
        return _inputNode;
    }

    OutputNodeInstance* Pipeline::GetOutputNode() const
    {
        return _outputNode;
    }

    NodeInstance* Pipeline::GetNode(AmObjectID id) const
    {
        if (_nodeInstances.find(id) != _nodeInstances.end())
            return _nodeInstances.at(id);

        return nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
