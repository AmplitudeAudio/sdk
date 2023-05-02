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

#ifndef SS_AMPLITUDE_AUDIO_R8BRAIN_RESAMPLER_H
#define SS_AMPLITUDE_AUDIO_R8BRAIN_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Utils/Audio/Resampling/CDSPResampler.h>
#include <Utils/miniaudio/miniaudio_utils.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    class R8BrainResamplerInstance final : public ResamplerInstance
    {
        void Init(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut, AmUInt64 frameCount) override
        {
            _numChannels = channelCount;
            _frameCount = frameCount;

            _resamplers.resize(channelCount);

            SetSampleRate(sampleRateIn, sampleRateOut);
        }

        bool Process(AmAudioSampleBuffer input, AmUInt64& inputFrames, AmAudioSampleBuffer output, AmUInt64& outputFrames) override
        {
            auto input64 = static_cast<AmReal64Buffer>(amMemory->Malloc(MemoryPoolKind::SoundData, inputFrames * sizeof(AmReal64)));

            for (AmUInt16 c = 0; c < _numChannels; c++)
            {
                r8b::CDSPResampler16* resampler = _resamplers[c].get();

                for (AmUInt64 i = 0; i < inputFrames; i++)
                    input64[i] = (AmReal64)input[i * _numChannels + c];

                AmReal64Buffer output64 = nullptr;
                outputFrames = resampler->process(input64, static_cast<AmInt32>(inputFrames), output64);
                resampler->clear();

                if (output64 == nullptr)
                {
                    amMemory->Free(MemoryPoolKind::SoundData, input64);
                    return false;
                }

                for (AmUInt64 i = 0; i < outputFrames; i++)
                    output[i * _numChannels + c] = (AmReal32)output64[i];
            }

            amMemory->Free(MemoryPoolKind::SoundData, input64);

            return true;
        }

        void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override
        {
            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;

            for (AmUInt16 c = 0; c < _numChannels; c++)
            {
                if (_resamplers[c] != nullptr)
                    _resamplers[c].reset();

                _resamplers[c] = std::make_unique<r8b::CDSPResampler16>(sampleRateIn, sampleRateOut, _frameCount);
            }
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
            if (_resamplers.empty())
                return 0;

            return _resamplers[0]->getInputRequiredForOutput(outputFrameCount);
        }

        [[nodiscard]] AmUInt64 GetExpectedOutputFrameCount(AmUInt64 inputFrameCount) const override
        {
            // TODO: Implement this
            return 0;
        }

        [[nodiscard]] AmUInt64 GetLatencyInFrames() const override
        {
            if (_resamplers.empty())
                return 0;

            return _resamplers[0]->getInLenBeforeOutPos(0);
        }

        void Reset() override
        {
            if (_resamplers.empty())
                return;

            for (AmUInt16 c = 0; c < _numChannels; c++)
                _resamplers[c]->clear();
        }

        void Clear() override
        {
            if (_resamplers.empty())
                return;

            for (AmUInt16 c = 0; c < _numChannels; c++)
                _resamplers[c].reset();
        }

    private:
        AmUInt16 _numChannels = 0;
        AmUInt64 _frameCount = 0;

        AmUInt32 _sampleRateIn = 0;
        AmUInt32 _sampleRateOut = 0;

        std::vector<std::unique_ptr<r8b::CDSPResampler16>> _resamplers;
    };

    [[maybe_unused]] static class R8BrainResampler final : public Resampler
    {
    public:
        R8BrainResampler()
            : Resampler("R8Brain")
        {}

        ResamplerInstance* CreateInstance() override
        {
            return amnew(R8BrainResamplerInstance);
        }

        void DestroyInstance(ResamplerInstance* instance) override
        {
            amdelete(R8BrainResamplerInstance, (R8BrainResamplerInstance*)instance);
        }
    } gR8BrainResampler; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_R8BRAIN_RESAMPLER_H
