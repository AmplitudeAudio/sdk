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

#include <Sound/Filters/RobotizeFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    RobotizeFilter::RobotizeFilter()
        : m_frequency(30)
        , m_waveform(0)
    {}

    AmResult RobotizeFilter::Init(float frequency, AmInt32 waveform)
    {
        m_frequency = frequency;
        m_waveform = waveform;

        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 RobotizeFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString RobotizeFilter::GetParamName(AmUInt32 index)
    {
        if (index >= ATTRIBUTE_LAST)
            return nullptr;

        AmString names[3] = { "Wet", "Frequency", "Waveform" };
        return names[index];
    }

    AmUInt32 RobotizeFilter::GetParamType(AmUInt32 index)
    {
        if (index == ATTRIBUTE_WAVEFORM)
            return PARAM_INT;

        return PARAM_FLOAT;
    }

    float RobotizeFilter::GetParamMax(AmUInt32 index)
    {
        if (index == ATTRIBUTE_WAVEFORM)
            return WAVE_LAST - 1;
        if (index == ATTRIBUTE_FREQUENCY)
            return 100.0f;

        return 1.0f;
    }

    float RobotizeFilter::GetParamMin(AmUInt32 index)
    {
        if (index == ATTRIBUTE_FREQUENCY)
            return 0.1f;

        return 0.0f;
    }

    FilterInstance* RobotizeFilter::CreateInstance()
    {
        return new RobotizeFilterInstance(this);
    }

    RobotizeFilterInstance::RobotizeFilterInstance(RobotizeFilter* parent)
        : FilterInstance(parent)
    {
        Init(3);
        _duration = 0.0;
        m_parameters[RobotizeFilter::ATTRIBUTE_FREQUENCY] = parent->m_frequency;
        m_parameters[RobotizeFilter::ATTRIBUTE_WAVEFORM] = static_cast<float>(parent->m_waveform);
    }

    void RobotizeFilterInstance::AdvanceFrame(AmTime deltaTime)
    {
        _duration += deltaTime;
        FilterInstance::AdvanceFrame(deltaTime);
    }

    void RobotizeFilterInstance::Process(
        AmInt16Buffer buffer, AmUInt64 samples, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        for (AmUInt64 c = 0; c < channels; c++)
        {
            const auto period = static_cast<AmInt32>(static_cast<float>(sampleRate) / m_parameters[RobotizeFilter::ATTRIBUTE_FREQUENCY]);
            const auto start = static_cast<AmInt32>(_duration * sampleRate) % period;

            for (AmUInt32 i = c; i < samples * channels; i += channels)
            {
                const AmInt32 x = buffer[i];
                /* */ AmInt32 y;

                const float wPos = static_cast<float>((start + i) % period) / static_cast<float>(period);

                // clang-format off
                y = x * AmFloatToFixedPoint(GenerateWaveform(static_cast<AmInt32>(m_parameters[RobotizeFilter::ATTRIBUTE_WAVEFORM]), wPos)) + AmFloatToFixedPoint(0.5f) >> kAmFixedPointShift;
                // clang-format on
                y = x + ((y - x) * AmFloatToFixedPoint(m_parameters[RobotizeFilter::ATTRIBUTE_WET]) >> kAmFixedPointShift);
                y = AM_CLAMP(y, INT16_MIN, INT16_MAX);

                buffer[i] = static_cast<AmInt16>(y);
            }
        }
    }

    float RobotizeFilterInstance::GenerateWaveform(AmInt32 waveform, float p)
    {
        switch (waveform)
        {
        default:
        case RobotizeFilter::WAVE_SQUARE:
            return p > 0.5f ? 0.5f : -0.5f;
        case RobotizeFilter::WAVE_SAW:
            return p - 0.5f;
        case RobotizeFilter::WAVE_SIN:
            return std::sinf(p * M_PI * 2.0f) * 0.5f;
        case RobotizeFilter::WAVE_TRIANGLE:
            return (p > 0.5f ? (1.0f - (p - 0.5f) * 2) : p * 2.0f) - 0.5f;
        case RobotizeFilter::WAVE_BOUNCE:
            return (p < 0.5f ? std::sinf(p * M_PI * 2.0f) * 0.5f : -std::sinf(p * M_PI * 2.0f) * 0.5f) - 0.5f;
        case RobotizeFilter::WAVE_JAWS:
            return (p < 0.25f ? std::sinf(p * M_PI * 2.0f) * 0.5f : 0) - 0.5f;
        case RobotizeFilter::WAVE_HUMPS:
            return (p < 0.5f ? std::sinf(p * M_PI * 2.0f) * 0.5f : 0) - 0.5f;
        case RobotizeFilter::WAVE_FSQUARE:
            {
                float f = 0;
                for (int i = 1; i < 22; i += 2)
                {
                    f += 4.0f / (M_PI * i) * std::sinf(2 * M_PI * i * p);
                }
                return f * 0.5f;
            }
        case RobotizeFilter::WAVE_FSAW:
            {
                float f = 0;
                for (int i = 1; i < 15; i++)
                {
                    if (i & 1)
                        f += 1.0f / (M_PI * i) * std::sinf(p * 2 * M_PI * i);
                    else
                        f -= 1.0f / (M_PI * i) * std::sinf(p * 2 * M_PI * i);
                }
                return f;
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
