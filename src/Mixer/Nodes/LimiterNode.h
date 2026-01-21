// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_LIMITER_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_LIMITER_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

namespace SparkyStudios::Audio::Amplitude
{
    class LimiterNodeInstance final : public ProcessorNodeInstance
    {
    public:
        enum ATTRIBUTES
        {
            ATTRIBUTE_THRESOLD_DB = 0,
            ATTRIBUTE_ATTACK_MS,
            ATTRIBUTE_RELEASE_MS,
            ATTRIBUTE_LAST
        };

        LimiterNodeInstance();

        const AudioBuffer* Process(const AudioBuffer* input) override;

        void Reset() override;

        void SetParameter(AmSize index, AmReal32 value) override;

    private:
        void UpdateCoefficients(AmReal32 sampleRate);

        AmReal32 _sampleRate;
        AmReal32 _attackCoeff;
        AmReal32 _releaseCoeff;

        AmReal32 _enveloppe;
    };

    class LimiterNode final : public Node
    {
    public:
        LimiterNode();

        [[nodiscard]] AM_INLINE std::shared_ptr<NodeInstance> CreateInstance() const override
        {
            return ampoolshared(eMemoryPoolKind_Amplimix, LimiterNodeInstance);
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

        [[nodiscard]] AM_INLINE AmSize GetParameterCount() const override
        {
            return 3;
        }

        [[nodiscard]] AM_INLINE AmString GetParameterName(AmSize index) const override
        {
            switch (index)
            {
            case 0:
                return "Thresold (dB)";
            case 1:
                return "Attack (ms)";
            case 2:
                return "Release (ms)";
            default:
                return "";
            }
        }

        [[nodiscard]] eParameterType GetParameterType(AmSize index) const override
        {
            return eParameterType_Float;
        }

        [[nodiscard]] AmReal32 GetParameterMax(AmSize index) const override
        {
            switch (index)
            {
            case 0:
                return 0.0f;
            case 1:
                return 100.0f;
            case 2:
                return 2000.0f;
            default:
                return 0.0f;
            }
        }

        [[nodiscard]] AmReal32 GetParameterMin(AmSize index) const override
        {
            switch (index)
            {
            case 0:
                return -60.0f;
            case 1:
                return 0.1f;
            case 2:
                return 20.0f;
            default:
                return 0.0f;
            }
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_LIMITER_NODE_H
