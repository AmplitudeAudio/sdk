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

#ifndef _AM_CORE_AUDIO_BUFFER_CROSS_FADER_H
#define _AM_CORE_AUDIO_BUFFER_CROSS_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AudioBufferCrossFader
    {
    public:
        explicit AudioBufferCrossFader(AmSize sampleCount);

        /**
         * @brief Performs a linear cross-fading between two audio buffers.
         *
         * @param bufferIn The first audio buffer to cross-fade from.
         * @param bufferOut The second audio buffer to cross-fade to.
         * @param outputBuffer The resulting cross-faded audio buffer.
         */
        void CrossFade(const AudioBuffer& bufferIn, const AudioBuffer& bufferOut, AudioBuffer& outputBuffer) const;

    private:
        AudioBuffer _crossFadeBuffer;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_AUDIO_BUFFER_CROSS_FADER_H