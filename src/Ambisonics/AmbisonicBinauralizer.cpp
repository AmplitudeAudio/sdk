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

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <Ambisonics/AmbisonicBinauralizer.h>
#include <Ambisonics/AmbisonicSource.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmUInt32 kInterpolationBlockSize = 128;

    AmbisonicBinauralizer::AmbisonicBinauralizer()
        : AmbisonicComponent()
        , _hrir(nullptr)
    {}

    bool AmbisonicBinauralizer::Configure(AmUInt32 order, bool is3D, const HRIRSphere* hrirSphere)
    {
        if (hrirSphere == nullptr)
            return false;

        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        _hrir = hrirSphere;
        const AmUInt32 hrirLength = _hrir->GetIRLength();

        SetUpSpeakers();
        const AmUInt32 nSpeakers = _decoder.GetSpeakerCount();

        // Setup left and right accumulated HRTFs
        _accumulatedHRIR[0] = AudioBuffer(hrirLength, m_channelCount);
        _accumulatedHRIR[1] = AudioBuffer(hrirLength, m_channelCount);

        // Sample HRIR values for each speaker
        {
            AmAlignedReal32Buffer hrir[2];
            hrir[0].Resize(hrirLength, true);
            hrir[1].Resize(hrirLength, true);

            for (AmUInt32 c = 0; c < m_channelCount; c++)
            {
                auto& leftChannel = _accumulatedHRIR[0][c];
                auto& rightChannel = _accumulatedHRIR[1][c];

                for (AmUInt32 i = 0; i < nSpeakers; i++)
                {
                    // What is the position of the current speaker
                    SphericalPosition position = _decoder.GetSpeakerPosition(i);

                    hrir[0].Clear();
                    hrir[1].Clear();

                    _hrir->SampleBilinear(position.ToCartesian(), hrir[0].GetBuffer(), hrir[1].GetBuffer());

                    // Scale the HRTFs by the coefficient of the current channel/component
                    //  The spherical harmonic coefficients are multiplied by (2*order + 1) to provide the correct decoder
                    //  for SN3D normalized Ambisonic inputs.
                    const AmReal32 coefficient =
                        _decoder.GetSpeakerCoefficient(i, c) * (2.f * std::floor(std::sqrt(static_cast<AmReal32>(c))) + 1.f);

                    ScalarMultiplyAccumulate(hrir[0].GetBuffer(), leftChannel.begin(), coefficient, hrirLength);
                    ScalarMultiplyAccumulate(hrir[1].GetBuffer(), rightChannel.begin(), coefficient, hrirLength);
                }
            }
        }

        // Encode a source at azimuth 90deg and elevation 0 and find the maximum coefficient
        AmReal32 max = 0;
        {
            AmbisonicSource source;
            source.Configure(m_order, true);

            const SphericalPosition position90(90.0f * AM_DegToRad, 0.f, 5.f);
            source.SetPosition(position90);

            AmAlignedReal32Buffer rightEar90;
            rightEar90.Resize(hrirLength, true);

            for (AmUInt32 c = 0; c < m_channelCount; c++)
            {
                const auto& accumulatedHRIRChannel = _accumulatedHRIR[0][c];
                ScalarMultiplyAccumulate(accumulatedHRIRChannel.begin(), rightEar90.GetBuffer(), source.GetCoefficient(c), hrirLength);
            }

            for (AmUInt32 i = 0; i < hrirLength; i++)
            {
                const AmReal32 val = std::fabs(rightEar90[i]);
                max = AM_MAX(val, max);
            }
        }

        // Normalize to pre-defined value
        const AmReal32 scaler = 0.35f / max;

        _accumulatedHRIR[0] *= scaler;
        _accumulatedHRIR[1] *= scaler;

        for (AmUInt32 c = 0; c < m_channelCount; c++)
        {
            _convL[c].Init(kInterpolationBlockSize, _accumulatedHRIR[0][c].begin(), _hrir->GetIRLength());
            _convR[c].Init(kInterpolationBlockSize, _accumulatedHRIR[1][c].begin(), _hrir->GetIRLength());
        }

        return true;
    }

    void AmbisonicBinauralizer::Reset()
    {}

    void AmbisonicBinauralizer::Refresh()
    {}

    void AmbisonicBinauralizer::Process(BFormat* input, AmUInt32 samples, AudioBuffer& output)
    {
        AudioBuffer scratch(samples, 2);
        auto& scratchL = scratch[0];
        auto& scratchR = scratch[1];

        auto& outputL = output[0];
        auto& outputR = output[1];

        for (AmUInt32 c = 0; c < m_channelCount; ++c)
        {
            const auto& inputChannel = input->GetBufferChannel(c);

            _convL[c].Process(inputChannel.begin(), scratchL.begin(), samples);
            _convR[c].Process(inputChannel.begin(), scratchR.begin(), samples);

            outputL += scratchL;
            outputR += scratchR;
        }
    }

    void AmbisonicBinauralizer::SetUpSpeakers()
    {
        SpeakersPreset preset;

        if (m_order <= 1)
            preset = eSpeakersPreset_CubePoints;
        else if (m_order == 2)
            preset = eSpeakersPreset_DodecahedronFaces;
        else
            preset = eSpeakersPreset_LebedevGridOrder26;

        _decoder.Configure(m_order, m_is3D, preset);
    }
} // namespace SparkyStudios::Audio::Amplitude
