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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_AMBISONIC_PANNING_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_AMBISONIC_PANNING_NODE_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <array>

#include <Ambisonics/AmbisonicSource.h>
#include <DSP/SpreadEmitters.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmbisonicPanningNodeInstance final : public ProcessorNodeInstance
    {
    public:
        AmbisonicPanningNodeInstance();

        const AudioBuffer* Process(const AudioBuffer* input) override;

        [[nodiscard]] bool ShouldSkip() const override;

        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

        [[nodiscard]] AmUInt16 GetOutputChannelCount() const override;

    private:
        void EncodeEmitters(
            const AudioBuffer* input, const AmVector3& location, const AmVector3& listenerPosition, AmReal32 spread, AmReal32 gain);

        /// One encoder per emitter, so each keeps its own coefficient interpolation.
        std::array<AmbisonicSource, kAmMaxLayoutChannelCount> _sources;
        BFormat _soundField;
        BFormat _emitterField;
        AudioBuffer _scaledInput;
        const ChannelLayout* _sourceLayout = &ChannelLayout::Get(eChannelLayout_Mono);

        AmUInt32 _ambisonicOrder = 1;
    };

    class AmbisonicPanningNode final : public Node
    {
    public:
        AmbisonicPanningNode();

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, AmbisonicPanningNodeInstance);
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

        [[nodiscard]] AM_INLINE eNodeChannelPolicy GetChannelPolicy() const override
        {
            return eNodeChannelPolicy_Custom;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_AMBISONIC_PANNING_NODE_H
