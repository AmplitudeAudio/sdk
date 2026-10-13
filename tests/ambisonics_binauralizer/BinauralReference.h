// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_TESTS_BINAURAL_REFERENCE_H
#define _AM_TESTS_BINAURAL_REFERENCE_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/AmbisonicSource.h>
#include <HRTF/HRIRSphere.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The pre-change per-channel accumulation of AmbisonicBinauralizer::Configure: the reference both for the filter
    // build and for the binaural output.
    inline void BuildReferenceHRIRs(AmUInt32 order, bool is3D, const HRIRSphere& sphere, AudioBuffer (&out)[2])
    {
        const AmUInt32 hrirLength = sphere.GetIRLength();

        AmbisonicDecoder decoder;
        const SpeakersPreset preset =
            order <= 1 ? eSpeakersPreset_CubePoints : (order == 2 ? eSpeakersPreset_DodecahedronFaces : eSpeakersPreset_LebedevGridOrder26);
        decoder.Configure(order, is3D, preset);

        const AmUInt32 channels = decoder.GetChannelCount();
        const AmUInt32 nSpeakers = decoder.GetSpeakerCount();

        out[0] = AudioBuffer(hrirLength, channels);
        out[1] = AudioBuffer(hrirLength, channels);

        AmAlignedReal32Buffer hrir[2];
        hrir[0].Resize(hrirLength, true);
        hrir[1].Resize(hrirLength, true);

        for (AmUInt32 c = 0; c < channels; c++)
        {
            for (AmUInt32 i = 0; i < nSpeakers; i++)
            {
                const SphericalPosition position = decoder.GetSpeakerPosition(i);

                hrir[0].Clear();
                hrir[1].Clear();

                sphere.Sample(position.ToCartesian(), hrir[0].GetBuffer(), hrir[1].GetBuffer());

                const AmReal32 coefficient =
                    decoder.GetSpeakerCoefficient(i, c) * (2.f * std::floor(std::sqrt(static_cast<AmReal32>(c))) + 1.f);

                ScalarMultiplyAccumulate(hrir[0].GetBuffer(), out[0][c].begin(), coefficient, hrirLength);
                ScalarMultiplyAccumulate(hrir[1].GetBuffer(), out[1][c].begin(), coefficient, hrirLength);
            }
        }

        AmReal32 max = 0;
        {
            AmbisonicSource source;
            source.Configure(order, true);
            source.SetPosition(SphericalPosition(90.0f * AM_DegToRad, 0.f, 5.f));

            AmAlignedReal32Buffer rightEar90;
            rightEar90.Resize(hrirLength, true);

            for (AmUInt32 c = 0; c < channels; c++)
                ScalarMultiplyAccumulate(out[0][c].begin(), rightEar90.GetBuffer(), source.GetCoefficient(c), hrirLength);

            for (AmUInt32 i = 0; i < hrirLength; i++)
                max = AM_MAX(std::fabs(rightEar90[i]), max);
        }

        const AmReal32 scaler = 0.35f / max;
        out[0] *= scaler;
        out[1] *= scaler;
    }

    inline HRIRSphereImpl* LoadTestSphere(HRIRSphereImpl& sphere)
    {
        auto fs = CreatePlatformFileSystem();
        fs->SetBasePath(GetPlatformAssetsBasePath());
        sphere.SetResource(AM_OS_STRING("./data/sadie_h12.amir"));
        sphere.Load(fs);
        return sphere.IsLoaded() ? &sphere : nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_BINAURAL_REFERENCE_H
