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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_AMBISONIC_BINAURAL_DECODER_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_AMBISONIC_BINAURAL_DECODER_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <Ambisonics/AmbisonicBinauralizer.h>
#include <Ambisonics/BFormat.h>
#include <HRTF/HRIRSphere.h>
#include <Mixer/Pipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmbisonicBinauralDecoderNodeInstance final : public ProcessorNodeInstance
    {
    public:
        AmbisonicBinauralDecoderNodeInstance();

        const AudioBuffer* Process(const AudioBuffer* input) override;

        [[nodiscard]] AM_INLINE AmUInt16 GetOutputChannelCount() const override
        {
            return kAmStereoChannelCount;
        }

        void Configure(AmUInt64 frameCount, AmUInt16 channelCount) override;

    private:
        std::shared_ptr<const HRIRSphere> _hrirSphere;
        AmbisonicBinauralizer _binauralizer;
        AmbisonicDecoder _decoder;
        BFormat _soundField;
        AmUInt32 _ambisonicOrder = 1;
    };

    class AmbisonicBinauralDecoderNode final : public Node
    {
    public:
        AmbisonicBinauralDecoderNode();

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, AmbisonicBinauralDecoderNodeInstance);
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

#endif // _AM_IMPLEMENTATION_MIXER_NODES_AMBISONIC_BINAURAL_DECODER_NODE_H
