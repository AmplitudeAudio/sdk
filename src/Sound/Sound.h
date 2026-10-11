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

#ifndef _AM_IMPLEMENTATION_SOUND_SOUND_H
#define _AM_IMPLEMENTATION_SOUND_SOUND_H

#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include <Core/Asset.h>
#include <IO/Resource.h>
#include <Sound/SoundObject.h>
#include <Sound/TagSet.h>

#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class SoundInstance;
    class CollectionImpl;
    class EffectInstance;
    class RealChannel;
    struct SoundChunk;

    /**
     * @brief Describes the place where a Sound belongs to.
     */
    enum class SoundKind
    {
        /**
         * @brief The sound is associated to a @a SwitchContainer.
         */
        Switched,

        /**
         * @brief The sound is associated to a @a Collection.
         */
        Contained,

        /**
         * @brief The sound is not associated to any container, and can be played directly.
         */
        Standalone,
    };

    /**
     * @brief Stores all the information required to play a sound instance.
     */
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
        RtpcValue m_nearFieldGain;
        RtpcValue m_pitch;
        bool m_loop;
        AmUInt32 m_loopCount;
        const Attenuation* m_attenuation = nullptr;
        TagSet m_tags; ///< Effective tags of the sound instance. Null means none.
    };

    class SoundImpl final
        : public Sound
        , public SoundObjectImpl<AmSoundID, SoundDefinition>
        , public ResourceImpl
    {
        friend class CollectionImpl;
        friend class SoundInstance;

    public:
        /**
         * @brief Destroys a SoundInstance.
         *
         * @param soundInstance The sound instance to destroy.
         */
        static void DestroyInstance(SoundInstance* soundInstance);

        SoundImpl();

        ~SoundImpl() override;

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
        [[nodiscard]] SoundInstance* CreateInstance(const CollectionImpl* collection);

        /**
         * @brief Create a new SoundInstance from this Sound, with the effective tags replaced.
         *
         * The SoundInstance created this way uses the settings of the Sound object, except for its tags.
         *
         * @param tags The effective tags of the instance.
         *
         * @return SoundInstance* A sound instance which can be played.
         */
        [[nodiscard]] SoundInstance* CreateInstance(const TagSet& tags);

        /**
         * @brief Returns the SoundChunk associated with this Sound
         * and increment its reference counter.
         *
         * If the reference equals 0, the SoundChunk is created.
         *
         * This method is used by the SoundInstance to get the SoundChunk
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
         * This method is used by the SoundInstance to get the SoundChunk
         * only when the audio file is not streamed.
         *
         * @return The SoundChunk of this sound.
         */
        void ReleaseSoundData();

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AmSoundID GetId() const override;

        /**
         * @copydoc Asset::GetName
         */
        [[nodiscard]] const AmString& GetName() const override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const SoundDefinition* definition, std::shared_ptr<EngineInternalState> state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const SoundDefinition* GetDefinition() const override;

        /**
         * @copydoc AssetImpl::AcquireReferences
         */
        void AcquireReferences(std::shared_ptr<EngineInternalState> state) override;

        /**
         * @copydoc AssetImpl::ReleaseReferences
         */
        void ReleaseReferences(std::shared_ptr<EngineInternalState> state) override;

        /**
         * @copydoc Resource::GetPath
         */
        [[nodiscard]] const AmOsString& GetPath() const override;

        /**
         * @copydoc Resource::Load
         */
        void Load(std::shared_ptr<const FileSystem> loader) override;

        /**
         * @copydoc SoundObject::GetGain
         */
        [[nodiscard]] const RtpcValue& GetGain() const override;

        /**
         * @brief Gets the near field effect gain.
         */
        [[nodiscard]] const RtpcValue& GetNearFieldGain() const override;

        /**
         * @copydoc Sound::GetSpread
         */
        [[nodiscard]] const RtpcValue& GetSpread() const override;

        /**
         * @copydoc SoundObject::GetPitch
         */
        [[nodiscard]] const RtpcValue& GetPitch() const override;

        /**
         * @copydoc SoundObject::GetPriority
         */
        [[nodiscard]] const RtpcValue& GetPriority() const override;

        /**
         * @copydoc SoundObject::GetEffect
         */
        [[nodiscard]] const Effect* GetEffect() const override;

        /**
         * @copydoc SoundObject::GetAttenuation
         */
        [[nodiscard]] const Attenuation* GetAttenuation() const override;

        /**
         * @copydoc SoundObject::GetBus
         */
        [[nodiscard]] Bus GetBus() const override;

        /**
         * @copydoc SoundObject::GetSpatialization
         */
        [[nodiscard]] AM_INLINE eSpatialization GetSpatialization() const override
        {
            return SoundObjectImpl::GetSpatialization();
        }

        /**
         * @copydoc SoundObject::GetScope
         */
        [[nodiscard]] AM_INLINE eScope GetScope() const override
        {
            return SoundObjectImpl::GetScope();
        }

        /**
         * @copydoc SoundObject::GetTags
         */
        [[nodiscard]] AM_INLINE std::span<const AmTagID> GetTags() const override
        {
            return SoundObjectImpl::GetTags();
        }

        /**
         * @copydoc SoundObject::HasTag
         */
        [[nodiscard]] AM_INLINE bool HasTag(AmTagID tag) const override
        {
            return SoundObjectImpl::HasTag(tag);
        }

        /**
         * @copydoc Sound::IsStream
         */
        [[nodiscard]] bool IsStream() const override;

        /**
         * @copydoc Sound::IsLoop
         */
        [[nodiscard]] bool IsLoop() const override;

        /**
         * @brief Gets the number of loops this sound plays.
         *
         * @return The loop count, or 0 (loop forever when looping) if the definition has no loop block.
         */
        [[nodiscard]] AmUInt32 GetLoopCount() const;

        /**
         * @brief Gets the format of this sound's decoded audio.
         *
         * @return The sound format.
         */
        [[nodiscard]] AM_INLINE const SoundFormat& GetFormat() const
        {
            return _format;
        }

    private:
        bool _loaded;

        std::shared_ptr<Codec> _codec;
        std::shared_ptr<Codec::Decoder> _decoder;

        bool _stream;
        bool _loop;
        AmUInt32 _loopCount;

        SoundChunk* _soundData;
        SoundFormat _format;
        RefCounter _soundDataRefCounter;

        RtpcValue _nearFieldGain;
        RtpcValue _spread;

        SoundInstanceSettings _settings;
    };

    class SoundInstance
    {
        friend class AmplimixImpl;
        friend class SoundImpl;

    public:
        /**
         * @brief Creates a new SoundInstance from the given Sound.
         *
         * @param parent The parent Sound from which to create an instance.
         * @param settings The settings of the Sound instance.
         * @param effect The sound effect to apply on playback.
         */
        SoundInstance(SoundImpl* parent, SoundInstanceSettings settings, const EffectImpl* effect = nullptr);
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
         * @brief Moves the stream decoder out of this instance (game thread).
         *
         * Streamed instances only. The caller owns the decoder from here on and is responsible for closing it; the
         * instance no longer closes it in Destroy().
         *
         * @return The decoder, or @c nullptr if this instance is not streamed, has no decoder, or already gave it away.
         */
        [[nodiscard]] std::shared_ptr<Codec::Decoder> TakeStreamDecoder();

        /**
         * @brief Opens a new decoder on the file of this instance (game thread).
         *
         * Used to give every extra stream (a seek, an instance) its own decoder position.
         *
         * @return The opened decoder, or @c nullptr on failure (logged).
         */
        [[nodiscard]] std::shared_ptr<Codec::Decoder> OpenStreamDecoder() const;

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
        [[nodiscard]] Channel GetChannel() const;

        /**
         * @brief Gets the RealChannel in which this SoundInstance is playing.
         *
         * @return The RealChannel instance in which this SoundInstance is playing.
         */
        [[nodiscard]] RealChannel* GetRealChannel() const;

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
        [[nodiscard]] const CollectionImpl* GetCollection() const;

        /**
         * @brief Gets the number of times this SoundInstance have looped.
         *
         * @return The number of times this sound instance have looped.
         */
        [[nodiscard]] AmUInt32 GetCurrentLoopCount() const;

        /**
         * @brief Overrides the number of remaining loop plays (0 loops forever), counting the one about to start.
         *
         * Used to resume a promoted virtual channel with only the loop passes it had left, instead of the sound's
         * full loop count.
         *
         * @param loopCount The number of remaining loop plays.
         */
        void SetLoopCount(AmUInt32 loopCount);

        /**
         * @brief Gets the effect applied to this SoundInstance, if any.
         *
         * @return An EffectInstance object if an effect was applied to the sound which have
         * generated this SoundInstance, or nullptr otherwise.
         */
        [[nodiscard]] const std::shared_ptr<EffectInstance> GetEffect() const;

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
        SoundImpl* _parent;
        const CollectionImpl* _collection;
        const EffectImpl* _effect;
        std::shared_ptr<EffectInstance> _effectInstance;
        std::shared_ptr<Codec::Decoder> _decoder;

        SoundInstanceSettings _settings;

        AmUInt32 _currentLoopCount;

        AmObjectID _id;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_SOUND_H
