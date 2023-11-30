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

#ifndef SS_AMPLITUDE_AUDIO_LIBSAMPLERATE_RESAMPLER_H
#define SS_AMPLITUDE_AUDIO_LIBSAMPLERATE_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "samplerate.h"

namespace SparkyStudios::Audio::Amplitude
{
    class LibsamplerateResamplerInstance final : public ResamplerInstance
    {
    public:
        void Init(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut, AmUInt64 frameCount) override
        {
            _resampler = src_new(SRC_SINC_BEST_QUALITY, channelCount, nullptr);

            _numChannels = channelCount;
            _frameCount = frameCount;

            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;
            _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);
        }

        bool Process(AmConstAudioSampleBuffer input, AmUInt64& inputFrames, AmAudioSampleBuffer output, AmUInt64& outputFrames) override
        {
            SRC_DATA data;
            data.data_in = input;
            data.data_out = output;
            data.end_of_input = 0;
            data.input_frames = inputFrames;
            data.output_frames = outputFrames;
            data.src_ratio = _sampleRatio;

            int err = src_process(_resampler, &data);

            inputFrames = data.input_frames_used;
            outputFrames = data.output_frames_gen;

            return err == 0;
        }

        void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override
        {
            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;

            _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);

            src_set_ratio(_resampler, _sampleRatio);
        }

        [[nodiscard]] AmUInt32 GetSampleRateIn() const override
        {
            return _sampleRateIn;
        }

        [[nodiscard]] AmUInt32 GetSampleRateOut() const override
        {
            return _sampleRateOut;
        }

        [[nodiscard]] AmUInt16 GetChannelCount() const override
        {
            return _numChannels;
        }

        [[nodiscard]] AmUInt64 GetRequiredInputFrameCount(AmUInt64 outputFrameCount) const override
        {
            return std::ceil(static_cast<AmReal32>(outputFrameCount) / _sampleRatio);
        }

        [[nodiscard]] AmUInt64 GetExpectedOutputFrameCount(AmUInt64 inputFrameCount) const override
        {
            return std::ceil(_sampleRatio * static_cast<AmReal32>(inputFrameCount));
        }

        [[nodiscard]] AmUInt64 GetLatencyInFrames() const override
        {
            return 0;
        }

        void Reset() override
        {
            src_reset(_resampler);
        }

        void Clear() override
        {
            if (_resampler == nullptr)
                return;

            src_delete(_resampler);
            _resampler = nullptr;
        }

    private:
        AmUInt16 _numChannels = 0;
        AmUInt64 _frameCount = 0;

        AmUInt32 _sampleRateIn = 0;
        AmUInt32 _sampleRateOut = 0;
        AmReal64 _sampleRatio = 0.0;

        SRC_STATE* _resampler;
    };

    [[maybe_unused]] static class LibsamplerateResampler final : public Resampler
    {
    public:
        LibsamplerateResampler()
            : Resampler("libsamplerate")
        {}

        ResamplerInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Filtering, LibsamplerateResamplerInstance);
        }

        void DestroyInstance(ResamplerInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Filtering, LibsamplerateResamplerInstance, (LibsamplerateResamplerInstance*)instance);
        }
    } gLibsamplerateResampler; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_LIBSAMPLERATE_RESAMPLER_H