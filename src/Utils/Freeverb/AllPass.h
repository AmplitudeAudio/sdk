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
//
// Based on code written by Jezar at Dreampoint, June 2000 http://www.dreampoint.co.uk,
// which was placed in public domain.

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_ALLPASS_H
#define SS_AMPLITUDE_AUDIO_ALLPASS_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include "denormals.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace Freeverb
{
    class AllPass
    {
    public:
        AllPass();

        void SetBuffer(AmReal32Buffer buffer, AmInt32 size);
        AM_INLINE(AmReal32) Process(AmReal32 input);
        void Mute();
        void SetFeedback(AmReal32 value);
        AmReal32 GetFeedback();

    private:
        AmReal32 _feedback;
        AmReal32Buffer _buffer;
        AmInt32 _bufferSize;
        AmInt32 _bufferIndex;
    };

    // Big to inline - but crucial for speed

    AM_INLINE(AmReal32) AllPass::Process(AmReal32 input)
    {
        AmReal32 output;
        AmReal32 bufout;

        bufout = _buffer[_bufferIndex];
        undenormalise(bufout);

        output = -input + bufout;
        _buffer[_bufferIndex] = input + (bufout * _feedback);

        if (++_bufferIndex >= _bufferSize)
            _bufferIndex = 0;

        return output;
    }
} // namespace Freeverb

#endif // SS_AMPLITUDE_AUDIO_ALLPASS_H
