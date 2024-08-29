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

#ifndef _AM_MIXER_AMPLIMIX_H
#define _AM_MIXER_AMPLIMIX_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Bus.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Amplimix;
    class EffectInstance;
    class Event;
    class Sound;

    /**
     * @brief Called just before the mixer process audio data.
     */
    AM_CALLBACK(void, BeforeMixCallback)(Amplimix* mixer, AmVoidPtr buffer, AmUInt32 frames);

    /**
     * @brief Called just after the mixer process audio data.
     */
    AM_CALLBACK(void, AfterMixCallback)(Amplimix* mixer, AmVoidPtr buffer, AmUInt32 frames);

    class AM_API_PUBLIC AmplimixLayer
    {
    public:
        virtual ~AmplimixLayer() = default;

        virtual AmUInt32 GetId() const = 0;

        virtual AmUInt64 GetStartPosition() const = 0;

        virtual AmUInt64 GetEndPosition() const = 0;

        virtual AmUInt64 GetCurrentPosition() const = 0;

        virtual AmReal32 GetGain() const = 0;

        virtual AmReal32 GetStereoPan() const = 0;

        virtual AmReal32 GetPitch() const = 0;

        virtual AmReal32 GetObstruction() const = 0;

        virtual AmReal32 GetOcclusion() const = 0;

        virtual AmReal32 GetPlaySpeed() const = 0;

        virtual AmVec3 GetLocation() const = 0;

        virtual Entity GetEntity() const = 0;

        virtual Listener GetListener() const = 0;

        virtual Channel GetChannel() const = 0;

        virtual Bus GetBus() const = 0;

        virtual SoundFormat GetSoundFormat() const = 0;

        virtual eSpatialization GetSpatialization() const = 0;

        virtual bool IsLoopEnabled() const = 0;

        virtual bool IsStreamEnabled() const = 0;

        virtual const Sound* GetSound() const = 0;

        virtual const EffectInstance* GetEffect() const = 0;

        virtual const Attenuation* GetAttenuation() const = 0;

        virtual AmUInt32 GetSampleRate() const = 0;
    };

    /**
     * @brief Amplitude Audio Mixer.
     *
     * This class handles processing of audio data by mixing multiple audio sources.
     * The resulting audio stream are next handled by the @c Driver for playback or recording.
     */
    class AM_API_PUBLIC Amplimix
    {
    public:
        virtual ~Amplimix() = default;

        /**
         * @brief Saves the information about the rendering audio device.
         *
         * @param deviceID The ID of the audio device.
         * @param deviceName The name of the audio device.
         * @param deviceOutputSampleRate The sample rate of the audio device's output.
         * @param deviceOutputChannels The number of audio channels of the audio device's output.
         * @param deviceOutputFormat The format of the audio device's output.
         */
        virtual void UpdateDevice(
            AmObjectID deviceID,
            AmString deviceName,
            AmUInt32 deviceOutputSampleRate,
            PlaybackOutputChannels deviceOutputChannels,
            PlaybackOutputFormat deviceOutputFormat) = 0;

        /**
         * @brief Checks if the mixer is initialized.
         *
         * @return @c true if the mixer is initialized, @c false otherwise.
         */
        [[nodiscard]] virtual bool IsInitialized() const = 0;

        /**
         * @brief Processes the audio data by mixing multiple audio sources for the specified number of frames.
         *
         * @param outBuffer The buffer to store the mixed audio data.
         * @param frameCount The number of frames to mix.
         *
         * @return The number of frames actually mixed.
         */
        virtual AmUInt64 Mix(AudioBuffer** outBuffer, AmUInt64 frameCount) = 0;

        /**
         * @brief Gets the description of the rendering audio device.
         *
         * @return The description of the rendering audio device.
         */
        [[nodiscard]] virtual const DeviceDescription& GetDeviceDescription() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_AMPLIMIX_H
