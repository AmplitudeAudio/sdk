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

#pragma once

#ifndef _AM_IMPLEMENTATION_DSP_RESAMPLERS_LIBSAMPLERATE_RESAMPLER_H
#define _AM_IMPLEMENTATION_DSP_RESAMPLERS_LIBSAMPLERATE_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "samplerate.h"

namespace SparkyStudios::Audio::Amplitude
{
    class LibsamplerateResamplerInstance final : public ResamplerInstance
    {
    public:
        /**
         * @copydoc ResamplerInstance::Instance
         */
        void Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override;

        /**
         * @copydoc ResamplerInstance::Process
         */
        bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) override;

        /**
         * @copydoc ResamplerInstance::SetSampleRate
         */
        void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override;

        [[nodiscard]] AM_INLINE AmUInt32 GetSampleRateIn() const override
        {
            return _sampleRateIn;
        }

        /**
         * @copydoc ResamplerInstance::GetSampleRateOut
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSampleRateOut() const override
        {
            return _sampleRateOut;
        }

        /**
         * @copydoc ResamplerInstance::GetChannelCount
         */
        [[nodiscard]] AM_INLINE AmUInt16 GetChannelCount() const override
        {
            return _numChannels;
        }

        /**
         * @copydoc ResamplerInstance::GetRequiredInputFrames
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetRequiredInputFrames(AmUInt64 outputFrameCount) const override
        {
            return (AmUInt64)(static_cast<AmReal32>(outputFrameCount) / _sampleRatio);
        }

        /**
         * @copydoc ResamplerInstance::GetExpectedOutputFrames
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetExpectedOutputFrames(AmUInt64 inputFrameCount) const override
        {
            return (AmUInt64)(_sampleRatio * static_cast<AmReal32>(inputFrameCount));
        }

        /**
         * @copydoc ResamplerInstance::GetInputLatency
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetInputLatency() const override
        {
            return 0;
        }

        /**
         * @copydoc ResamplerInstance::GetOutputLatency
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetOutputLatency() const override
        {
            return 0;
        }

        /**
         * @copydoc ResamplerInstance::Reset
         */
        void Reset() override;

        /**
         * @copydoc ResamplerInstance::Clear
         */
        void Clear() override;

    private:
        AmUInt16 _numChannels = 0;

        AmUInt32 _sampleRateIn = 0;
        AmUInt32 _sampleRateOut = 0;
        AmReal64 _sampleRatio = 0.0;

        std::vector<SRC_STATE*> _resampler;
    };

    class LibsamplerateResampler final : public Resampler
    {
    public:
        LibsamplerateResampler()
            : Resampler("libsamplerate")
        {}

        ResamplerInstance* CreateInstance() override;

        void DestroyInstance(ResamplerInstance* instance) override;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_RESAMPLERS_LIBSAMPLERATE_RESAMPLER_H