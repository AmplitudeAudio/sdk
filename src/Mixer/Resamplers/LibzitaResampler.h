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

#include <zita-resampler/vresampler.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ZitaResamplerInstance final : public ResamplerInstance
    {
    public:
        void Init(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut, AmUInt64 frameCount) override
        {
            _resampler = amnew(VResampler);

            _numChannels = channelCount;
            _frameCount = frameCount;

            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;
            _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);

            _resampler->setup(_sampleRatio, channelCount, 48);
        }

        bool Process(AmAudioSampleBuffer input, AmUInt64& inputFrames, AmAudioSampleBuffer output, AmUInt64& outputFrames) override
        {
            _resampler->out_count = outputFrames;
            _resampler->out_data = output;

            _resampler->inp_count = inputFrames;
            _resampler->inp_data = input;

            _resampler->process();

            if (_resampler->inp_count > 0)
                inputFrames -= _resampler->inp_count;

            if (_resampler->out_count > 0)
                outputFrames -= _resampler->out_count;

            return true;
        }

        void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override
        {
            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;

            const AmReal64 ratio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);

            _resampler->set_rratio(ratio / _sampleRatio);

            _sampleRatio = ratio;
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
            VResampler temp;

            temp.setup(_sampleRatio, _numChannels, 48);

            AmUInt64 step = temp.inpsize() - 1;

            temp.inp_count = step;
            temp.inp_data = nullptr;
            temp.out_count = outputFrameCount;
            temp.out_data = nullptr;

            AmUInt64 inputFrameCount = 0;
            while (temp.out_count > 0)
            {
                temp.process();

                if (temp.out_count == 0)
                {
                    if (temp.inp_count > 0)
                        inputFrameCount += step - temp.inp_count;

                    break;
                }

                if (temp.inp_count == 0)
                {
                    temp.inp_count = step;
                    inputFrameCount += step;
                }
            }

            return inputFrameCount - GetLatencyInFrames();
        }

        [[nodiscard]] AmUInt64 GetExpectedOutputFrameCount(AmUInt64 inputFrameCount) const override
        {
            return 0;
        }

        [[nodiscard]] AmUInt64 GetLatencyInFrames() const override
        {
            return _resampler->inpsize() - 1;
        }

        void Reset() override
        {
            _resampler->reset();
        }

        void Clear() override
        {
            if (_resampler == nullptr)
                return;

            _resampler->clear();
            amdelete(VResampler, _resampler);
            _resampler = nullptr;
        }

    private:
        AmUInt16 _numChannels = 0;
        AmUInt64 _frameCount = 0;

        AmUInt32 _sampleRateIn = 0;
        AmUInt32 _sampleRateOut = 0;
        AmReal64 _sampleRatio = 0.0;

        VResampler* _resampler = nullptr;
    };

    [[maybe_unused]] static class ZitaResampler final : public Resampler
    {
    public:
        ZitaResampler()
            : Resampler("libzita")
        {}

        ResamplerInstance* CreateInstance() override
        {
            return amnew(ZitaResamplerInstance);
        }

        void DestroyInstance(ResamplerInstance* instance) override
        {
            amdelete(ZitaResamplerInstance, (ZitaResamplerInstance*)instance);
        }
    } gZitaResampler; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_LIBSAMPLERATE_RESAMPLER_H
