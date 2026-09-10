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
        : _sampleRate(44100)
    {
        // Allocate buffers for the default 44.1kHz sample rate
        AllocateBuffers();

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
            const AmReal32 resultL = outL * _wet1 + outR * _wet2 + *inputL * _dry;
            const AmReal32 resultR = outR * _wet1 + outL * _wet2 + *inputR * _dry;

            outL = resultL;
            outR = resultR;

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

    void ReverbModel::AllocateBuffers()
    {
        const AmReal32 scale = static_cast<AmReal32>(_sampleRate) / 44100.0f;

        constexpr AmInt32 kCombTuningL[kNumCombs] = {
            kCombTuningL1, kCombTuningL2, kCombTuningL3, kCombTuningL4, kCombTuningL5, kCombTuningL6, kCombTuningL7, kCombTuningL8};
        constexpr AmInt32 kCombTuningR[kNumCombs] = {
            kCombTuningR1, kCombTuningR2, kCombTuningR3, kCombTuningR4, kCombTuningR5, kCombTuningR6, kCombTuningR7, kCombTuningR8};
        constexpr AmInt32 kAllPassTuningL[kNumAllPasses] = { kAllPassTuningL1, kAllPassTuningL2, kAllPassTuningL3, kAllPassTuningL4 };
        constexpr AmInt32 kAllPassTuningR[kNumAllPasses] = { kAllPassTuningR1, kAllPassTuningR2, kAllPassTuningR3, kAllPassTuningR4 };

        for (AmInt32 i = 0; i < kNumCombs; i++)
        {
            const auto sizeL = static_cast<AmInt32>(std::round(kCombTuningL[i] * scale));
            const auto sizeR = static_cast<AmInt32>(std::round(kCombTuningR[i] * scale));
            _bufCombL[i].assign(sizeL, 0.0f);
            _bufCombR[i].assign(sizeR, 0.0f);
            _combL[i].SetBuffer(_bufCombL[i].data(), sizeL);
            _combR[i].SetBuffer(_bufCombR[i].data(), sizeR);
        }

        for (AmInt32 i = 0; i < kNumAllPasses; i++)
        {
            const auto sizeL = static_cast<AmInt32>(std::round(kAllPassTuningL[i] * scale));
            const auto sizeR = static_cast<AmInt32>(std::round(kAllPassTuningR[i] * scale));
            _bufAllPassL[i].assign(sizeL, 0.0f);
            _bufAllPassR[i].assign(sizeR, 0.0f);
            _allPassL[i].SetBuffer(_bufAllPassL[i].data(), sizeL);
            _allPassR[i].SetBuffer(_bufAllPassR[i].data(), sizeR);
        }
    }

    void ReverbModel::SetSampleRate(AmUInt32 sampleRate)
    {
        if (sampleRate == 0)
            sampleRate = 44100;

        if (_sampleRate == sampleRate)
            return;

        _sampleRate = sampleRate;
        AllocateBuffers();
        Mute();
    }

    AmUInt32 ReverbModel::GetSampleRate() const
    {
        return _sampleRate;
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
