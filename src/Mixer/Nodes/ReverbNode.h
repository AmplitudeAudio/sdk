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

#pragma once

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_REVERB_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_REVERB_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <Utils/Freeverb/ReverbModel.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ReverbNodeInstance final : public ProcessorNodeInstance
    {
    public:
        ReverbNodeInstance();

        void Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* node) override;
        void Reset() override;

        const AudioBuffer* Process(const AudioBuffer* input) override;

    private:
        Freeverb::ReverbModel _model;
        AudioBuffer _output;
    };

    class ReverbNode final : public Node
    {
    public:
        ReverbNode();

        [[nodiscard]] AM_INLINE NodeInstance* CreateInstance() const override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, ReverbNodeInstance);
        }

        void DestroyInstance(NodeInstance* instance) const override
        {
            ampooldelete(MemoryPoolKind::Amplimix, ReverbNodeInstance, (ReverbNodeInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_REVERB_NODE_H