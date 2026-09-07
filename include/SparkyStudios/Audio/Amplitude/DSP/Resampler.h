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

#ifndef _AM_DSP_RESAMPLER_H
#define _AM_DSP_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Resampler;

    /**
     * @brief A Resampler instance.
     *
     * This class is the place where the resampling is performed. It exposes methods
     * to initialize, configure, and process the resampling on an AudioBuffer.
     *
     * An instance of this class will be created each time its parent Resampler will be requested.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC ResamplerInstance
    {
    public:
        /**
         * @brief Constructs a new @c ResamplerInstance object.
         *
         * This will initialize the resampler instance state to default values.
         */
        ResamplerInstance() = default;

        /**
         * @brief Default destructor.
         */
        virtual ~ResamplerInstance() = default;

        /**
         * @brief Initializes the resampler instance.
         *
         * @note Implementations must accept any pair of positive sample rates and must never read or write
         * outside their buffers for any input. When the requested ratio cannot be represented exactly, the
         * implementation must approximate it as closely as it can and keep converting.
         *
         * @param[in] channelCount The number of channels in the audio data.
         * @param[in] sampleRateIn The input sample rate.
         * @param[in] sampleRateOut The output sample rate.
         */
        virtual void Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) = 0;

        /**
         * @brief Processes the audio data.
         *
         * @param[in] input The input audio data.
         * @param[in,out] inputFrames The number of frames in the input buffer.
         * @param[out] output The output audio data.
         * @param[in,out] outputFrames The number of frames in the output buffer.
         *
         * @return @c true if the resampling was successful, @c false otherwise.
         */
        virtual bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) = 0;

        /**
         * @brief Changes the input and output sample rate.
         *
         * @note This method may be called from the audio thread on every mix block, as the mixer maps
         * playback speed changes to a rate ratio. Implementations must not allocate, lock, log, or block
         * here, and must follow the same total-function contract as @c Initialize.
         *
         * @param[in] sampleRateIn The new input sample rate.
         * @param[in] sampleRateOut The new output sample rate.
         */
        virtual void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) = 0;

        /**
         * @brief Checks whether the given conversion is performed exactly, with no approximation of the ratio.
         *
         * Every conversion is supported: an implementation that cannot represent a ratio exactly approximates
         * it. This query reports which of the two happens, so tools and asset pipelines can warn about rates
         * that will be approximated.
         *
         * @param[in] sampleRateIn The input sample rate.
         * @param[in] sampleRateOut The output sample rate.
         *
         * @return @c true if the conversion is exact, @c false if the ratio is approximated.
         */
        [[nodiscard]] virtual bool IsConversionExact(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) const
        {
            return true;
        }

        /**
         * @brief Gets the current input sample rate.
         *
         * @note This is the rate that was requested, not the internal approximation.
         *
         * @return The current input sample rate.
         */
        [[nodiscard]] virtual AmUInt32 GetSampleRateIn() const = 0;

        /**
         * @brief Gets the current output sample rate.
         *
         * @note This is the rate that was requested, not the internal approximation.
         *
         * @return The current output sample rate.
         */
        [[nodiscard]] virtual AmUInt32 GetSampleRateOut() const = 0;

        /**
         * @brief Gets the current channels count.
         *
         * @return The current channels count.
         */
        [[nodiscard]] virtual AmUInt16 GetChannelCount() const = 0;

        /**
         * @brief Returns the required number of frames to have as input for the given number of output frames.
         *
         * @param[in] outputFrameCount The number of output frames.
         *
         * @return The input frame count needed to produce the given output frame count.
         */
        [[nodiscard]] virtual AmUInt64 GetRequiredInputFrames(AmUInt64 outputFrameCount) const = 0;

        /**
         * @brief Returns the expected number of frames to have as output for the given number of input frames.
         *
         * @param[in] inputFrameCount The number of input frames.
         *
         * @return The expected number of output frames for the given input frame count.
         */
        [[nodiscard]] virtual AmUInt64 GetExpectedOutputFrames(AmUInt64 inputFrameCount) const = 0;

        /**
         * @brief Returns the current input latency in frames.
         *
         * @return The resampler's current input latency in frames.
         */
        [[nodiscard]] virtual AmUInt64 GetInputLatency() const = 0;

        /**
         * @brief Returns the current output latency in frames.
         *
         * @return The resampler's current output latency in frames.
         */
        [[nodiscard]] virtual AmUInt64 GetOutputLatency() const = 0;

        /**
         * @brief Resets the internal resampler state.
         */
        virtual void Reset() = 0;

        /**
         * @brief Cleans up the internal resampler state and allocated data.
         *
         * @note This method is called when the resampler is about to be destroyed.
         */
        virtual void Clear() = 0;
    };

    /**
     * @brief Base class used to create resamplers.
     *
     * A resampler is used to change the sample rate of an audio buffer. The @c Resampler class implements
     * factory methods to create instances of @c ResamplerInstance objects, which are where the resampling is done.
     *
     * The @c Resampler class follows the [plugin architecture](/plugins/anatomy), and thus, you are able to create
     * your own resamplers and register them to the Engine by inheriting from this class and by implementing the necessary dependencies.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC Resampler
    {
    public:
        /**
         * @brief Create a new resampler instance.
         *
         * @param[in] name The resampler name, e.g., "Libsamplerate".
         */
        explicit Resampler(AmString name);

        /**
         * @brief Default resampler constructor.
         *
         * @warning This constructor is meant for internal resamplers only.
         */
        Resampler();

        /**
         * @brief Default destructor.
         */
        virtual ~Resampler();

        /**
         * @brief Creates a new instance of the resampler.
         *
         * @return A new instance of the resampler.
         */
        virtual std::shared_ptr<ResamplerInstance> CreateInstance() = 0;

        /**
         * @brief Gets the name of this resampler.
         *
         * @return The name of this resampler.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Registers a new resampler.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] resampler The resampler to add in the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Register(std::shared_ptr<Resampler> resampler);

        /**
         * @brief Unregisters a resampler.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] resampler The resampler to remove from the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Unregister(std::shared_ptr<const Resampler> resampler);

        /**
         * @brief Look up a resampler by name.
         *
         * @param[in] name The name of the resampler to find.
         *
         * @return The resampler with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<Resampler> Find(const AmString& name);

        /**
         * @brief Creates a new instance of the resampler with the given name and returns its pointer.
         *
         * @param[in] name The name of the resampler.
         *
         * @return The resampler with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<ResamplerInstance> Construct(const AmString& name);

        /**
         * @brief Locks the resamplers registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the @c Engine initialization, to discard the registration
         * of new resamplers after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the resamplers registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the @c Engine deinitialization, to allow the registration
         * of new resamplers after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered resamplers.
         *
         * @return The registry of resamplers.
         */
        static const std::map<AmString, std::shared_ptr<Resampler>>& GetRegistry();

    protected:
        /**
         * @brief The name of this resampler.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_RESAMPLER_H
