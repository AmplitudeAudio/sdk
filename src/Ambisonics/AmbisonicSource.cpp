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

#include <algorithm>

#include <Ambisonics/AmbisonicSource.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicSource::AmbisonicSource()
        : _interpolationDuration(1.0f)
    {}

    AmbisonicSource::~AmbisonicSource() = default;

    bool AmbisonicSource::Configure(AmUInt32 order, bool is3D)
    {
        return AmbisonicEntity::Configure(order, is3D);
    }

    void AmbisonicSource::Refresh()
    {
        AmbisonicEntity::Refresh();
    }

    void AmbisonicSource::SetPosition(const SphericalPosition& position, AmTime duration)
    {
        _interpolationDuration = duration;

        _oldCoefficients.Resize(m_coefficients.GetSize());
        _oldCoefficients.CopyFrom(m_coefficients);

        // Update the coefficients
        AmbisonicEntity::SetPosition(position);
        Refresh();
    }

    void AmbisonicSource::Process(const AudioBufferChannel& input, AmUInt32 samples, BFormat* output)
    {
        if (_interpolationDuration > 0.0f)
        {
            // Number of samples expected per frame
            for (AmUInt32 c = 0; c < m_channelCount; c++)
            {
                auto& channelBuffer = output->_buffer->GetChannel(c);
                const auto nInterpSamples =
                    std::clamp(static_cast<AmUInt32>(std::round(_interpolationDuration * samples)), 1u, samples);
                const AmReal32 deltaCoeff = m_coefficients[c] - _oldCoefficients[c];

                for (AmUInt32 i = 0; i < nInterpSamples; i++)
                {
                    const AmReal32 fInterp = static_cast<AmReal32>(i) / static_cast<AmReal32>(nInterpSamples);
                    channelBuffer[i] = input[i] * (_oldCoefficients[c] + fInterp * deltaCoeff);
                }

                // once interpolation has finished
                ScalarMultiply(
                    input.begin() + nInterpSamples, channelBuffer.begin() + nInterpSamples, m_coefficients[c], samples - nInterpSamples);
            }

            // Set interpolation duration to zero so none is applied on next call
            _interpolationDuration = 0.0f;
        }
        else
        {
            for (AmUInt32 c = 0; c < m_channelCount; c++)
            {
                auto& channelBuffer = output->_buffer->GetChannel(c);
                ScalarMultiply(input.begin(), channelBuffer.begin(), m_coefficients[c], samples);
            }
        }
    }

    void AmbisonicSource::ProcessAccumulate(
        const AudioBufferChannel& input, AmUInt32 samples, BFormat* output, AmUInt32 offset, AmReal32 gain)
    {
        if (_interpolationDuration > 0.0f)
        {
            // Number of samples expected per frame
            for (AmUInt32 c = 0; c < m_channelCount; c++)
            {
                auto& channelBuffer = output->_buffer->GetChannel(c);
                const auto nInterpSamples =
                    std::clamp(static_cast<AmUInt32>(std::round(_interpolationDuration * samples)), 1u, samples);
                const AmReal32 deltaCoeff = m_coefficients[c] - _oldCoefficients[c];

                for (AmUInt32 i = 0; i < nInterpSamples; i++)
                {
                    const AmReal32 fInterp = static_cast<AmReal32>(i) / static_cast<AmReal32>(nInterpSamples);
                    channelBuffer[i + offset] += input[i] * (_oldCoefficients[c] + fInterp * deltaCoeff) * gain;
                }

                // once interpolation has finished
                ScalarMultiplyAccumulate(
                    input.begin() + nInterpSamples, channelBuffer.begin() + offset + nInterpSamples, m_coefficients[c] * gain,
                    samples - nInterpSamples);
            }

            // Set interpolation duration to zero so none is applied on next call
            _interpolationDuration = 0.0f;
        }
        else
        {
            for (AmUInt32 c = 0; c < m_channelCount; c++)
            {
                auto& channelBuffer = output->_buffer->GetChannel(c);
                ScalarMultiplyAccumulate(input.begin(), channelBuffer.begin() + offset, m_coefficients[c] * gain, samples);
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
