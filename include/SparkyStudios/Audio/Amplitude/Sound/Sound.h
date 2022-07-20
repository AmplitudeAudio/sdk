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

#ifndef SS_AMPLITUDE_AUDIO_SOUND_H
#define SS_AMPLITUDE_AUDIO_SOUND_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>

#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SoundDefinition;

    struct EngineInternalState;
    class BusInternalState;

    class Collection;
    class SoundInstance;
    class RealChannel;

    class SoundChunk;

    enum class SoundKind
    {
        Switched,
        Contained,
        Standalone,
    };

    struct SoundInstanceSettings
    {
        AmObjectID m_id;
        SoundKind m_kind;
        AmBusID m_busID;
        AmAttenuationID m_attenuationID;
        AmEffectID m_effectID;
        AmUInt8 m_spatialization;
        RtpcValue m_priority;
        RtpcValue m_gain;
        bool m_loop;
        AmUInt32 m_loopCount;
    };

    class Sound
        : public SoundObject
        , public Resource
    {
        friend class Collection;
        friend class SoundInstance;

    public:
        Sound();
        ~Sound() override;

        /**
         * @brief Loads the sound from the given source.
         *
         * @param source The sound file content to load.
         * @param state The engine state to use while loading the sound.
         *
         * @return true if the sound was loaded successfully, false otherwise.
         */
        bool LoadDefinition(const std::string& source, EngineInternalState* state) override;

        /**
         * @brief Loads the sound from the given file path.
         *
         * @param filename The path to the sound file to load.
         * @param state The engine state to use while loading the sound.
         *
         * @return true if the sound was loaded successfully, false otherwise.
         */
        bool LoadDefinitionFromFile(const AmOsString& filename, EngineInternalState* state) override;

        /**
         * @brief Acquires referenced objects in this Sound.
         *
         * @param state The engine state used while loading the sound.
         */
        void AcquireReferences(EngineInternalState* state) override;

        /**
         * @brief Releases the references acquired when loading the sound.
         *
         * @param state The engine state used while loading the sound.
         */
        void ReleaseReferences(EngineInternalState* state) override;

        /**
         * @brief Returns the loaded sound definition.
         *
         * @return The loaded sound definition.
         */
        [[nodiscard]] const SoundDefinition* GetSoundDefinition() const;

        /**
         * @brief Loads the audio file.
         */
        void Load(const FileLoader* loader) override;

        /**
         * @brief Create a new SoundInstance from this Sound.
         *
         * The SoundInstance created this way will use settings fom the Sound object.
         *
         * @return SoundInstance* A sound instance which can be played.
         */
        [[nodiscard]] SoundInstance* CreateInstance();

        /**
         * @brief Create a new SoundInstance from this Sound.
         *
         * The SoundInstance created this way will use the settings fom the given
         * Collection. This is mainly used when a Collection play a sound fom his container.
         *
         * @param collection The Collection that the sound is part of.
         *
         * @return SoundInstance* A sound instance which can be played.
         */
        [[nodiscard]] SoundInstance* CreateInstance(const Collection* collection);

        /**
         * @brief Sets the format of this Sound.
         *
         * @param format The sound format.
         */
        void SetFormat(const SoundFormat& format);

        /**
         * @brief Gets the format of this Sound.
         *
         * @return The format of this Sound.
         */
        [[nodiscard]] const SoundFormat& GetFormat() const;

        /**
         * @brief Checks streaming is enabled for this Sound.
         *
         * @return true if streaming is enabled, false otherwise.
         */
        [[nodiscard]] bool IsStream() const;

        /**
         * @brief Checks if looping is enabled for this Sound.
         *
         * @return true if looping is enabled, false otherwise.
         */
        [[nodiscard]] bool IsLoop() const;

    protected:
        /**
         * @brief Returns the SoundChunk associated with this Sound
         * and increment its reference counter.
         *
         * If the reference equals 0, the SoundChunk is created.
         *
         * This methods is used by the SoundInstance to get the SoundChunk
         * only when the audio file is not streamed.
         *
         * @return The SoundChunk of this sound.
         */
        SoundChunk* AcquireSoundData();

        /**
         * @brief Decrements the SoundChunk's reference counter.
         *
         * If the reference counter reaches 0, the SoundChunk is deleted.
         *
         * This methods is used by the SoundInstance to get the SoundChunk
         * only when the audio file is not streamed.
         *
         * @return The SoundChunk of this sound.
         */
        void ReleaseSoundData();

        SoundFormat m_format;

    private:
        Codec::Decoder* _decoder;

        bool _stream;
        bool _loop;
        AmUInt32 _loopCount;

        std::string _source;
        SoundInstanceSettings _settings;

        SoundChunk* _soundData;
        RefCounter _soundDataRefCounter;
    };

    class SoundInstance
    {
        friend class Mixer;
        friend class Sound;

    public:
        /**
         * @brief Construct a new SoundInstance from the given Sound.
         *
         * @param parent The parent Sound from which to create an instance.
         * @param settings The settings of the Sound instance.
         * @param effect The sound effect to apply on playback.
         */
        SoundInstance(Sound* parent, const SoundInstanceSettings& settings, const Effect* effect = nullptr);
        ~SoundInstance();

        /**
         * @brief Loads the audio sample data into this SoundInstance.
         */
        void Load();

        /**
         * @brief Get the settings used to create this SoundInstance.
         *
         * @return The SoundInstance settings.
         */
        [[nodiscard]] const SoundInstanceSettings& GetSettings() const;

        /**
         * @brief Returns the user data associated to this Sound.
         * @return The user data.
         */
        [[nodiscard]] AmVoidPtr GetUserData() const;

        /**
         * @brief Sets the user data associated to this Sound.
         * @param userData The user data.
         */
        void SetUserData(AmVoidPtr userData);

        /**
         * @brief Renders audio data.
         *
         * The audio data is read from the audio file loaded by this
         * Sound. This function is mostly for internal uses.
         *
         * @param offset The offset in the audio data to start reading from.
         * @param frames The number of audio frames to read.
         *
         * @return The number of frames read.
         */
        [[nodiscard]] AmUInt64 GetAudio(AmUInt64 offset, AmUInt64 frames) const;

        /**
         * @brief Destroys the audio sample loaded by this Sound and
         * releases all resources.
         */
        void Destroy();

        /**
         * @brief Checks if this SoundInstance has a valid state.
         *
         * @return true when the state of this SoundInstance is valid.
         * @return false when the state of this SoundInstance is not valid.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Sets the RealChannel in which this SoundInstance is playing.
         *
         * @param channel The RealChannel instance in which this SoundInstance is playing.
         */
        void SetChannel(RealChannel* channel);

        /**
         * @brief Gets the RealChannel in which this SoundInstance is playing.
         *
         * @return The RealChannel instance in which this SoundInstance is playing.
         */
        [[nodiscard]] RealChannel* GetChannel() const;

        /**
         * @brief Gets the Sound source which have generated this SoundInstance.
         *
         * @return The Sound object which is the source of this instance.
         */
        [[nodiscard]] const Sound* GetSound() const;

        /**
         * @brief Gets the Collection which have generated this SoundInstance.
         *
         * @return The Collection object which have generated this instance, if any.
         */
        [[nodiscard]] const Collection* GetCollection() const;

        /**
         * @brief Gets the number of times this SoundInstance have looped.
         *
         * @return The number of times this sound instance have looped.
         */
        [[nodiscard]] AmUInt32 GetCurrentLoopCount() const;

        /**
         * @brief Gets the effect applied to this SoundInstance, if any.
         *
         * @return An EffectInstance object if an effect was applied to the sound which have
         * generated this SoundInstance, or nullptr otherwise.
         */
        [[nodiscard]] const EffectInstance* GetEffect() const;

        /**
         * @brief Set the obstruction level of sounds played by this Entity.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction);

        /**
         * @brief Set the occlusion level of sounds played by this Entity.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion);

        /**
         * @brief Get the obstruction level of sounds played by this Entity.
         *
         * @return The obstruction amount.
         */
        [[nodiscard]] AmReal32 GetObstruction() const;

        /**
         * @brief Get the occlusion level of sounds played by this Entity.
         *
         * @return The occlusion amount.
         */
        [[nodiscard]] AmReal32 GetOcclusion() const;

        /**
         * @brief Get the generated sound instance ID.
         *
         * This generated ID is ensured to be unique within all the sound instances
         * created within the engine.
         *
         * @return A generated sound instance ID.
         */
        [[nodiscard]] AmObjectID GetId() const;

    private:
        AmVoidPtr _userData;

        RealChannel* _channel;
        Sound* _parent;
        const Collection* _collection;
        EffectInstance* _effectInstance;

        SoundInstanceSettings _settings;

        AmUInt32 _currentLoopCount;

        AmReal32 _obstruction;
        AmReal32 _occlusion;

        AmObjectID _id;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUND_H
