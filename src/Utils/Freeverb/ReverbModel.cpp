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

#include <Utils/Utils.h>

#include "ReverbModel.h"

namespace Freeverb
{
    ReverbModel::ReverbModel()
    {
        // Tie the components to their buffers
        _combL[0].SetBuffer(_bufCombL1, kCombTuningL1);
        _combR[0].SetBuffer(_bufCombR1, kCombTuningR1);
        _combL[1].SetBuffer(_bufCombL2, kCombTuningL2);
        _combR[1].SetBuffer(_bufCombR2, kCombTuningR2);
        _combL[2].SetBuffer(_bufCombL3, kCombTuningL3);
        _combR[2].SetBuffer(_bufCombR3, kCombTuningR3);
        _combL[3].SetBuffer(_bufCombL4, kCombTuningL4);
        _combR[3].SetBuffer(_bufCombR4, kCombTuningR4);
        _combL[4].SetBuffer(_bufCombL5, kCombTuningL5);
        _combR[4].SetBuffer(_bufCombR5, kCombTuningR5);
        _combL[5].SetBuffer(_bufCombL6, kCombTuningL6);
        _combR[5].SetBuffer(_bufCombR6, kCombTuningR6);
        _combL[6].SetBuffer(_bufCombL7, kCombTuningL7);
        _combR[6].SetBuffer(_bufCombR7, kCombTuningR7);
        _combL[7].SetBuffer(_bufCombL8, kCombTuningL8);
        _combR[7].SetBuffer(_bufCombR8, kCombTuningR8);
        _allPassL[0].SetBuffer(_bufAllPassL1, kAllPassTuningL1);
        _allPassR[0].SetBuffer(_bufAllPassR1, kAllPassTuningR1);
        _allPassL[1].SetBuffer(_bufAllPassL2, kAllPassTuningL2);
        _allPassR[1].SetBuffer(_bufAllPassR2, kAllPassTuningR2);
        _allPassL[2].SetBuffer(_bufAllPassL3, kAllPassTuningL3);
        _allPassR[2].SetBuffer(_bufAllPassR3, kAllPassTuningR3);
        _allPassL[3].SetBuffer(_bufAllPassL4, kAllPassTuningL4);
        _allPassR[3].SetBuffer(_bufAllPassR4, kAllPassTuningR4);

        // Set default values
        _allPassL[0].SetFeedback(0.5f);
        _allPassR[0].SetFeedback(0.5f);
        _allPassL[1].SetFeedback(0.5f);
        _allPassR[1].SetFeedback(0.5f);
        _allPassL[2].SetFeedback(0.5f);
        _allPassR[2].SetFeedback(0.5f);
        _allPassL[3].SetFeedback(0.5f);
        _allPassR[3].SetFeedback(0.5f);
        SetWet(kInitialWet);
        SetRoomSize(kInitialRoom);
        SetDry(kInitialDry);
        SetDamp(kInitialDamp);
        SetWidth(kInitialWidth);
        SetMode(kInitialMode);

        // Buffer will be full of rubbish - so we MUST Mute them
        Mute();
    }

    void ReverbModel::Mute()
    {
        if (GetMode() >= kFreezeMode)
            return;

        for (AmInt32 i = 0; i < kNumCombs; i++)
        {
            _combL[i].Mute();
            _combR[i].Mute();
        }
        for (AmInt32 i = 0; i < kNumAllPasses; i++)
        {
            _allPassL[i].Mute();
            _allPassR[i].Mute();
        }
    }

    void ReverbModel::ProcessReplace(
        AmConstAudioSampleBuffer inputL, AmConstAudioSampleBuffer inputR, AmAudioSampleBuffer outputL, AmAudioSampleBuffer outputR, AmUInt64 frames, AmUInt32 skip)
    {
        AmReal32 outL, outR, input;

        if (_dirty)
            Update();

        while (frames-- > 0)
        {
            outL = outR = 0;
            input = (*inputL + *inputR) * _gain;

            // Accumulate comb filters in parallel
            for (AmUInt32 i = 0; i < kNumCombs; i++)
            {
                outL += _combL[i].Process(input);
                outR += _combR[i].Process(input);
            }

            // Feed through allpasses in series
            for (AmUInt32 i = 0; i < kNumAllPasses; i++)
            {
                outL = _allPassL[i].Process(outL);
                outR = _allPassR[i].Process(outR);
            }

            // Calculate output REPLACING anything already there
            outL = outL * _wet1 + outR * _wet2 + *inputL * _dry;
            outR = outR * _wet1 + outL * _wet2 + *inputR * _dry;

            undenormalise(outL);
            undenormalise(outR);

            *outputL = outL;
            *outputR = outR;

            // Increment sample pointers, allowing for interleave (if any)
            inputL += skip;
            inputR += skip;
            outputL += skip;
            outputR += skip;
        }
    }

