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

#ifndef SS_AMPLITUDE_AUDIO_MIXER_DEVICE_DESCRIPTION_H
#define SS_AMPLITUDE_AUDIO_MIXER_DEVICE_DESCRIPTION_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The playback output format of the device.
     */
    enum class PlaybackOutputFormat : AmUInt16
    {
        /**
         * @brief The default output format of the device.
         *
         * This instruct to use the output format provided by the device.
         */
        Default = 0,

        /**
         * @brief Send audio samples as unsigned 8-bit integers
         * to the device.
         */
        UInt8 = 1,

        /**
         * @brief Send audio samples as signed 16-bit integers
         * to the device.
         */
        Int16 = 2,

        /**
         * @brief Send audio samples as signed 24-bit integers
         * to the device.
         */
        Int24 = 3,

        /**
         * @brief Send audio samples as signed 32-bit integers
         * to the device.
         */
        Int32 = 4,

        /**
         * @brief Send audio samples as 32-bit floating point values
         * to the device.
         */
        Float32 = 5
    };

    /**
     * @brief The playback output channel layout of the device.
     */
    enum class PlaybackOutputChannels : AmUInt16
    {
        /**
         * @brief The default output channel layout of the device.
         *
         * This instruct to use the output channel layout provided by the device.
         */
        Default = 0,

        /**
         * @brief Send audio samples as mono.
         */
        Mono = 1,

        /**
         * @brief Send audio samples as interleaved stereo.
         */
        Stereo = 2,

        /**
         * @brief Send audio samples as quadraphonic.
         */
        Quad = 4,

        /**
         * @brief Send audio samples as 5.1 surround sound.
         */
        Surround_5_1 = 6,

        /**
         * @brief Send audio samples as 7.1 surround sound.
         */
        Surround_7_1 = 8
    };

    /**
     * @brief The device description.
     *
     * This stores the settings requested from the engine configuration
     * and the actual settings provided by the device.
     *
     * The device settings are filled after the Amplimix initialization,
     * and are provided by the selected Driver.
     */
    struct DeviceDescription
    {
        /**
         * @brief The device name.
         */
        std::string mDeviceName;

        /**
         * @brief The device ID.
         */
        AmObjectID mDeviceID;

        /**
         * @brief The requested device output format.
         */
        PlaybackOutputFormat mRequestedOutputFormat;

        /**
         * @brief The actual device format.
         */
        PlaybackOutputFormat mDeviceOutputFormat;

        /**
         * @brief The requested device output channel layout.
         */
        PlaybackOutputChannels mRequestedOutputChannels;

        /**
         * @brief The actual device channel layout.
         */
        PlaybackOutputChannels mDeviceOutputChannels;

        /**
         * @brief The requested device sample rate.
         */
        AmUInt32 mRequestedOutputSampleRate;

        /**
         * @brief The actual device sample rate.
         */
        AmUInt32 mDeviceOutputSampleRate;

        /**
         * @brief The device output buffer size.
         */
        AmUInt32 mOutputBufferSize;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_MIXER_DEVICE_DESCRIPTION_H
