// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef _AM_DSP_AUDIO_CONVERTER_H
#define _AM_DSP_AUDIO_CONVERTER_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Resampler.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Allow converting audio buffers between different sample rates and channel counts.
     *
     * @note This class uses the `Resampler` class to perform sample rate conversion.
     *
     * @note Only mono to stereo or vice versa conversions are currently supported.
     *
     * @ingroup dsp
     */
    class AudioConverter final
    {
    public:
        /**
         * @brief Store conversion settings for an `AudioConverter` instance.
         */
        struct Settings
        {
            /**
             * @brief The source sample rate.
             */
            AmUInt32 m_sourceSampleRate;

            /**
             * @brief The destination sample rate.
             */
            AmUInt32 m_targetSampleRate;

            /**
             * @brief The source channel count.
             */
            AmUInt16 m_sourceChannelCount;

            /**
             * @brief The destination channel count.
             */
            AmUInt16 m_targetChannelCount;
        };

        /**
         * @brief Default constructor.
         */
        AudioConverter();

        /**
         * @brief Destroys the instance and release associated resources.
         */
        ~AudioConverter();

        /**
         * @brief Initializes the audio converter with the given conversion settings.
         *
         * @param[in] settings The conversion settings.
         *
         * @return `true` if the initialization was successful, `false` otherwise.
         */
        bool Configure(const Settings& settings);

        /**
         * @brief Converts the audio buffer from the source sample rate and channel count to the target sample rate and channel count.
         *
         * @param[in] input The source audio buffer.
         * @param[in,out] inputFrames The number of frames to process in the input audio buffer.
         * @param[out] output The target audio buffer to store the converted audio.
         * @param[in,out] outputFrames The number of frames to process in the target audio buffer.
         */
        void Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames);

        /**
         * @brief Updates the source sample rate and target sample rate.
         *
         * @param[in] sourceSampleRate The source sample rate.
         * @param[in] targetSampleRate The target sample rate.
         */
        void SetSampleRate(AmUInt64 sourceSampleRate, AmUInt64 targetSampleRate);

        /**
         * @brief Returns the required number of frames to have as input for the given amount of output frames.
         *
         * @param[in] outputFrameCount The number of output frames.
         *
         * @return The input frame count needed to produce the given output frame count.
         */
        [[nodiscard]] AmUInt64 GetRequiredInputFrameCount(AmUInt64 outputFrameCount) const;

        /**
         * @brief Returns the expected number of frames to have as output for the given amount of input frames.
         *
         * @param[in] inputFrameCount The number of input frames.
         *
         * @return The expected number of output frames for the given input frame count.
         */
        [[nodiscard]] AmUInt64 GetExpectedOutputFrameCount(AmUInt64 inputFrameCount) const;

        /**
         * @brief Returns the current input latency in frames.
         *
         * @return The current input latency in frames.
         */
        [[nodiscard]] AmUInt64 GetInputLatency() const;

        /**
         * @brief Returns the current output latency in frames.
         *
         * @return The current output latency in frames.
         */
        [[nodiscard]] AmUInt64 GetOutputLatency() const;

        /**
         * @brief Resets the internal state of the converter.
         */
        void Reset();

    private:
        enum ChannelConversionMode
        {
            kChannelConversionModeDisabled,
            kChannelConversionModeStereoToMono,
            kChannelConversionModeMonoToStereo,
        };

        /**
         * @brief Converts stereo audio to mono.
         *
         * @param[in] input The input audio buffer.
         * @param[out] output The output audio buffer to store the converted audio.
         *
         * @internal
         */
        static void ConvertStereoFromMono(const AudioBuffer& input, AudioBuffer& output);

        /**
         * @brief Converts mono audio to stereo.
         *
         * @param[in] input The input audio buffer.
         * @param[out] output The output audio buffer to store the converted audio.
         *
         * @internal
         */
        static void ConvertMonoFromStereo(const AudioBuffer& input, AudioBuffer& output);

        ResamplerInstance* _resampler;
        ChannelConversionMode _channelConversionMode;

        bool _needResampling;
        bool _srcInitialized;

        Settings _settings;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_AUDIO_CONVERTER_H
