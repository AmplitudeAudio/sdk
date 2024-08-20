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

#ifndef _AM_DSP_FILTER_H
#define _AM_DSP_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Filter;

    /**
     * @brief A Filter instance.
     *
     * An object of this class will be created each time a @c Filter is requested.
     */
    class AM_API_PUBLIC FilterInstance
    {
    public:
        /**
         * @brief Constructs a new @c FilterInstance object.
         *
         * @param parent The parent @c Filter object that created this instance.
         */
        explicit FilterInstance(Filter* parent);

        virtual ~FilterInstance();

        /**
         * @brief Initializes the filter instance with the provided number of
         * parameters.
         *
         * @param paramCount The number of parameters the filter will need.
         */
        AmResult Initialize(AmUInt32 paramCount);

        /**
         * @brief Updates the filter instance state for the provided delta time.
         *
         * @param deltaTime The time in milliseconds since the last frame.
         */
        virtual void AdvanceFrame(AmTime deltaTime);

        /**
         * @brief Executes the filter instance.
         *
         * @param in The input buffer on which the filter should be applied.
         * @param out The output buffer where the filtered output will be stored.
         * @param frames The number of frames to process.
         * @param sampleRate The current sample rate of the @c buffer.
         */
        virtual void Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate);

        /**
         * @brief Gets the current value of the parameter at the given index.
         *
         * @param parameterIndex The index of the parameter to retrieve.
         *
         * @return The current value of the parameter.
         */
        virtual AmReal32 GetParameter(AmUInt32 parameterIndex);

        /**
         * @brief Sets the value of the parameter at the given index.
         *
         * @param parameterIndex The index of the parameter to retrieve.
         * @param value The value to set to the parameter.
         */
        virtual void SetParameter(AmUInt32 parameterIndex, AmReal32 value);

    protected:
        /**
         * @brief Executes the filter instance on a single channel of the given buffer.
         *
         * @param in The input buffer on which the filter should be applied.
         * @param out The output buffer where the filtered output will be stored.
         * @param channel The index of the channel to process.
         * @param frames The number of frames to process.
         * @param sampleRate The current sample rate of the @c buffer.
         */
        virtual void ProcessChannel(const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate);

        /**
         * @brief Executes the filter instance on a single sample of the given buffer.
         *
         * @param sample The audio sample to process.
         * @param channel The index of the channel to process.
         * @param sampleRate The current sample rate of the @c buffer.
         */
        virtual AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate);

        Filter* m_parent;

        AmUInt32 m_numParams;
        AmUInt32 m_numParamsChanged;
        AmReal32Buffer m_parameters;
    };

    /**
     * @brief Helper class to manage filters.
     *
     * A filter applies transformations on an audio buffer..
     */
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
         * @brief Create a new @c Filter instance.
         *
         * @param name The filter name. eg. "Delay".
         */
        explicit Filter(AmString name);

        virtual ~Filter();

        /**
         * @brief Get the maximum number of parameters available for this filter.
         *
         * @return The maximum number of filter parameters.
         */
        [[nodiscard]] virtual AmUInt32 GetParamCount() const;

        /**
         * @brief Get the name of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The name of the parameter at the given index.
         */
        [[nodiscard]] virtual AmString GetParamName(AmUInt32 index) const;

        /**
         * @brief Get the type of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The type of the parameter at the given index.
         */
        [[nodiscard]] virtual AmUInt32 GetParamType(AmUInt32 index) const;

        /**
         * @brief Get the maximum allowed value of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The maximum allowed value of the parameter at the given index.
         */
        [[nodiscard]] virtual AmReal32 GetParamMax(AmUInt32 index) const;

        /**
         * @brief Get the minimum allowed value of the parameter at the given index.
         *
         * @param index The parameter index.
         *
         * @return The minimum allowed value of the parameter at the given index.
         */
        [[nodiscard]] virtual AmReal32 GetParamMin(AmUInt32 index) const;

        /**
         * @brief Creates a new instance of the filter.
         *
         * @return A new instance of the filter.
         */
        virtual FilterInstance* CreateInstance() = 0;

        /**
         * @brief Destroys an instance of the filter. The instance should have
         * been created with @c Filter::CreateInstance().
         *
         * @param instance The filter instance to be destroyed.
         */
        virtual void DestroyInstance(FilterInstance* instance) = 0;

        /**
         * @brief Gets the name of this filter.
         *
         * @return The name of this filter.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Registers a new filter.
         *
         * @param filter The filter to add in the registry.
         */
        static void Register(Filter* filter);

        /**
         * @brief Unregisters a filter.
         *
         * @param filter The filter to remove from the registry.
         */
        static void Unregister(const Filter* filter);

        /**
         * @brief Look up a filter by name.
         *
         * @return The filter with the given name, or @c nullptr if none.
         */
        static Filter* Find(const AmString& name);

        /**
         * @brief Creates a new instance of the the filter with the given name
         * and returns its pointer. The returned pointer should be deleted using @c Filter::Destruct().
         *
         * @param name The name of the filter.
         *
         * @return The filter with the given name, or @c nullptr if none.
         */
        static FilterInstance* Construct(const AmString& name);

        /**
         * @brief Destroys the given filter instance.
         *
         * @param name The name of the filter.
         * @param instance The filter instance to destroy.
         */
        static void Destruct(const AmString& name, FilterInstance* instance);

        /**
         * @brief Locks the filters registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new filters after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the filters registry.
         *
         * This function is mainly used for internal purposes. Its
         * called after the Engine deinitialization, to allow the
         * registration of new divers after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered filters.
         *
         * @return The registry of filters.
         */
        static const std::map<AmString, Filter*>& GetRegistry();

    protected:
        /**
         * @brief The name of this filter.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_FILTER_H