    void ReverbModel::ProcessMix(
        AmConstAudioSampleBuffer inputL, AmConstAudioSampleBuffer inputR, AmAudioSampleBuffer outputL, AmAudioSampleBuffer outputR, AmUInt64 frames, AmUInt32 skip)
    {
        AmReal32 outL, outR, input;

        if (_dirty)
            Update();

        while (frames-- > 0)
        {
            outL = outR = 0;
            input = (*inputL + *inputR) * _gain;

            // Accumulate comb filters in parallel
            for (AmUInt32 i = 0; i < kNumCombs; i++)
            {
                outL += _combL[i].Process(input);
                outR += _combR[i].Process(input);
            }

            // Feed through allpasses in series
            for (AmUInt32 i = 0; i < kNumAllPasses; i++)
            {
                outL = _allPassL[i].Process(outL);
                outR = _allPassR[i].Process(outR);
            }

            // Calculate output MIXING with anything already there
            *outputL += outL * _wet1 + outR * _wet2 + *inputL * _dry;
            *outputR += outR * _wet1 + outL * _wet2 + *inputR * _dry;

            // Increment sample pointers, allowing for interleave (if any)
            inputL += skip;
            inputR += skip;
            outputL += skip;
            outputR += skip;
        }
    }

    void ReverbModel::Update()
    {
        // Recalculate internal values after parameter change

        AmInt32 i;

        _wet1 = _wet * (_width / 2 + 0.5f);
        _wet2 = _wet * ((1 - _width) / 2);

        if (_mode >= kFreezeMode)
        {
            _roomSize1 = 1;
            _damp1 = 0;
            _gain = kMuted;
        }
        else
        {
            _roomSize1 = _roomSize;
            _damp1 = _damp;
            _gain = kFixedGain;
        }

        for (i = 0; i < kNumCombs; i++)
        {
            _combL[i].SetFeedback(_roomSize1);
            _combR[i].SetFeedback(_roomSize1);

            _combL[i].SetDamp(_damp1);
            _combR[i].SetDamp(_damp1);
        }

        _dirty = false;
    }

    // The following get/set functions are not inlined, because
    // speed is never an issue when calling them, and also
    // because as you develop the reverb model, you may
    // wish to take dynamic action when they are called.

    void ReverbModel::SetRoomSize(AmReal32 value)
    {
        if (GetRoomSize() == value)
            return;

        _roomSize = (value * kScaleRoom) + kOffsetRoom;
        _dirty = true;
    }

    AmReal32 ReverbModel::GetRoomSize() const
    {
        return (_roomSize - kOffsetRoom) / kScaleRoom;
    }

    void ReverbModel::SetDamp(AmReal32 value)
    {
        if (GetDamp() == value)
            return;

        _damp = value * kScaleDamp;
        _dirty = true;
    }

    AmReal32 ReverbModel::GetDamp() const
    {
        return _damp / kScaleDamp;
    }

    void ReverbModel::SetWet(AmReal32 value)
    {
        if (GetWet() == value)
            return;

        _wet = value * kScaleWet;
        _dirty = true;
    }

    AmReal32 ReverbModel::GetWet() const
    {
        return _wet / kScaleWet;
    }

    void ReverbModel::SetDry(AmReal32 value)
    {
        if (GetDry() == value)
            return;

        _dry = value * kScaleDry;
    }

    AmReal32 ReverbModel::GetDry() const
    {
        return _dry / kScaleDry;
    }

    void ReverbModel::SetWidth(AmReal32 value)
    {
        if (GetWidth() == value)
            return;

        _width = value;
        _dirty = true;
    }

    AmReal32 ReverbModel::GetWidth() const
    {
        return _width;
    }

    void ReverbModel::SetMode(AmReal32 value)
    {
        if (GetMode() == value)
            return;

        _mode = value;
        _dirty = true;
    }

    AmReal32 ReverbModel::GetMode() const
    {
        return _mode >= kFreezeMode ? 1 : 0;
    }
} // namespace Freeverb
