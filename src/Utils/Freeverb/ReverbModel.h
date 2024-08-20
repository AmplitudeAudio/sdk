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

#ifndef SS_AMPLITUDE_AUDIO_REVMODEL_H
#define SS_AMPLITUDE_AUDIO_REVMODEL_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include "AllPass.h"
#include "Comb.h"

#include "tuning.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace Freeverb
{
    class ReverbModel
    {
    public:
        ReverbModel();

        void Mute();
        void ProcessMix(
            AmConstAudioSampleBuffer inputL, AmConstAudioSampleBuffer inputR, AmAudioSampleBuffer outputL, AmAudioSampleBuffer outputR, AmUInt64 frames, AmUInt32 skip);
        void ProcessReplace(
            AmConstAudioSampleBuffer inputL, AmConstAudioSampleBuffer inputR, AmAudioSampleBuffer outputL, AmAudioSampleBuffer outputR, AmUInt64 frames, AmUInt32 skip);
        void SetRoomSize(AmReal32 value);
        AmReal32 GetRoomSize() const;
        void SetDamp(AmReal32 value);
        AmReal32 GetDamp() const;
        void SetWet(AmReal32 value);
        AmReal32 GetWet() const;
        void SetDry(AmReal32 value);
        AmReal32 GetDry() const;
        void SetWidth(AmReal32 value);
        AmReal32 GetWidth() const;
        void SetMode(AmReal32 value);
        AmReal32 GetMode() const;

    private:
        void Update();

    private:
        AmReal32 _gain;
        AmReal32 _roomSize, _roomSize1;
        AmReal32 _damp, _damp1;
        AmReal32 _wet, _wet1, _wet2;
        AmReal32 _dry;
        AmReal32 _width;
        AmReal32 _mode;

        bool _dirty;

        // The following are all declared inline
        // to remove the need for dynamic allocation
        // with its subsequent error-checking messiness

        // Comb filters
        Comb _combL[kNumCombs];
        Comb _combR[kNumCombs];

        // Allpass filters
        AllPass _allPassL[kNumAllPasses];
        AllPass _allPassR[kNumAllPasses];

        // Buffers for the combs
        AmReal32 _bufCombL1[kCombTuningL1];
        AmReal32 _bufCombR1[kCombTuningR1];
        AmReal32 _bufCombL2[kCombTuningL2];
        AmReal32 _bufCombR2[kCombTuningR2];
        AmReal32 _bufCombL3[kCombTuningL3];
        AmReal32 _bufCombR3[kCombTuningR3];
        AmReal32 _bufCombL4[kCombTuningL4];
        AmReal32 _bufCombR4[kCombTuningR4];
        AmReal32 _bufCombL5[kCombTuningL5];
        AmReal32 _bufCombR5[kCombTuningR5];
        AmReal32 _bufCombL6[kCombTuningL6];
        AmReal32 _bufCombR6[kCombTuningR6];
        AmReal32 _bufCombL7[kCombTuningL7];
        AmReal32 _bufCombR7[kCombTuningR7];
        AmReal32 _bufCombL8[kCombTuningL8];
        AmReal32 _bufCombR8[kCombTuningR8];

        // Buffers for the allpasses
        AmReal32 _bufAllPassL1[kAllPassTuningL1];
        AmReal32 _bufAllPassR1[kAllPassTuningR1];
        AmReal32 _bufAllPassL2[kAllPassTuningL2];
        AmReal32 _bufAllPassR2[kAllPassTuningR2];
        AmReal32 _bufAllPassL3[kAllPassTuningL3];
        AmReal32 _bufAllPassR3[kAllPassTuningR3];
        AmReal32 _bufAllPassL4[kAllPassTuningL4];
        AmReal32 _bufAllPassR4[kAllPassTuningR4];
    };
} // namespace Freeverb

#endif // SS_AMPLITUDE_AUDIO_REVMODEL_H
