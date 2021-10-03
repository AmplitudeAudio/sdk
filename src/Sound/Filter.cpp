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

#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmUInt32 Filter::GetParamCount()
    {
        return 1;
    }

    AmString Filter::GetParamName(AmUInt32 index)
    {
        return "Wet";
    }

    AmUInt32 Filter::GetParamType(AmUInt32 index)
    {
        return Filter::PARAM_FLOAT;
    }

    float Filter::GetParamMax(AmUInt32 index)
    {
        return 1.0f;
    }

    float Filter::GetParamMin(AmUInt32 index)
    {
        return 0.0f;
    }

    FilterInstance::FilterInstance(Filter* parent)
        : m_parent(parent)
        , m_numParams(0)
        , m_numParamsChanged(0)
        , m_parameters(nullptr)
    {}

    FilterInstance::~FilterInstance()
    {
        delete[] m_parameters;
    }

    AmResult FilterInstance::Init(int numParams)
    {
        delete[] m_parameters;

        m_numParams = numParams;
        m_parameters = new float[numParams];

        if (m_parameters == nullptr)
        {
            delete[] m_parameters;

            m_parameters = nullptr;
            m_numParams = 0;

            return AM_ERROR_OUT_OF_MEMORY;
        }

        for (AmUInt32 i = 0; i < m_numParams; i++)
        {
            m_parameters[i] = 0;
        }

        m_parameters[0] = 1; // Set 'Wet' to 1

        return 0;
    }

    void FilterInstance::AdvanceFrame(AmTime delta_time)
    {}

    void FilterInstance::Process(AmInt16Buffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        if (buffer == nullptr)
            return;

        AmInt16Buffer frame = buffer;

        for (AmUInt64 i = 0; i < frames; i++)
        {
            ProcessFrame(frame, channels, sampleRate);
            frame += channels;
        }
    }

    void FilterInstance::ProcessFrame(AmInt16Buffer frame, AmUInt16 channels, AmUInt32 sampleRate)
    {
        if (frame == nullptr)
            return;

        for (AmUInt16 c = 0; c < channels; c++)
        {
            frame[c] = ProcessSample(frame[c], c, sampleRate);
        }
    }

    AmInt16 FilterInstance::ProcessSample(AmInt16 sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        return sample;
    }

    float FilterInstance::GetFilterParameter(AmUInt32 attributeId)
    {
        if (attributeId >= m_numParams)
            return 0;

        return m_parameters[attributeId];
    }

    void FilterInstance::SetFilterParameter(AmUInt32 attributeId, float value)
    {
        if (attributeId >= m_numParams)
            return;

        m_parameters[attributeId] = value;
        m_numParamsChanged |= 1 << attributeId;
    }
} // namespace SparkyStudios::Audio::Amplitude