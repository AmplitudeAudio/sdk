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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/AmbisonicSource.h>
#include <Ambisonics/BinauralDecodingFilters.h>
#include <DSP/SummingStereoConvolver.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    static SpeakersPreset VirtualSpeakersForOrder(AmUInt32 order)
    {
        if (order <= 1)
            return eSpeakersPreset_CubePoints;

        if (order == 2)
            return eSpeakersPreset_DodecahedronFaces;

        return eSpeakersPreset_LebedevGridOrder26;
    }

    std::shared_ptr<const BinauralDecodingFilters> BinauralDecodingFilters::Build(AmUInt32 order, bool is3D, const HRIRSphere& sphere)
    {
        const AmUInt32 hrirLength = sphere.GetIRLength();
        if (hrirLength == 0)
            return nullptr;

        AmbisonicDecoder decoder;
        if (!decoder.Configure(order, is3D, VirtualSpeakersForOrder(order)))
            return nullptr;

        const AmUInt32 channels = decoder.GetChannelCount();
        if (channels == 0 || channels > SummingStereoConvolver::kMaxInputs)
            return nullptr;

        const AmUInt32 nSpeakers = decoder.GetSpeakerCount();

        // Sample each virtual speaker once; every channel reuses the same HRIRs. The stride keeps each HRIR aligned.
        const AmUInt32 stride = (hrirLength + 15u) & ~15u;
        AmAlignedReal32Buffer speakerHRIR[2];
        speakerHRIR[0].Resize(stride * nSpeakers, true);
        speakerHRIR[1].Resize(stride * nSpeakers, true);

        for (AmUInt32 i = 0; i < nSpeakers; i++)
        {
            const SphericalPosition position = decoder.GetSpeakerPosition(i);
            sphere.Sample(position.ToCartesian(), speakerHRIR[0].GetBuffer() + i * stride, speakerHRIR[1].GetBuffer() + i * stride);
        }

        AudioBuffer accumulated[2] = { AudioBuffer(hrirLength, channels), AudioBuffer(hrirLength, channels) };

        for (AmUInt32 c = 0; c < channels; c++)
        {
            for (AmUInt32 i = 0; i < nSpeakers; i++)
            {
                // The spherical harmonic coefficients are multiplied by (2*order + 1) to provide the correct decoder for
                // SN3D normalized Ambisonic inputs.
                const AmReal32 coefficient =
                    decoder.GetSpeakerCoefficient(i, c) * (2.f * std::floor(std::sqrt(static_cast<AmReal32>(c))) + 1.f);

                ScalarMultiplyAccumulate(speakerHRIR[0].GetBuffer() + i * stride, accumulated[0][c].begin(), coefficient, hrirLength);
                ScalarMultiplyAccumulate(speakerHRIR[1].GetBuffer() + i * stride, accumulated[1][c].begin(), coefficient, hrirLength);
            }
        }

        // Encode a source at azimuth 90deg and elevation 0 and find the maximum coefficient
        AmReal32 max = 0;
        {
            AmbisonicSource source;
            source.Configure(order, true);
            source.SetPosition(SphericalPosition(90.0f * AM_DegToRad, 0.f, 5.f));

            AmAlignedReal32Buffer rightEar90;
            rightEar90.Resize(hrirLength, true);

            for (AmUInt32 c = 0; c < channels; c++)
                ScalarMultiplyAccumulate(accumulated[0][c].begin(), rightEar90.GetBuffer(), source.GetCoefficient(c), hrirLength);

            for (AmUInt32 i = 0; i < hrirLength; i++)
                max = AM_MAX(std::fabs(rightEar90[i]), max);
        }

        if (max <= 0.0f)
            return nullptr;

        // Normalize to pre-defined value
        const AmReal32 scaler = 0.35f / max;
        accumulated[0] *= scaler;
        accumulated[1] *= scaler;

        auto filters = ampoolshared(eMemoryPoolKind_Filtering, BinauralDecodingFilters);
        filters->_order = order;
        filters->_is3D = is3D;
        filters->_channelCount = channels;
        filters->_hrirLength = hrirLength;
        filters->_hrirs[0] = std::move(accumulated[0]);
        filters->_hrirs[1] = std::move(accumulated[1]);

        filters->_planKeeper.Initialize(2 * NextPowerOf2(hrirLength));

        for (AmUInt32 ear = 0; ear < 2; ++ear)
        {
            filters->_kernels[ear].reserve(channels);
            for (AmUInt32 c = 0; c < channels; c++)
                filters->_kernels[ear].push_back(ConvolverKernel::Create(hrirLength, filters->_hrirs[ear][c].begin(), hrirLength));
        }

        return filters;
    }

    AmUInt32 BinauralDecodingFilters::GetOrder() const
    {
        return _order;
    }

    bool BinauralDecodingFilters::Is3D() const
    {
        return _is3D;
    }

    AmUInt32 BinauralDecodingFilters::GetChannelCount() const
    {
        return _channelCount;
    }

    AmUInt32 BinauralDecodingFilters::GetHRIRLength() const
    {
        return _hrirLength;
    }

    const AmReal32* BinauralDecodingFilters::GetHRIR(AmUInt32 ear, AmUInt32 channel) const
    {
        AMPLITUDE_ASSERT(ear < 2 && channel < _channelCount);
        return _hrirs[ear][channel].begin();
    }

    const std::shared_ptr<const ConvolverKernel>& BinauralDecodingFilters::GetKernel(AmUInt32 ear, AmUInt32 channel) const
    {
        AMPLITUDE_ASSERT(ear < 2 && channel < _channelCount);
        return _kernels[ear][channel];
    }

    std::span<const std::shared_ptr<const ConvolverKernel>> BinauralDecodingFilters::GetKernels(AmUInt32 ear) const
    {
        AMPLITUDE_ASSERT(ear < 2);
        return _kernels[ear];
    }
} // namespace SparkyStudios::Audio::Amplitude
