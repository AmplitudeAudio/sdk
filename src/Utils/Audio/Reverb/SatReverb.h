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

#ifndef _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_SAT_REVERB_H
#define _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_SAT_REVERB_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>

#include <Utils/Audio/Reverb/BaseReverb.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SatReverb : public BaseReverb
    {
    public:
        SatReverb(AmUInt64 sampleRate);

        void Process(AmReal32 x, AmReal32& l, AmReal32& r);
        void Process(const AudioBuffer& input, AudioBuffer& output);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_UTILS_AUDIO_REVERB_SAT_REVERB_H
