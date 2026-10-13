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
         * @note Runs on the game thread and may allocate.
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
         * @brief Resamples audio data.
         *
         * Contract, required from every implementation:
         * - The call consumes only the input it needs for the output it produces, and reports both counts.
         * - The output never depends on input that has not been received yet.
         * - The only internal state is filter history and phase: the output sequence does not depend on how the input
         *   and the output are split across calls.
         * - Runs on the audio thread: it never allocates, locks or logs.
         *
         * @param[in] input The input buffer.
         * @param[in,out] inputFrames The number of available input frames; on return, the number of frames consumed.
         * @param[out] output The output buffer.
         * @param[in,out] outputFrames The number of output frames wanted; on return, the number of frames produced.
         *
         * @return @c true on success, @c false otherwise.
         */
        virtual bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) = 0;

        /**
         * @brief Sets the conversion ratio, pitch and playback speed included.
         *
         * Audio-thread safe and allocation-free. Changing the ratio keeps the phase: the output stays continuous.
         * Implementations that cannot represent the ratio exactly approximate it.
         *
         * Non-finite or non-positive values are treated as 1.
         *
         * @param[in] inputPerOutput The number of input frames consumed per output frame.
         */
        virtual void SetRatio(AmReal64 inputPerOutput) = 0;

        /**
         * @brief Sets the conversion ratio to follow a ramp across the output frames of the next call.
         *
         * A mixer whose speed moves during a block -- pitch, Doppler, playback rate -- publishes a ramp rather than a
         * step. Holding one constant ratio for the whole block makes the read position a staircase, and a staircase
         * whose steps move is audible as a click at every block boundary. An implementation that varies the ratio per
         * output frame makes the read position continuous, and the click with it.
         *
         * The ramp covers @p outputFrames output frames from the first frame @c Process() produces after this call; a
         * call producing more frames than that keeps the end value. Several calls may share one ramp, as a block split
         * into segments makes several: each consumes the part of the ramp that follows the frames already produced.
         * @p outputFrames of 0, or a start equal to the end, is a constant ratio.
         *
         * The sum of the input consumed over the ramp is that of the mean ratio, whichever way an implementation gets
         * there, so a caller can size buffers and map positions from the mean.
         *
         * @warning This default implementation collapses the ramp to its mean: the stream advances correctly, but the
         * read position is the staircase described above. Override it to vary the ratio per output frame.
         *
         * Audio-thread safe and allocation-free, like @c SetRatio().
         *
         * @param[in] inputPerOutputStart The input frames consumed per output frame, at the first output frame.
         * @param[in] inputPerOutputEnd The input frames consumed per output frame, at the last output frame of the ramp.
         * @param[in] outputFrames The number of output frames the ramp spans.
         */
        virtual void SetRatioRamp(AmReal64 inputPerOutputStart, AmReal64 inputPerOutputEnd, AmUInt64 outputFrames)
        {
            AM_UNUSED(outputFrames);
            SetRatio(0.5 * (inputPerOutputStart + inputPerOutputEnd));
        }

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
         * @brief Returns the exact number of input frames the next @c Process() call needs to produce the given output.
         *
         * Counts from the first frame of the next input, taking the current phase into account.
         *
         * @param[in] outputFrameCount The number of output frames wanted.
         *
         * @return The input frame count needed; 0 when @p outputFrameCount is 0.
         */
        [[nodiscard]] virtual AmUInt64 GetInputFramesNeeded(AmUInt64 outputFrameCount) const = 0;

        /**
         * @brief Returns the output delay relative to the input, in input frames. The built-in resamplers centre their
         * kernel on the output time and report 0; their read-ahead is part of @c GetInputFramesNeeded().
         *
         * A stream that ends keeps feeding zeros until @c GetInputFramesNeeded(1) frames past its end, plus twice this
         * delay, are consumed.
         *
         * @return The output delay in input frames.
         */
        [[nodiscard]] virtual AmUInt64 GetLatency() const = 0;

        /**
         * @brief Resets the internal resampler state (history and phase). Audio-thread safe.
         */
        virtual void Reset() = 0;

        /**
         * @brief Fills the history with the input that precedes the next one, as if it had just been consumed.
         *
         * A stream that restarts mid-signal -- a seek, or a start at an offset -- calls this after @c Reset() and before
         * the next @c Process(), with the frames just before its new position. The first outputs then read real signal on
         * their left instead of zeros, and start without the ringing of an abrupt onset. The phase is left untouched.
         *
         * @warning This default implementation ignores the frames: the stream restarts on a silent history.
         *
         * Audio-thread safe and allocation-free.
         *
         * @param[in] input The frames before the next input, on every channel.
         * @param[in] frames The number of frames of @p input to take, from its start. An implementation keeps the last
         * ones it needs.
         */
        virtual void PrimeHistory(const AudioBuffer& input, AmUInt64 frames)
        {
            AM_UNUSED(input);
            AM_UNUSED(frames);
        }

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
