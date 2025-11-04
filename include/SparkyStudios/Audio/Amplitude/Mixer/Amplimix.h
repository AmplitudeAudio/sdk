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
#include <SparkyStudios/Audio/Amplitude/Core/Room.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Amplimix;
    class EffectInstance;
    class Event;
    class Sound;

    /**
     * @brief Called just after the mixer processes audio data.
     *
     * @param[in] mixer The Amplimix instance.
     * @param[in] buffer The audio buffer that has been mixed.
     * @param[in] frames The number of audio frames that have been mixed.
     *
     * @ingroup mixer
     */
    AM_CALLBACK(void, AfterMixCallback)(const Amplimix* mixer, const AudioBuffer* buffer, AmUInt32 frames);

    /**
     * @brief A single layer in the mixer.
     *
     * A mixer layer is a container for audio data and associated properties. Each layer is linked
     * to a single @c SoundInstance and manages its life cycle inside @c Amplimix.
     *
     * @see Amplimix
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC AmplimixLayer
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~AmplimixLayer() = default;

        /**
         * @brief Gets the unique identifier of the layer.
         *
         * @return The unique identifier of the layer.
         */
        virtual AmUInt32 GetId() const = 0;

        /**
         * @brief Gets the start position of the audio data in the layer.
         *
         * This position is in samples, not bytes. It represents the offset from the start of the
         * linked sound data, where the mixer should start playing audio.
         *
         * @return The start position of the audio data in the layer.
         */
        virtual AmUInt64 GetStartPosition() const = 0;

        /**
         * @brief Gets the end position of the audio data in the layer.
         *
         * This position is in samples, not bytes. It represents the offset from the start of the
         * linked sound data, where the mixer should stop playing audio.
         *
         * @return The end position of the audio data in the layer.
         */
        virtual AmUInt64 GetEndPosition() const = 0;

        /**
         * @brief Gets the current position of the audio data in the layer.
         *
         * This position is in samples, not bytes. It represents the current offset from the start of the
         * linked sound data, where the mixer is currently playing audio.
         *
         * @return The current playback position of the audio data in the layer.
         */
        virtual AmUInt64 GetCurrentPosition() const = 0;

        /**
         * @brief Gets the final gain of the audio data in the layer.
         *
         * @return The final gain of the audio data in the layer.
         */
        virtual AmReal32 GetGain() const = 0;

        /**
         * @brief Gets the pitch of the audio data in the layer.
         *
         * This value is affected by the Doppler effect. A value of 1.0 will play the audio data at its original pitch,
         * while any value greater than 1.0 will increase the pitch, and any value less than 1.0 will decrease the pitch.
         *
         * @return The pitch of the audio data in the layer.
         */
        virtual AmReal32 GetPitch() const = 0;

        /**
         * @brief Gets the obstruction amount of the audio data in the layer.
         *
         * This value is useful only when the layer is associated with a sound instance that has spatialization enabled. You
         * can update this value using the @c SetObstruction method from the @c Entity this layer is associated with.
         *
         * @return The obstruction amount of the audio data in the layer.
         *
         * @see Entity::SetObstruction, Entity::GetObstruction
         */
        virtual AmReal32 GetObstruction() const = 0;

        /**
         * @brief Gets the occlusion amount of the audio data in the layer.
         *
         * This value is useful only when the layer is associated with a sound instance that has spatialization enabled. You
         * can update this value using the @c SetOcclusion method from the @c Entity this layer is associated with.
         *
         * @return The occlusion amount of the audio data in the layer.
         *
         * @see Entity::SetOcclusion, Entity::GetOcclusion
         */
        virtual AmReal32 GetOcclusion() const = 0;

        /**
         * @brief Gets the play speed of the audio data in the layer.
         *
         * This value affects the final @ref GetPitch pitch.
         *
         * @return The play speed of the audio data in the layer.
         */
        virtual AmReal32 GetPlaySpeed() const = 0;

        /**
         * @brief Gets the location of the audio data in the layer.
         *
         * The location is expressed as a 3D vector in the global space. For sound instances linked
         * to an @c Entity, the location of that entity is returned instead.
         *
         * @return The location of the audio data in the layer.
         *
         * @see Entity::SetLocation, Entity::GetLocation
         */
        virtual AmVector3 GetLocation() const = 0;

        /**
         * @brief Gets the entity associated with the audio data in the layer.
         *
         * @return The entity associated with the audio data in the layer. If the layer is not associated with
         * an @c Entity, an invalid entity is returned.
         *
         * @see Entity
         */
        virtual Entity GetEntity() const = 0;

        /**
         * @brief Gets the listener currently rendering the audio data in the layer.
         *
         * @return The listener currently rendering the audio data in the layer. If the layer is not associated with
         * a @c Listener, an invalid listener is returned.
         *
         * @see Listener
         */
        virtual Listener GetListener() const = 0;

        /**
         * @brief Gets the room in which the audio data in the layer is currently located.
         *
         * @return The room in which the audio data in the layer is currently located. If the layer is not located
         * in a @c Room, an invalid room is returned.
         *
         * @see Room
         */
        virtual Room GetRoom() const = 0;

        /**
         * @brief Gets the channel managing the audio data in the layer.
         *
         * Multiple layers can be linked to the same @c Channel.
         *
         * @return The channel managing the audio data in the layer.
         *
         * @see Channel
         */
        virtual Channel GetChannel() const = 0;

        /**
         * @brief Gets the bus on which the audio data in the layer is playing.
         *
         * @return The bus on which the audio data in the layer is playing.
         *
         * @see Bus
         */
        virtual Bus GetBus() const = 0;

        /**
         * @brief Gets the sound format of the audio data in the layer.
         *
         * The sound format specifies the number of channels, sample rate, and other audio properties. It is
         * filled by the @c Codec that handled the decoding of the audio data.
         *
         * @return The sound format of the audio data in the layer.
         *
         * @see SoundFormat, Codec
         */
        virtual SoundFormat GetSoundFormat() const = 0;

        /**
         * @brief Gets the spatialization mode of the audio data in the layer.
         *
         * The spatialization mode determines how the audio data in the layer is processed to produce a stereoscopic
         * sound. This value must be set in the asset file of the sound object.
         *
         * @return The spatialization mode of the audio data in the layer.
         *
         * @see eSpatialization
         */
        virtual eSpatialization GetSpatialization() const = 0;

        /**
         * @brief Checks if the audio data in the layer is looping.
         *
         * @return @c true if the audio data in the layer is looping, @c false otherwise.
         */
        virtual bool IsLoopEnabled() const = 0;

        /**
         * @brief Checks if the audio data in the layer is streaming from the file system.
         *
         * @return @c true if the audio data in the layer is streaming from the file system, @c false otherwise.
         */
        virtual bool IsStreamEnabled() const = 0;

        /**
         * @brief Gets the sound associated with the audio data in the layer.
         *
         * @return The sound associated with the audio data in the layer.
         *
         * @see Sound
         */
        virtual const Sound* GetSound() const = 0;

        /**
         * @brief Gets the effect associated with the audio data in the layer.
         *
         * @return The effect associated with the audio data in the layer.
         *
         * @see EffectInstance, Effect
         */
        virtual const std::shared_ptr<EffectInstance> GetEffect() const = 0;

        /**
         * @brief Gets the attenuation associated with the audio data in the layer.
         *
         * @return The attenuation associated with the audio data in the layer.
         *
         * @see Attenuation
         */
        virtual const Attenuation* GetAttenuation() const = 0;

        /**
         * @brief Gets the current sample rate of the audio data in the layer.
         *
         * The current sample rate of the audio data can be different from the original sample rate
         * stored in the sound format. Its value may change due to the Doppler effect or due to an internal
         * sample rate conversion to match the one specified in the loaded engine configuration.
         *
         * @return The current sample rate of the audio data in the layer.
         */
        virtual AmUInt32 GetSampleRate() const = 0;
    };

    /**
     * @brief Amplitude Audio Mixer.
     *
     * This class handles the processing of audio data by mixing multiple audio sources.
     * The @c Driver next handles the resulting audio stream for playback or recording.
     *
     * @see Driver
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC Amplimix
    {
    public:
        /**
         * @brief Default Destructor.
         */
        virtual ~Amplimix() = default;

        /**
         * @brief Saves the information about the rendering audio device.
         *
         * @param[in] deviceID The ID of the audio device.
         * @param[in] deviceName The name of the audio device.
         * @param[in] deviceOutputSampleRate The sample rate of the audio device's output.
         * @param[in] deviceOutputChannels The number of audio channels for the audio device's output.
         * @param[in] deviceOutputFormat The format of the audio device's output.
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
         * @brief Sets the callback function to be called after the audio data is mixed.
         *
         * @param[in] callback The callback function.
         */
        virtual void SetAfterMixCallback(AfterMixCallback callback) = 0;

        /**
         * @brief Processes the audio data by mixing multiple audio sources for the specified number of frames.
         *
         * @param[out] outBuffer The buffer to store the mixed audio data.
         * @param[in] frameCount The number of frames to mix.
         *
         * @return The number of processed frames.
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
