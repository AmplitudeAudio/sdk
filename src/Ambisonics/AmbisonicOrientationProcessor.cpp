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

#include <Ambisonics/AmbisonicOrientationProcessor.h>
#include <Ambisonics/BFormat.h>

#define sqrt3 std::sqrt(3.f)
#define sqrt3_2 std::sqrt(3.f / 2.f)
#define sqrt5_2 std::sqrt(5.f / 2.f)
#define sqrt15 std::sqrt(15.f)

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicOrientationProcessor::AmbisonicOrientationProcessor()
        : _orientation(0, 0, 0)
    {}

    AmbisonicOrientationProcessor::~AmbisonicOrientationProcessor()
    {
        _tempSamples.Release();
    }

    bool AmbisonicOrientationProcessor::Configure(AmUInt32 order, bool is3D)
    {
        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        _tempSamples.Resize(m_channelCount);
        return true;
    }

    void AmbisonicOrientationProcessor::Refresh()
    {
        _cosAlpha = std::cos(_orientation.GetAlpha());
        _sinAlpha = std::sin(_orientation.GetAlpha());
        _cosBeta = std::cos(_orientation.GetBeta());
        _sinBeta = std::sin(_orientation.GetBeta());
        _cosGamma = std::cos(_orientation.GetGamma());
        _sinGamma = std::sin(_orientation.GetGamma());

        _cos2Alpha = std::cos(2.0f * _orientation.GetAlpha());
        _sin2Alpha = std::sin(2.0f * _orientation.GetAlpha());
        _cos2Beta = std::cos(2.0f * _orientation.GetBeta());
        _sin2Beta = std::sin(2.0f * _orientation.GetBeta());
        _cos2Gamma = std::cos(2.0f * _orientation.GetGamma());
        _sin2Gamma = std::sin(2.0f * _orientation.GetGamma());

        _cos3Alpha = std::cos(3.0f * _orientation.GetAlpha());
        _sin3Alpha = std::sin(3.0f * _orientation.GetAlpha());
        _cos3Beta = std::cos(3.0f * _orientation.GetBeta());
        _sin3Beta = std::sin(3.0f * _orientation.GetBeta());
        _cos3Gamma = std::cos(3.0f * _orientation.GetGamma());
        _sin3Gamma = std::sin(3.0f * _orientation.GetGamma());
    }

    void AmbisonicOrientationProcessor::Reset()
    {
        // noop
    }

    void AmbisonicOrientationProcessor::Process(BFormat* input, AmUInt32 samples)
    {
        if (!m_is3D)
            return; // 3D input expected

        if (m_order >= 1)
            ProcessOrder1(input, samples);
        if (m_order >= 2)
            ProcessOrder2(input, samples);
        if (m_order >= 3)
            ProcessOrder3(input, samples);
    }

    void AmbisonicOrientationProcessor::ProcessOrder1(BFormat* input, AmUInt32 samples)
    {
        auto& xChannel = input->GetBufferChannel(eBFormatChannel_X);
        auto& yChannel = input->GetBufferChannel(eBFormatChannel_Y);
        auto& zChannel = input->GetBufferChannel(eBFormatChannel_Z);

        for (AmUInt32 i = 0; i < samples; ++i)
        {
            // clang-format off
            // Alpha rotation
            _tempSamples[eBFormatChannel_X] = xChannel[i] * _cosAlpha + yChannel[i] * _sinAlpha;
            _tempSamples[eBFormatChannel_Y] = yChannel[i] * _cosAlpha - xChannel[i] * _sinAlpha;
            _tempSamples[eBFormatChannel_Z] = zChannel[i];

            // Beta rotation
            xChannel[i] = _tempSamples[eBFormatChannel_X] * _cosBeta - _tempSamples[eBFormatChannel_Z] * _sinBeta;
            yChannel[i] = _tempSamples[eBFormatChannel_Y];
            zChannel[i] = _tempSamples[eBFormatChannel_Z] * _cosBeta + _tempSamples[eBFormatChannel_X] * _sinBeta;

            // Gamma rotation
            _tempSamples[eBFormatChannel_X] = xChannel[i] * _cosGamma + yChannel[i] * _sinGamma;
            _tempSamples[eBFormatChannel_Y] = yChannel[i] * _cosGamma - xChannel[i] * _sinGamma;
            _tempSamples[eBFormatChannel_Z] = zChannel[i];

            // Save results
            xChannel[i] = _tempSamples[eBFormatChannel_X];
            yChannel[i] = _tempSamples[eBFormatChannel_Y];
            zChannel[i] = _tempSamples[eBFormatChannel_Z];
            // clang-format on
        }
    }

    void AmbisonicOrientationProcessor::ProcessOrder2(BFormat* input, AmUInt32 samples)
    {
        auto& rChannel = input->GetBufferChannel(eBFormatChannel_R);
        auto& sChannel = input->GetBufferChannel(eBFormatChannel_S);
        auto& tChannel = input->GetBufferChannel(eBFormatChannel_T);
        auto& uChannel = input->GetBufferChannel(eBFormatChannel_U);
        auto& vChannel = input->GetBufferChannel(eBFormatChannel_V);

        for (AmUInt32 i = 0; i < samples; ++i)
        {
            // clang-format off
            // Alpha rotation
            _tempSamples[eBFormatChannel_R] = rChannel[i];
            _tempSamples[eBFormatChannel_S] = sChannel[i] * _cosAlpha + tChannel[i] * _sinAlpha;
            _tempSamples[eBFormatChannel_T] = tChannel[i] * _cosAlpha - sChannel[i] * _sinAlpha;
            _tempSamples[eBFormatChannel_U] = uChannel[i] * _cos2Alpha + vChannel[i] * _sin2Alpha;
            _tempSamples[eBFormatChannel_V] = vChannel[i] * _cos2Alpha - uChannel[i] * _sin2Alpha;

            // Beta rotation
            rChannel[i] = _tempSamples[eBFormatChannel_R] * (0.75f * _cosBeta + 0.25f) + _tempSamples[eBFormatChannel_U] * (0.5f * sqrt3 * std::pow(_sinBeta, 2.0f)) + _tempSamples[eBFormatChannel_S] * (sqrt3 * _sinBeta * _cosBeta);
            sChannel[i] = _tempSamples[eBFormatChannel_S] * _cos2Beta - _tempSamples[eBFormatChannel_R] * _cosBeta * _sinBeta * sqrt3 + _tempSamples[eBFormatChannel_U] * _cosBeta * _sinBeta;
            tChannel[i] = _tempSamples[eBFormatChannel_V] * _sinBeta - _tempSamples[eBFormatChannel_T] * _cosBeta;
            uChannel[i] = _tempSamples[eBFormatChannel_U] * (0.25f * _cos2Beta + 0.75f) - _tempSamples[eBFormatChannel_S] * _cosBeta * _sinBeta + _tempSamples[eBFormatChannel_R] * (0.5f * sqrt3 * std::pow(_sinBeta, 2.0f));
            vChannel[i] = _tempSamples[eBFormatChannel_V] * _cosBeta - _tempSamples[eBFormatChannel_T] * _sinBeta;

            // Gamma rotation
            _tempSamples[eBFormatChannel_R] = rChannel[i];
            _tempSamples[eBFormatChannel_S] = sChannel[i] * _cosGamma + tChannel[i] * _sinGamma;
            _tempSamples[eBFormatChannel_T] = tChannel[i] * _cosGamma - sChannel[i] * _sinGamma;
            _tempSamples[eBFormatChannel_U] = uChannel[i] * _cos2Gamma + vChannel[i] * _sin2Gamma;
            _tempSamples[eBFormatChannel_V] = vChannel[i] * _cos2Gamma - uChannel[i] * _sin2Gamma;

            // Save results
            rChannel[i] = _tempSamples[eBFormatChannel_R];
            sChannel[i] = _tempSamples[eBFormatChannel_S];
            tChannel[i] = _tempSamples[eBFormatChannel_T];
            uChannel[i] = _tempSamples[eBFormatChannel_U];
            vChannel[i] = _tempSamples[eBFormatChannel_V];
            // clang-format on
        }
    }

    void AmbisonicOrientationProcessor::ProcessOrder3(BFormat* input, AmUInt32 samples)
    {
        auto& kChannel = input->GetBufferChannel(eBFormatChannel_K);
        auto& lChannel = input->GetBufferChannel(eBFormatChannel_L);
        auto& mChannel = input->GetBufferChannel(eBFormatChannel_M);
        auto& nChannel = input->GetBufferChannel(eBFormatChannel_N);
        auto& oChannel = input->GetBufferChannel(eBFormatChannel_O);
        auto& pChannel = input->GetBufferChannel(eBFormatChannel_P);
        auto& qChannel = input->GetBufferChannel(eBFormatChannel_Q);

        for (AmUInt32 i = 0; i < samples; i++)
        {
            // clang-format off
            // Alpha rotation
            _tempSamples[eBFormatChannel_K] = kChannel[i];
            _tempSamples[eBFormatChannel_L] = lChannel[i] * _cosAlpha + mChannel[i] * _sinAlpha;
            _tempSamples[eBFormatChannel_M] = mChannel[i] * _cosAlpha - lChannel[i] * _sinAlpha;
            _tempSamples[eBFormatChannel_N] = nChannel[i] * _cos2Alpha + oChannel[i] * _sin2Alpha;
            _tempSamples[eBFormatChannel_O] = oChannel[i] * _cos2Alpha - nChannel[i] * _sin2Alpha;
            _tempSamples[eBFormatChannel_P] = pChannel[i] * _cos3Alpha + qChannel[i] * _sin3Alpha;
            _tempSamples[eBFormatChannel_Q] = qChannel[i] * _cos3Alpha - pChannel[i] * _sin3Alpha;

            // Beta rotation
            qChannel[i] = 0.125f * _tempSamples[eBFormatChannel_Q] * (5.f + 3.f * _cos2Beta) - sqrt3_2 * _tempSamples[eBFormatChannel_O] * _cosBeta * _sinBeta + 0.25f * sqrt15 * _tempSamples[eBFormatChannel_M] * powf(_sinBeta, 2.0f);
            oChannel[i] = _tempSamples[eBFormatChannel_O] * _cos2Beta - sqrt5_2 * _tempSamples[eBFormatChannel_M] * _cosBeta * _sinBeta + sqrt3_2 * _tempSamples[eBFormatChannel_Q] * _cosBeta * _sinBeta;
            mChannel[i] = 0.125f * _tempSamples[eBFormatChannel_M] * (3.f + 5.f * _cos2Beta) - sqrt5_2 * _tempSamples[eBFormatChannel_O] * _cosBeta * _sinBeta + 0.25f * sqrt15 * _tempSamples[eBFormatChannel_Q] * powf(_sinBeta, 2.0f);
            kChannel[i] = 0.25f * _tempSamples[eBFormatChannel_K] * _cosBeta * (-1.f + 15.f * _cos2Beta) + 0.5f * sqrt15 * _tempSamples[eBFormatChannel_N] * _cosBeta * powf(_sinBeta, 2.f) + 0.5f * sqrt5_2 * _tempSamples[eBFormatChannel_P] * powf(_sinBeta, 3.f) + 0.125f * sqrt3_2 * _tempSamples[eBFormatChannel_L] * (_sinBeta + 5.f * _sin3Beta);
            lChannel[i] = 0.0625f * _tempSamples[eBFormatChannel_L] * (_cosBeta + 15.f * _cos3Beta) + 0.25f * sqrt5_2 * _tempSamples[eBFormatChannel_N] * (1.f + 3.f * _cos2Beta) * _sinBeta + 0.25f * sqrt15 * _tempSamples[eBFormatChannel_P] * _cosBeta * powf(_sinBeta, 2.f) - 0.125f * sqrt3_2 * _tempSamples[eBFormatChannel_K] * (_sinBeta + 5.f * _sin3Beta);
            nChannel[i] = 0.125f * _tempSamples[eBFormatChannel_N] * (5.f * _cosBeta + 3.f * _cos3Beta) + 0.25f * sqrt3_2 * _tempSamples[eBFormatChannel_P] * (3.f + _cos2Beta) * _sinBeta + 0.5f * sqrt15 * _tempSamples[eBFormatChannel_K] * _cosBeta * powf(_sinBeta, 2.f) + 0.125f * sqrt5_2 * _tempSamples[eBFormatChannel_L] * (_sinBeta - 3.f * _sin3Beta);
            pChannel[i] = 0.0625f * _tempSamples[eBFormatChannel_P] * (15.f * _cosBeta + _cos3Beta) - 0.25f * sqrt3_2 * _tempSamples[eBFormatChannel_N] * (3.f + _cos2Beta) * _sinBeta + 0.25f * sqrt15 * _tempSamples[eBFormatChannel_L] * _cosBeta * powf(_sinBeta, 2.f) - 0.5f * sqrt5_2 * _tempSamples[eBFormatChannel_K] * powf(_sinBeta, 3.f);

            // Gamma rotation
            _tempSamples[eBFormatChannel_K] = kChannel[i];
            _tempSamples[eBFormatChannel_L] = lChannel[i] * _cosGamma + mChannel[i] * _sinGamma;
            _tempSamples[eBFormatChannel_M] = mChannel[i] * _cosGamma - lChannel[i] * _sinGamma;
            _tempSamples[eBFormatChannel_N] = nChannel[i] * _cos2Gamma + oChannel[i] * _sin2Gamma;
            _tempSamples[eBFormatChannel_O] = oChannel[i] * _cos2Gamma - nChannel[i] * _sin2Gamma;
            _tempSamples[eBFormatChannel_P] = pChannel[i] * _cos3Gamma + qChannel[i] * _sin3Gamma;
            _tempSamples[eBFormatChannel_Q] = qChannel[i] * _cos3Gamma - pChannel[i] * _sin3Gamma;

            // Save results
            kChannel[i] = _tempSamples[eBFormatChannel_K];
            lChannel[i] = _tempSamples[eBFormatChannel_L];
            mChannel[i] = _tempSamples[eBFormatChannel_M];
            nChannel[i] = _tempSamples[eBFormatChannel_N];
            oChannel[i] = _tempSamples[eBFormatChannel_O];
            pChannel[i] = _tempSamples[eBFormatChannel_P];
            qChannel[i] = _tempSamples[eBFormatChannel_Q];
            // clang-format off
        }
    }
}
