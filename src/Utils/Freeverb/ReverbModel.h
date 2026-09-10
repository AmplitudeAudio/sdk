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

#include <vector>

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
        void SetSampleRate(AmUInt32 sampleRate);
        AmUInt32 GetSampleRate() const;

    private:
        void Update();
        void AllocateBuffers();

    private:
        AmReal32 _gain;
        AmReal32 _roomSize, _roomSize1;
        AmReal32 _damp, _damp1;
        AmReal32 _wet, _wet1, _wet2;
        AmReal32 _dry;
        AmReal32 _width;
        AmReal32 _mode;

        bool _dirty;

        AmUInt32 _sampleRate;

        // Comb filters
        Comb _combL[kNumCombs];
        Comb _combR[kNumCombs];

        // Allpass filters
        AllPass _allPassL[kNumAllPasses];
        AllPass _allPassR[kNumAllPasses];

        // Dynamically sized buffers for the combs
        std::vector<AmReal32> _bufCombL[kNumCombs];
        std::vector<AmReal32> _bufCombR[kNumCombs];

        // Dynamically sized buffers for the allpasses
        std::vector<AmReal32> _bufAllPassL[kNumAllPasses];
        std::vector<AmReal32> _bufAllPassR[kNumAllPasses];
    };
} // namespace Freeverb

#endif // SS_AMPLITUDE_AUDIO_REVMODEL_H
