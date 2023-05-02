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

#ifndef SS_AMPLITUDE_AUDIO_RESAMPLER_H
#define SS_AMPLITUDE_AUDIO_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ResamplerInstance
    {
    public:
        ResamplerInstance() = default;
        virtual ~ResamplerInstance() = default;

        /**
         * @brief Initialize a new instance of the resampler.
         */
        virtual void Init(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut, AmUInt64 frameCount) = 0;

        /**
         * @brief Processes the audio data.
         * @param input The input audio data.
         * @param inputFrames The number of frames in the input buffer.
         * @param output The output audio data.
         * @param outputFrames The number of frames in the output buffer.
         * @return
         */
        virtual bool Process(AmAudioSampleBuffer input, AmUInt64& inputFrames, AmAudioSampleBuffer output, AmUInt64& outputFrames) = 0;

        /**
         * @brief Changes the input and output sample rate.
         * @param sampleRateIn The new input sample rate.
         * @param sampleRateOut The new output sample rate.
         */
        virtual void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) = 0;

        /**
         * @brief Get the current input sample rate.
         * @return The current input sample rate.
         */
        [[nodiscard]] virtual AmUInt32 GetSampleRateIn() const = 0;

        /**
         * @brief Get the current output sample rate.
         * @return The current output sample rate.
         */
        [[nodiscard]] virtual AmUInt32 GetSampleRateOut() const = 0;

        /**
         * @brief Get the current channels count.
         * @return The current channels count.
         */
        [[nodiscard]] virtual AmUInt16 GetChannelCount() const = 0;

        /**
         * @brief Returns the required number of frames to have as input for the
         * given amount of output frames.
         * @param outputFrameCount The number of output frames.
         * @return The input frame count needed to produce the given output frame count.
         */
        [[nodiscard]] virtual AmUInt64 GetRequiredInputFrameCount(AmUInt64 outputFrameCount) const = 0;

        /**
         * @brief Returns the expected number of frames to have as output for the
         * given amount of input frames.
         * @param inputFrameCount The number of input frames.
         * @return The expected number of output frames for the given input frame count.
         */
        [[nodiscard]] virtual AmUInt64 GetExpectedOutputFrameCount(AmUInt64 inputFrameCount) const = 0;

        [[nodiscard]] virtual AmUInt64 GetLatencyInFrames() const = 0;

        /**
         * @biref Resets the internal resampler state.
         */
        virtual void Reset() = 0;

        /**
         * @bbrief Cleans up the internal resampler state and allocated data.
         * @note This method is called when the resampler is about to be destroyed.
         */
        virtual void Clear() = 0;
    };

    class Resampler
    {
    public:
        /**
         * @brief Create a new Resampler instance.
         *
         * @param name The resampler name. eg. "MiniAudioLinear".
         */
        explicit Resampler(std::string name);

        /**
         * @brief Default Resampler constructor.
         *
         * This will not automatically register the resampler. It's meant for internal resamplers only.
         */
        Resampler();

        virtual ~Resampler() = default;

        /**
         * @brief Creates a new instance of the resampler.
         * @return A new instance of the resampler.
         */
        virtual ResamplerInstance* CreateInstance() = 0;

        /**
         * @brief Destroys an instance of the resampler. The instance should have
         * been created with CreateInstance().
         * @param instance The resampler instance to be destroyed.
         */
        virtual void DestroyInstance(ResamplerInstance* instance) = 0;

        /**
         * @brief Gets the name of this resampler.
         *
         * @return The name of this resampler.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Registers a new resampler.
         *
         * @param resampler The resampler to add in the registry.
         */
        static void Register(Resampler* resampler);

        /**
         * @brief Creates a new instance of the the resampler with the given name
         * and returns its pointer. The returned pointer should be deleted using Resampler::Destruct().
         *
         * @param name The name of the resampler.
         *
         * @return The resampler with the given name, or NULL if none.
         */
        static ResamplerInstance* Construct(const std::string& name);

        /**
         * @brief Destroys the given resampler instance.
         *
         * @param name The name of the resampler.
         * @param instance The resampler instance to destroy.
         */
        static void Destruct(const std::string& name, ResamplerInstance* instance);

        /**
         * @brief Locks the resamplers registry.
         *
         * This function is mainly used for internal purposes. Its
         * called before the Engine initialization, to discard the
         * registration of new resamplers after the engine is fully loaded.
         */
        static void LockRegistry();

    protected:
        /**
         * @brief The name of this resampler.
         */
        std::string m_name;

    private:
        /**
         * @brief Look up a resampler by name.
         *
         * @return The resampler with the given name, or NULL if none.
         */
        static Resampler* Find(const std::string& name);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_RESAMPLER_H
