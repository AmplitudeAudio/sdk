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

#include <Core/AudioBufferCrossFader.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AudioBufferCrossFader::AudioBufferCrossFader(AmSize sampleCount)
        : _crossFadeBuffer(sampleCount, 2)
    {
        AMPLITUDE_ASSERT(sampleCount != 0);

        auto& fadeInChannel = _crossFadeBuffer[0];
        auto& fadeOutChannel = _crossFadeBuffer[1];

        for (AmSize i = 0; i < sampleCount; ++i)
        {
            const float factor = static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleCount);
            fadeInChannel[i] = factor;
            fadeOutChannel[i] = 1.0f - factor;
        }
    }

    void AudioBufferCrossFader::CrossFade(const AudioBuffer& bufferIn, const AudioBuffer& bufferOut, AudioBuffer& outputBuffer) const
    {
        AMPLITUDE_ASSERT(&outputBuffer != &bufferIn && &outputBuffer != &bufferOut);
        AMPLITUDE_ASSERT(bufferIn.GetChannelCount() == bufferOut.GetChannelCount());
        AMPLITUDE_ASSERT(bufferIn.GetFrameCount() == bufferOut.GetFrameCount());

        const AmSize channelCount = bufferIn.GetChannelCount();
        const AmSize frameCount = bufferIn.GetFrameCount();

        const auto& fadeInChannel = _crossFadeBuffer[0];
        const auto& fadeOutChannel = _crossFadeBuffer[1];

        for (AmSize channel = 0; channel < channelCount; ++channel)
        {
            const auto& inputChannelIn = bufferIn[channel];
            const auto& inputChannelOut = bufferOut[channel];
            auto& outputChannel = outputBuffer[channel];

            PointWiseMultiply(fadeInChannel.begin(), inputChannelIn.begin(), outputChannel.begin(), frameCount);
            PointWiseMultiplyAccumulate(fadeOutChannel.begin(), inputChannelOut.begin(), outputChannel.begin(), frameCount);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
