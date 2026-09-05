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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_ENVIRONMENT_EFFECT_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_ENVIRONMENT_EFFECT_NODE_H

#include <utility>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EnvironmentEffectNodeInstance final : public ProcessorNodeInstance
    {
    public:
        EnvironmentEffectNodeInstance();
        ~EnvironmentEffectNodeInstance() override;

        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

        bool ShouldSkip() const override;

        const AudioBuffer* Process(const AudioBuffer* input) override;

    private:
        AudioBuffer _scratch;
        std::unordered_map<AmEnvironmentID, std::unordered_map<AmObjectID, std::shared_ptr<EffectInstance>>> _environmentFilters;
        std::vector<std::pair<AmEnvironmentID, AmReal32>> _items;
    };

    class EnvironmentEffectNode final : public Node
    {
    public:
        EnvironmentEffectNode();

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, EnvironmentEffectNodeInstance);
        }

        [[nodiscard]] AM_INLINE bool CanConsume() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE bool CanProduce() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE AmSize GetMaxInputCount() const override
        {
            return 1;
        }

        [[nodiscard]] AM_INLINE AmSize GetMinInputCount() const override
        {
            return 1;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_ENVIRONMENT_EFFECT_NODE_H
