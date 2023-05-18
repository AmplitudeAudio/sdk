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
    typedef std::map<std::string, Filter*> FilterRegistry;
    typedef FilterRegistry::value_type FilterImpl;

    static FilterRegistry& filterRegistry()
    {
        static FilterRegistry r;
        return r;
    }

    static bool& lockFilters()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& filtersCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    Filter::Filter(std::string name)
        : m_name(std::move(name))
    {
        Filter::Register(this);
    }

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

    AmReal32 Filter::GetParamMax(AmUInt32 index)
    {
        return 1.0f;
    }

    AmReal32 Filter::GetParamMin(AmUInt32 index)
    {
        return 0.0f;
    }

    const std::string& Filter::GetName() const
    {
        return m_name;
    }

    void Filter::Register(Filter* codec)
    {
        if (lockFilters())
            return;

        if (Find(codec->GetName()) != nullptr)
            return;

        FilterRegistry& filters = filterRegistry();
        filters.insert(FilterImpl(codec->GetName(), codec));
        filtersCount()++;
    }

    Filter* Filter::Find(const std::string& name)
    {
        FilterRegistry& filters = filterRegistry();
        for (auto&& filter : filters)
            if (filter.second->m_name == name)
                return filter.second;

        return nullptr;
    }

    FilterInstance* Filter::Construct(const std::string& name)
    {
        Filter* filter = Find(name);
        if (filter == nullptr)
            return nullptr;

        return filter->CreateInstance();
    }

    void Filter::Destruct(const std::string& name, FilterInstance* instance)
    {
        if (instance == nullptr)
            return;

        Filter* filter = Find(name);
        if (filter == nullptr)
            return;

        filter->DestroyInstance(instance);
    }

    void Filter::LockRegistry()
    {
        lockFilters() = true;
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

    AmResult FilterInstance::Init(AmUInt32 numParams)
    {
        delete[] m_parameters;

        m_numParams = numParams;
        m_parameters = new AmReal32[numParams];

        if (m_parameters == nullptr)
        {
            delete[] m_parameters;

            m_parameters = nullptr;
            m_numParams = 0;

            return AM_ERROR_OUT_OF_MEMORY;
        }

        std::memset(m_parameters, 0, m_numParams * sizeof(AmReal32));

        m_parameters[0] = 1; // Set 'Wet' to 1

        return 0;
    }

    void FilterInstance::AdvanceFrame(AmTime delta_time)
    {}

    void FilterInstance::Process(AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        if (buffer == nullptr)
            return;

        for (AmUInt16 c = 0; c < channels; c++)
            ProcessChannel(buffer, c, frames, channels, sampleRate, false);
    }

    void FilterInstance::ProcessInterleaved(
        AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        if (buffer == nullptr)
            return;

        for (AmUInt16 c = 0; c < channels; c++)
            ProcessChannel(buffer, c, frames, channels, sampleRate, true);
    }

    void FilterInstance::ProcessChannel(
        AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved)
    {
        if (buffer == nullptr)
            return;

        if (isInterleaved)
        {
            for (AmUInt64 i = channel; i < frames * channels; i += channels)
                buffer[i] = ProcessSample(buffer[i], channel, sampleRate);
        }
        else
        {
            for (AmUInt64 i = channel * frames; i < frames; i++)
                buffer[i] = ProcessSample(buffer[i], channel, sampleRate);
        }
    }

    AmAudioSample FilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        return sample;
    }

    AmReal32 FilterInstance::GetFilterParameter(AmUInt32 attributeId)
    {
        if (attributeId >= m_numParams)
            return 0;

        return m_parameters[attributeId];
    }

    void FilterInstance::SetFilterParameter(AmUInt32 attributeId, AmReal32 value)
    {
        if (attributeId >= m_numParams)
            return;

        if (m_parameters[attributeId] == value)
            return;

        m_parameters[attributeId] = value;
        m_numParamsChanged |= 1 << attributeId;
    }
} // namespace SparkyStudios::Audio::Amplitude