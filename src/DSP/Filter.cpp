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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<std::string, std::shared_ptr<Filter>> FilterRegistry;
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
    {}

    Filter::~Filter()
    {}

    AmUInt32 Filter::GetParameterCount() const
    {
        return 1;
    }

    AmString Filter::GetParameterName(AmUInt32 index) const
    {
        return "Wet";
    }

    eParameterType Filter::GetParameterType(AmUInt32 index) const
    {
        return eParameterType_Float;
    }

    AmReal32 Filter::GetParameterMax(AmUInt32 index) const
    {
        return 1.0f;
    }

    AmReal32 Filter::GetParameterMin(AmUInt32 index) const
    {
        return 0.0f;
    }

    const std::string& Filter::GetName() const
    {
        return m_name;
    }

    void Filter::Register(std::shared_ptr<Filter> filter)
    {
        if (lockFilters() || filter == nullptr)
            return;

        if (Find(filter->GetName()) != nullptr)
            return;

        FilterRegistry& filters = filterRegistry();
        filters.insert(FilterImpl(filter->GetName(), filter));
        filtersCount()++;
    }

    void Filter::Unregister(std::shared_ptr<const Filter> filter)
    {
        if (lockFilters() || filter == nullptr)
            return;

        FilterRegistry& filters = filterRegistry();
        if (const auto& it = filters.find(filter->GetName()); it != filters.end())
        {
            filters.erase(it);
            filtersCount()--;
        }
    }

    std::shared_ptr<Filter> Filter::Find(const std::string& name)
    {
        for (const FilterRegistry& filters = filterRegistry(); auto&& filter : filters)
            if (filter.second->m_name == name)
                return filter.second;

        return nullptr;
    }

    std::shared_ptr<FilterInstance> Filter::Construct(const std::string& name)
    {
        std::shared_ptr<Filter> filter = Find(name);
        if (filter == nullptr)
            return nullptr;

        return filter->CreateInstance();
    }

    void Filter::LockRegistry()
    {
        lockFilters() = true;
    }

    void Filter::UnlockRegistry()
    {
        lockFilters() = false;
    }

    const std::map<std::string, std::shared_ptr<Filter>>& Filter::GetRegistry()
    {
        return filterRegistry();
    }

    FilterInstance::FilterInstance(Filter* parent)
        : m_parent(parent)
        , m_numParamsChanged(0)
        , m_parameters()
    {}

    FilterInstance::~FilterInstance()
    {
        m_parameters.clear();
    }

    AmResult FilterInstance::Initialize(AmUInt32 numParams)
    {
        m_parameters.clear();

        m_parameters.assign(numParams, 0);

        m_parameters[0] = 1; // Set 'Wet' to 1

        return 0;
    }

    void FilterInstance::AdvanceFrame(AmTime delta_time)
    {}

    void FilterInstance::Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate)
    {
        AMPLITUDE_ASSERT(out.GetChannelCount() >= in.GetChannelCount());

        for (AmUInt16 c = 0, l = in.GetChannelCount(); c < l; c++)
            ProcessChannel(in, out, c, frames, sampleRate);
    }

    void FilterInstance::ProcessChannel(const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate)
    {
        const auto& inChannel = in[channel];
        auto& outChannel = out[channel];

        for (AmUInt64 s = 0, l = in.GetFrameCount(); s < l; s++)
            outChannel[s] = ProcessSample(inChannel[s], channel, sampleRate);
    }

    AmAudioSample FilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        return sample;
    }

    AmReal32 FilterInstance::GetParameter(AmUInt32 attributeId)
    {
        if (attributeId >= m_parameters.size())
            return 0;

        return m_parameters[attributeId];
    }

    void FilterInstance::SetParameter(AmUInt32 attributeId, AmReal32 value)
    {
        if (attributeId >= m_parameters.size())
            return;

        if (m_parameters[attributeId] == value)
            return;

        m_parameters[attributeId] = value;
        m_numParamsChanged |= 1 << attributeId;
    }
} // namespace SparkyStudios::Audio::Amplitude
