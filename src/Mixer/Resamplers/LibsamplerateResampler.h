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
        void Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override
        {
            _resampler.resize(channelCount);
            for (AmUInt16 c = 0; c < channelCount; c++)
                _resampler[c] = src_new(SRC_SINC_BEST_QUALITY, 1, nullptr);

            _numChannels = channelCount;

            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;
            _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);
        }

        bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) override
        {
            AMPLITUDE_ASSERT(input.GetChannelCount() == _numChannels);
            AMPLITUDE_ASSERT(output.GetChannelCount() == _numChannels);

            SRC_DATA data;
            for (AmUInt16 c = 0; c < _numChannels; c++)
            {
                data.data_in = input[c].begin();
                data.data_out = output[c].begin();
                data.end_of_input = 0;
                data.input_frames = inputFrames;
                data.output_frames = outputFrames;
                data.src_ratio = _sampleRatio;

                int err = src_process(_resampler[c], &data);

                if (err != 0)
                    return false;
            }

            inputFrames = data.input_frames_used;
            outputFrames = data.output_frames_gen;

            return true;
        }

        void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override
        {
            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;

            _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);

            for (AmUInt16 c = 0; c < _numChannels; c++)
                src_set_ratio(_resampler[c], _sampleRatio);
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

        [[nodiscard]] AmUInt64 GetRequiredInputFrames(AmUInt64 outputFrameCount) const override
        {
            return (AmUInt64)(static_cast<AmReal32>(outputFrameCount) / _sampleRatio);
        }

        [[nodiscard]] AmUInt64 GetExpectedOutputFrames(AmUInt64 inputFrameCount) const override
        {
            return (AmUInt64)(_sampleRatio * static_cast<AmReal32>(inputFrameCount));
        }

        [[nodiscard]] AmUInt64 GetInputLatency() const override
        {
            return 0;
        }

        [[nodiscard]] AmUInt64 GetOutputLatency() const override
        {
            return 0;
        }

        void Reset() override
        {
            for (AmUInt16 c = 0; c < _numChannels; c++)
                src_reset(_resampler[c]);
        }

        void Clear() override
        {
            if (_resampler.empty())
                return;

            for (AmUInt16 c = 0; c < _numChannels; c++)
                src_delete(_resampler[c]);

            _resampler.clear();
        }

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

        ResamplerInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Filtering, LibsamplerateResamplerInstance);
        }

        void DestroyInstance(ResamplerInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Filtering, LibsamplerateResamplerInstance, (LibsamplerateResamplerInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_LIBSAMPLERATE_RESAMPLER_H