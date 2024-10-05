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
     * An object of this class will be created each time a `Filter` is requested.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC FilterInstance
    {
    public:
        /**
         * @brief Constructs a new `FilterInstance` object.
         *
         * @param[in] parent The parent `Filter` object that created this instance.
         */
        explicit FilterInstance(Filter* parent);

        /**
         * @brief Destroys the `FilterInstance` object.
         */
        virtual ~FilterInstance();

        /**
         * @brief Initializes the filter instance with the provided number of parameters.
         *
         * @param[in] paramCount The number of parameters the filter will need.
         */
        AmResult Initialize(AmUInt32 paramCount);

        /**
         * @brief Updates the filter instance state for the provided delta time.
         *
         * @param[in] deltaTime The time in milliseconds since the last frame.
         */
        virtual void AdvanceFrame(AmTime deltaTime);

        /**
         * @brief Executes the filter instance.
         *
         * @param[in] in The input buffer on which the filter should be applied.
         * @param[out] out The output buffer where the filtered output will be stored.
         * @param[in] frames The number of frames to process.
         * @param[in] sampleRate The current sample rate of the `buffer`.
         */
        virtual void Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate);

        /**
         * @brief Gets the current value of the parameter at the given index.
         *
         * @param[in] parameterIndex The index of the parameter to retrieve.
         *
         * @return The current value of the parameter.
         */
        virtual AmReal32 GetParameter(AmUInt32 parameterIndex);

        /**
         * @brief Sets the value of the parameter at the given index.
         *
         * @param[in] parameterIndex The index of the parameter to retrieve.
         * @param[in] value The value to set to the parameter.
         */
        virtual void SetParameter(AmUInt32 parameterIndex, AmReal32 value);

    protected:
        /**
         * @brief Executes the filter instance on a single channel of the given buffer.
         *
         * @param[in] in The input buffer on which the filter should be applied.
         * @param[out] out The output buffer where the filtered output will be stored.
         * @param[in] channel The index of the channel to process.
         * @param[in] frames The number of frames to process.
         * @param[in] sampleRate The current sample rate of the `buffer`.
         */
        virtual void ProcessChannel(const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate);

        /**
         * @brief Executes the filter instance on a single sample of the given buffer.
         *
         * @param sample The audio sample to process.
         * @param channel The index of the channel to process.
         * @param sampleRate The current sample rate of the `buffer`.
         */
        virtual AmAudioSample ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate);

        /**
         * @brief The parent filter object that created this instance.
         */
        Filter* m_parent;

        /**
         * @brief The number of parameters available for this filter.
         */
        AmUInt32 m_numParams;

        /**
         * @brief The number of parameters that have changed since the last frame.
         */
        AmUInt32 m_numParamsChanged;

        /**
         * @brief The parameters buffer.
         */
        AmReal32* m_parameters;
    };

    /**
     * @brief Base class to manage filters.
     *
     * A filter applies transformations to an audio buffer. The `Filter` class implements factory methods to create
     * instances of `FilterInstance` objects, which are where the the filtering is done.
     *
     * The `Filter` class follows the [plugins architecture](/plugins/anatomy.md), and thus, you are able to create your own filters
     * by inheriting from this class, and by implementing the necessary dependencies.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC Filter
    {
        friend class FilterInstance;

    public:
        /**
         * @brief The type of a filter parameter.
         */
        enum ParameterType
        {
            kParameterTypeFloat = 0, ///< The parameter is a float.
            kParameterTypeInt, ///< The parameter is an integer.
            kParameterTypeBool ///< The parameter is a boolean.
        };

        /**
         * @brief Create a new `Filter` instance.
         *
         * @param[in] name The filter name. eg. "Echo".
         */
        explicit Filter(AmString name);

        /**
         * @brief Default destructor.
         */
        virtual ~Filter();

        /**
         * @brief Gets the maximum number of parameters available for this filter.
         *
         * @return The maximum number of filter parameters.
         */
        [[nodiscard]] virtual AmUInt32 GetParamCount() const;

        /**
         * @brief Gets the name of the parameter at the given index.
         *
         * @param[in] index The parameter index.
         *
         * @return The name of the parameter at the given index.
         */
        [[nodiscard]] virtual AmString GetParamName(AmUInt32 index) const;

        /**
         * @brief Gets the type of the parameter at the given index.
         *
         * @param[in] index The parameter index.
         *
         * @return The type of the parameter at the given index.
         */
        [[nodiscard]] virtual AmUInt32 GetParamType(AmUInt32 index) const;

        /**
         * @brief Gets the maximum allowed value of the parameter at the given index.
         *
         * @param[in] index The parameter index.
         *
         * @return The maximum allowed value of the parameter at the given index.
         */
        [[nodiscard]] virtual AmReal32 GetParamMax(AmUInt32 index) const;

        /**
         * @brief Gets the minimum allowed value of the parameter at the given index.
         *
         * @param[in] index The parameter index.
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
         * @brief Destroys an instance of the filter.
         *
         * @warning The instance should have been created with @ref CreateInstance `CreateInstance()`
         * before being destroyed with this method.
         *
         * @param[in] instance The filter instance to be destroyed.
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
         * @param[in] filter The filter to add in the registry.
         */
        static void Register(Filter* filter);

        /**
         * @brief Unregisters a filter.
         *
         * @param[in] filter The filter to remove from the registry.
         */
        static void Unregister(const Filter* filter);

        /**
         * @brief Look up a filter by name.
         *
         * @return The filter with the given name, or `nullptr` if none.
         */
        static Filter* Find(const AmString& name);

        /**
         * @brief Creates a new instance of the the filter with the given name and returns its pointer.
         *
         * @note The returned pointer should be deleted using @ref Destruct `Destruct()`.
         *
         * @param[in] name The name of the filter.
         *
         * @return The filter with the given name, or `nullptr` if none.
         */
        static FilterInstance* Construct(const AmString& name);

        /**
         * @brief Destroys the given filter instance.
         *
         * @param[in] name The name of the filter.
         * @param[in] instance The filter instance to destroy.
         */
        static void Destruct(const AmString& name, FilterInstance* instance);

        /**
         * @brief Locks the filters registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the `Engine` initialization, to discard the registration
         * of new filters after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the filters registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the `Engine` deinitialization, to allow the registration
         * of new filters after the engine is fully unloaded.
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
