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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_OCCLUSION_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_OCCLUSION_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <DSP/Filters/MonoPoleFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class OcclusionNodeInstance final : public ProcessorNodeInstance
    {
    public:
        OcclusionNodeInstance();

        ~OcclusionNodeInstance() override;

        const AudioBuffer* Process(const AudioBuffer* input) override;

    private:
        MonoPoleFilter _filter;

        AmReal32 _currentOcclusion;
        FilterInstance* _occlusionFilter;

        AudioBuffer _output;
    };

    class OcclusionNode final : public Node
    {
    public:
        OcclusionNode();

        [[nodiscard]] AM_INLINE NodeInstance* CreateInstance() const override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, OcclusionNodeInstance);
        }

        AM_INLINE void DestroyInstance(NodeInstance* instance) const override
        {
            ampooldelete(MemoryPoolKind::Amplimix, OcclusionNodeInstance, (OcclusionNodeInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_OCCLUSION_NODE_H
