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

namespace SparkyStudios::Audio::Amplitude
{
    Pipeline::Pipeline()
        : _processors()
    {
        _inputNode = ampoolnew(MemoryPoolKind::Amplimix, InputNodeInstance, 1, this);
        _outputNode = ampoolnew(MemoryPoolKind::Amplimix, OutputNodeInstance, 2, this);

        _outputNode->Connect(_inputNode->GetId());

        _nodes[_inputNode->GetId()] = _inputNode;
        _nodes[_outputNode->GetId()] = _outputNode;
    }

    Pipeline::~Pipeline()
    {
        for (const auto& processor : _processors)
            ampooldelete(MemoryPoolKind::Amplimix, SoundProcessorInstance, processor);

        _processors.clear();

        ampooldelete(MemoryPoolKind::Amplimix, InputNodeInstance, _inputNode);
        ampooldelete(MemoryPoolKind::Amplimix, OutputNodeInstance, _outputNode);
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
        _inputNode->SetInput(layer, &in);
        _outputNode->SetOutput(&out);

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
        if (_nodes.find(id) != _nodes.end())
            return _nodes.at(id);

        return nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
