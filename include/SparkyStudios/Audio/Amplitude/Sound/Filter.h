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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_FILTER_H
#define SS_AMPLITUDE_AUDIO_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class FilterInstance;

    class AM_API_PUBLIC Filter
    {
        friend class FilterInstance;

    public:
        enum PARAM_TYPE
        {
            PARAM_FLOAT = 0,
            PARAM_INT,
            PARAM_BOOL
        };

        /**
         * @brief Create a new Filter instance.
         *
         * @param name The filter name. eg. "MiniAudioLinear".
         */
        explicit Filter(std::string name);

        virtual ~Filter() = default;

        /**
         * @brief Get the maximum number of parameters available for this filter.
         *
         * @return The maximum number of filter parameters.
         */
        virtual AmUInt32 GetParamCount();

        /**
         * @brief Get the name of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The name of the parameter at the given index.
         */
        virtual AmString GetParamName(AmUInt32 index);

        /**
         * @brief Get the type of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The type of the parameter at the given index.
         */
        virtual AmUInt32 GetParamType(AmUInt32 index);

        /**
         * @brief Get the maximum allowed value of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The maximum allowed value of the parameter at the given index.
         */
        virtual AmReal32 GetParamMax(AmUInt32 index);

        /**
         * @brief Get the minimum allowed value of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The minimum allowed value of the parameter at the given index.
         */
        virtual AmReal32 GetParamMin(AmUInt32 index);

        /**
         * @brief Creates a new instance of the filter.
         *
         * @return A new instance of the filter.
         */
        virtual FilterInstance* CreateInstance() = 0;

        /**
         * @brief Destroys an instance of the filter. The instance should have
         * been created with CreateInstance().
         *
         * @param instance The filter instance to be destroyed.
         */
        virtual void DestroyInstance(FilterInstance* instance) = 0;

        /**
         * @brief Gets the name of this filter.
         *
         * @return The name of this filter.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Registers a new filter.
         *
         * @param filter The filter to add in the registry.
         */
        static void Register(Filter* filter);

        /**
         * @brief Look up a filter by name.
         *
         * @return The filter with the given name, or NULL if none.
         */
        static Filter* Find(const std::string& name);

        /**
         * @brief Creates a new instance of the the filter with the given name
         * and returns its pointer. The returned pointer should be deleted using Filter::Destruct().
         *
         * @param name The name of the filter.
         *
         * @return The filter with the given name, or NULL if none.
         */
        static FilterInstance* Construct(const std::string& name);

        /**
         * @brief Destroys the given filter instance.
         *
         * @param name The name of the filter.
         * @param instance The filter instance to destroy.
         */
        static void Destruct(const std::string& name, FilterInstance* instance);

        /**
         * @brief Locks the filters registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new filters after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Gets the list of registered Faders.
         *
         * @return The registry of Faders.
         */
        static const std::map<std::string, Filter*>& GetRegistry();

    protected:
        /**
         * @brief The name of this filter.
         */
        std::string m_name;
    };

    class AM_API_PUBLIC FilterInstance
    {
    public:
        explicit FilterInstance(Filter* parent);
        virtual ~FilterInstance();

        AmResult Init(AmUInt32 numParams);

        virtual void AdvanceFrame(AmTime delta_time);

        virtual void Process(AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate);

        virtual void ProcessChannel(AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate);

        virtual AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate);

        virtual AmReal32 GetFilterParameter(AmUInt32 attributeId);

        virtual void SetFilterParameter(AmUInt32 attributeId, AmReal32 value);

    protected:
        Filter* m_parent;

        AmUInt32 m_numParams;
        AmUInt32 m_numParamsChanged;
        AmReal32Buffer m_parameters;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FILTER_H
