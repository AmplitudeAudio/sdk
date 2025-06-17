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
     * This class is the place where the DSP filtering is performed. It stores the actual values
     * of the Filter parameters and uses them to apply filtering on an AudioBuffer.
     *
     * An instance of this class will be created each time its parent Filter will be requested.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC FilterInstance
    {
    public:
        /**
         * @brief Constructs a new @c FilterInstance object.
         *
         * @param[in] parent The parent @c Filter object that created this instance.
         */
        explicit FilterInstance(Filter* parent);

        /**
         * @brief Destroys the @c FilterInstance object.
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
         * @param[in] sampleRate The current sample rate of the input buffer.
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
         * @param[in] sampleRate The current sample rate of the input buffer.
         */
        virtual void ProcessChannel(const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate);

        /**
         * @brief Executes the filter instance on a single sample of the given buffer.
         *
         * @param[in] sample The audio sample to process.
         * @param[in] channel The index of the channel to process.
         * @param[in] sampleRate The current sample rate of the input buffer.
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
     * @brief Base class used to create DSP filters.
     *
     * A filter applies transformations to an audio buffer. The @c Filter class implements
     * factory methods to create instances of @c FilterInstance objects, which are where the filtering is done.
     *
     * The @c Filter class follows the [plugin architecture](/plugins/anatomy), and thus, you are able to create
     * your own filters and register them to the Engine by inheriting from this class and by implementing the necessary dependencies.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC Filter
    {
        friend class FilterInstance;

    public:
        /**
         * @brief Lists the available parameter types for a filter.
         */
        enum ParameterType
        {
            /**
             * @brief The parameter stores a @c float value.
             */
            kParameterTypeFloat = 0,

            /**
             * @brief The parameter stores an @c integer value.
             */
            kParameterTypeInt,

            /**
             * @brief The parameter stores a @c boolean value.
             */
            kParameterTypeBool
        };

        /**
         * @brief Creates a new filter instance.
         *
         * @param[in] name The filter name, e.g. "Echo".
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
        virtual std::shared_ptr<FilterInstance> CreateInstance() = 0;

        /**
         * @brief Gets the name of this filter.
         *
         * @return The name of this filter.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Registers a new filter.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] filter The filter to add in the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Register(std::shared_ptr<Filter> filter);

        /**
         * @brief Unregisters a filter.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] filter The filter to remove from the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Unregister(std::shared_ptr<const Filter> filter);

        /**
         * @brief Look up a filter by name.
         *
         * @return The filter with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<Filter> Find(const AmString& name);

        /**
         * @brief Creates a new instance of the filter with the given name and returns its pointer.
         *
         * @param[in] name The name of the filter.
         *
         * @return The filter with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<FilterInstance> Construct(const AmString& name);

        /**
         * @brief Locks the filters' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the @c Engine initialization, to discard the registration
         * of new filters after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the filters' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the @c Engine deinitialization, to allow the registration
         * of new filters after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered filters.
         *
         * @return The registry of filters.
         */
        static const std::map<AmString, std::shared_ptr<Filter>>& GetRegistry();

    protected:
        /**
         * @brief The name of this filter.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_FILTER_H
