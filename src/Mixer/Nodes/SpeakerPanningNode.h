// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_SPEAKER_PANNING_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_SPEAKER_PANNING_NODE_H

#include <array>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <DSP/Gain.h>
#include <DSP/SpeakerPanner.h>
#include <DSP/SpreadEmitters.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Places a sound's channels on the output speakers.
     *
     * 3D: each source channel is a spread emitter around the sound's direction, panned with constant-power VBAP. 2D: the
     * channel map (identity, ITU fold-down or direct placement). The LFE goes to the output LFE in both cases.
     */
    class SpeakerPanningNodeInstance final : public ProcessorNodeInstance
    {
    public:
        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

        const AudioBuffer* Process(const AudioBuffer* input) override;

        [[nodiscard]] bool ShouldSkip() const override;

        [[nodiscard]] AmUInt16 GetOutputChannelCount() const override;

    private:
        /// Gains per emitter and output speaker.
        using EmitterGains = std::array<SpeakerGains, kAmMaxLayoutChannelCount>;

        [[nodiscard]] AmReal32 EffectiveSpread(const Listener& listener) const;
        SpreadEmitterSet SingleSourceGains(const AmVector3& location, const AmMatrix4& view, AmReal32 spread, EmitterGains& gains) const;
        SpreadEmitterSet MultiSourceGains(const AmMatrix4& view, AmReal32 spread, EmitterGains& gains) const;
        void Render(const AudioBuffer* input, const SpreadEmitterSet& emitters, const EmitterGains& gains, AmUInt64 frames);

        const ChannelLayout* _sourceLayout = &ChannelLayout::Get(eChannelLayout_Mono);
        ChannelMatrix _map2D{};
        std::array<std::array<GainProcessor, kAmMaxLayoutChannelCount>, kAmMaxLayoutChannelCount> _gains{};
    };

    class SpeakerPanningNode final : public Node
    {
    public:
        explicit SpeakerPanningNode(AmString name = "SpeakerPanning");

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, SpeakerPanningNodeInstance);
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

#endif // _AM_IMPLEMENTATION_MIXER_NODES_SPEAKER_PANNING_NODE_H
