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
#include <SparkyStudios/Audio/Amplitude/DSP/Reverb.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ReverbNodeInstance final : public ProcessorNodeInstance
    {
    public:
        ReverbNodeInstance();
        explicit ReverbNodeInstance(AmString algorithmName);

        void Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline, AmSize paramCount) override;
        void Reset() override;

        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

        const AudioBuffer* Process(const AudioBuffer* input) override;

        [[nodiscard]] AM_INLINE AmUInt16 GetOutputChannelCount() const override
        {
            return kAmStereoChannelCount;
        }

    private:
        AmString _algorithmName;
        std::shared_ptr<ReverbInstance> _reverbInstance;
        AudioBuffer _tempBuffer;
    };

    class ReverbNode final : public Node
    {
    public:
        ReverbNode();
        explicit ReverbNode(AmString algorithmName);
        ReverbNode(AmString name, AmString algorithmName);

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, ReverbNodeInstance, _algorithmName);
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

        [[nodiscard]] AM_INLINE const AmString& GetAlgorithmName() const
        {
            return _algorithmName;
        }

    private:
        AmString _algorithmName;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_REVERB_NODE_H
