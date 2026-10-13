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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/AmbisonicSpeaker.h>
#include <Utils/lebedev-quadrature/lebedev_quadrature.hpp>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    // Decoder coefficients for Ambisonic to stereo. Useful for conversion to 2-channels when not using headphone.
    constexpr AmReal32 kDecoderCoefficientStereo[][16] = {
        { 0.5f, 0.5f / 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
        { 0.5f, -0.5f / 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
    };

    AmbisonicDecoder::AmbisonicDecoder()
        : _speakersPreset(eSpeakerSetUp_COUNT)
        , _speakerCount(0)
        , _speakers()
        , _isLoaded(false)
    {}

    AmbisonicDecoder::~AmbisonicDecoder()
    {
        _speakers.clear();
    }

    bool AmbisonicDecoder::Configure(AmUInt32 order, bool is3D, SpeakersPreset setUp, AmUInt32 speakerCount)
    {
        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        SetUpSpeakers(setUp, speakerCount);
        Refresh();

        return true;
    }

    void AmbisonicDecoder::Reset()
    {
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].Reset();
    }

    void AmbisonicDecoder::Refresh()
    {
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].Refresh();

        DetectSpeakersPreset();
        LoadDecoderPreset();
    }

    void AmbisonicDecoder::Process(BFormat* input, AmUInt32 samples, AudioBuffer& output)
    {
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].Process(input, samples, output[i]);
    }

    void AmbisonicDecoder::SetSpeakerPosition(AmUInt32 speaker, const SphericalPosition& position)
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        _speakers[speaker].SetPosition(position);
    }

    SphericalPosition AmbisonicDecoder::GetSpeakerPosition(AmUInt32 speaker) const
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        return _speakers[speaker].GetPosition();
    }

    void AmbisonicDecoder::SetSpeakerOrderWeight(AmUInt32 speaker, AmUInt32 order, AmReal32 weight)
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        _speakers[speaker].SetOrderWeight(order, weight);
    }

    AmReal32 AmbisonicDecoder::GetSpeakerOrderWeight(AmUInt32 speaker, AmUInt32 order) const
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        return _speakers[speaker].GetOrderWeight(order);
    }

    void AmbisonicDecoder::SetSpeakerCoefficient(AmUInt32 speaker, AmUInt32 channel, AmReal32 coefficient)
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        _speakers[speaker].SetCoefficient(channel, coefficient);
    }

    AmReal32 AmbisonicDecoder::GetSpeakerCoefficient(AmUInt32 speaker, AmUInt32 channel) const
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        return _speakers[speaker].GetCoefficient(channel);
    }

    void AmbisonicDecoder::SetUpSpeakers(SpeakersPreset setUp, AmUInt32 speakerCount)
    {
        _speakersPreset = setUp;

        _speakers.clear();

        _isLoaded = false;

        switch (_speakersPreset)
        {
        case eSpeakersPreset_Custom:
            _speakerCount = speakerCount;
            _speakers.resize(_speakerCount);
            for (AmUInt32 i = 0; i < _speakerCount; ++i)
                _speakers[i].Configure(m_order, m_is3D);
            break;

        case eSpeakersPreset_Mono:
            _speakerCount = 1;
            _speakers.resize(_speakerCount);
            _speakers[0].Configure(m_order, m_is3D);
            _speakers[0].SetPosition({ 0.0f, 0.0f, 1.0f });
            break;

        case eSpeakersPreset_Stereo:
            _speakerCount = 2;
            _speakers.resize(_speakerCount);
            _speakers[0].Configure(m_order, m_is3D);
            _speakers[0].SetPosition({ +30.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[1].Configure(m_order, m_is3D);
            _speakers[1].SetPosition({ -30.0f * AM_DegToRad, 0.0f, 1.0f });
            break;

        case eSpeakersPreset_CubePoints:
            {
                _speakerCount = 8;
                _speakers.resize(_speakerCount);
                SphericalPosition position = { 0.0f, +35.2f * AM_DegToRad, 1.0f };
                for (AmUInt32 i = 0; i < 4; ++i)
                {
                    position.SetAzimuth(-(static_cast<AmReal32>(i) * 360.f / 4.0f + 45.f) * AM_DegToRad);
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition(position);
                }
                position.SetElevation(-35.2f * AM_DegToRad);
                for (AmUInt32 i = 4; i < 8; ++i)
                {
                    position.SetAzimuth(-(static_cast<AmReal32>(i - 4) * 360.f / 4.0f + 45.f) * AM_DegToRad);
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition(position);
                }
                break;
            }

        case eSpeakersPreset_DodecahedronFaces:
            {
                // The 20 vertices of a regular dodecahedron (elevation, azimuth in degrees): a degree-5 t-design, exact for
                // a second-order decode, with height.
                constexpr AmReal32 kVertices[20][2] = { { -69.1f, 90.0f },  { -69.1f, -90.0f }, { -35.3f, 45.0f },  { -35.3f, 135.0f },
                                                        { -35.3f, -45.0f }, { -35.3f, -135.0f }, { -20.9f, 180.0f }, { -20.9f, 0.0f },
                                                        { 0.0f, 69.1f },    { 0.0f, 110.9f },   { 0.0f, -69.1f },   { 0.0f, -110.9f },
                                                        { 20.9f, 180.0f },  { 20.9f, 0.0f },    { 35.3f, 45.0f },   { 35.3f, 135.0f },
                                                        { 35.3f, -45.0f },  { 35.3f, -135.0f }, { 69.1f, 90.0f },   { 69.1f, -90.0f } };

                _speakerCount = 20;
                _speakers.resize(_speakerCount);
                for (AmUInt32 i = 0; i < _speakerCount; ++i)
                {
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition({ kVertices[i][1] * AM_DegToRad, kVertices[i][0] * AM_DegToRad, 1.0f });
                }
                break;
            }

        case eSpeakersPreset_LebedevGridOrder26:
            {
                _speakerCount = 26;
                _speakers.resize(_speakerCount);

                const auto quad_points = lebedev::generate_quadrature_points(lebedev::QuadratureOrder::order_26);

                const auto& xAxis = std::get<0>(quad_points);
                const auto& yAxis = std::get<1>(quad_points);
                const auto& zAxis = std::get<2>(quad_points);

                for (AmUInt32 i = 0; i < 26; ++i)
                {
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition(SphericalPosition::ForHRTF(AmVector3{
                        static_cast<AmReal32>(xAxis.at(i)), static_cast<AmReal32>(yAxis.at(i)), static_cast<AmReal32>(zAxis.at(i)) }));
                }
                break;
            }

        default:
            amLogError("Invalid speaker setup: %u", static_cast<unsigned int>(_speakersPreset));
            AMPLITUDE_ASSERT(false);
            break;
        }

        const AmReal32 speakerGain = 1.0f / static_cast<AmReal32>(_speakerCount);
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].SetGain(speakerGain);
    }

    void AmbisonicDecoder::DetectSpeakersPreset()
    {
        if (_speakersPreset != eSpeakersPreset_Custom)
            return;

        AmUInt32 speakerMatchCount = 0;

        // clang-format off
        constexpr AmReal32 azimuthStereo[] = { 30.0f * AM_DegToRad, -30.0f * AM_DegToRad };
        // clang-format on

        switch (_speakerCount)
        {
        case 1:
            _speakersPreset = eSpeakersPreset_Mono;
            break;

        case 2:
            for (AmUInt32 i = 0; i < 2; ++i)
                if (const auto position = _speakers[i].GetPosition(); position.GetElevation() == 0.0f)
                    if (std::abs(position.GetAzimuth() - azimuthStereo[i]) < kEpsilon)
                        speakerMatchCount++;

            if (speakerMatchCount == 2)
                _speakersPreset = eSpeakersPreset_Stereo;
            break;

        default:
            break;
        }
    }

    void AmbisonicDecoder::LoadDecoderPreset()
    {
        const AmUInt32 ambisonicComponents = OrderToComponents(m_order, m_is3D);

        switch (_speakersPreset)
        {
        case eSpeakersPreset_Mono:
            [[fallthrough]];
        case eSpeakersPreset_CubePoints:
            [[fallthrough]];
        case eSpeakersPreset_DodecahedronFaces:
            [[fallthrough]];
        case eSpeakersPreset_LebedevGridOrder26:
            _isLoaded = true;
            break;

        case eSpeakersPreset_Stereo:
            {
                for (AmUInt32 i = 0; i < 2; ++i)
                    for (AmUInt32 j = 0; j < ambisonicComponents; ++j)
                        _speakers[i].SetCoefficient(j, kDecoderCoefficientStereo[i][j]);

                _isLoaded = true;
            }
            break;

        default:
            _isLoaded = false;
            break;
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
