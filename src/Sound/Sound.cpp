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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <Mixer/SoundData.h>

#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static AmObjectID gLastSoundInstanceID = 0;

    Sound::Sound()
        : SoundObject()
        , _codec(nullptr)
        , _decoder(nullptr)
        , _stream(false)
        , _loop(false)
        , _loopCount(0)
        , _settings()
        , _soundData(nullptr)
        , _format()
        , _soundDataRefCounter()
    {}

    Sound::~Sound()
    {
        if (_decoder != nullptr)
        {
            _decoder->Close();
            _codec->DestroyDecoder(_decoder);

            _decoder = nullptr;
            _codec = nullptr;
        }

        if (_soundData != nullptr)
        {
            AMPLITUDE_ASSERT(_soundDataRefCounter.GetCount() == 0);

            SoundChunk::DestroyChunk(_soundData);
            _soundData = nullptr;
        }

        m_bus = nullptr;
        m_effect = nullptr;
        m_attenuation = nullptr;
    }

    SoundInstance* Sound::CreateInstance()
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);
        return ampoolnew(MemoryPoolKind::Engine, SoundInstance, this, _settings, m_effect);
    }

    SoundInstance* Sound::CreateInstance(const Collection* collection)
    {
        if (collection == nullptr)
            return CreateInstance();

        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        auto* sound = ampoolnew(MemoryPoolKind::Engine, SoundInstance, this, collection->_soundSettings.at(_id), collection->m_effect);
        sound->_collection = collection;

        return sound;
    }

    SoundChunk* Sound::AcquireSoundData()
    {
        if (_stream)
            return nullptr;

        if (_soundDataRefCounter.GetCount() == 0)
        {
            _soundData = SoundChunk::CreateChunk(_format.GetFramesCount(), _format.GetNumChannels());

            if (_decoder->Load(reinterpret_cast<AmAudioSampleBuffer>(_soundData->buffer)) != _format.GetFramesCount())
            {
                SoundChunk::DestroyChunk(_soundData);
                amLogError("Could not load a sound instance. Unable to read data from the parent sound.");
                return nullptr;
            }
        }

        _soundDataRefCounter.Increment();
        return _soundData;
    }

    void Sound::ReleaseSoundData()
    {
        if (_stream)
            return;

        _soundDataRefCounter.Decrement();

        if (_soundDataRefCounter.GetCount() == 0)
        {
            SoundChunk::DestroyChunk(_soundData);
            _soundData = nullptr;
        }
    }

    bool Sound::IsStream() const
    {
        return _stream;
    }

    bool Sound::IsLoop() const
    {
        return _loop;
    }

    void Sound::Load(const FileSystem* loader)
    {
        const AmOsString& filename = GetPath();

        if (filename.empty())
        {
            amLogError("Cannot load the sound: the filename is empty.");
            return;
        }

        if (!loader->Exists(filename))
        {
            amLogError("Cannot load the sound: the file \"{}\" does not exist.", filename);
            return;
        }

        const auto file = loader->OpenFile(filename);

        _codec = Codec::FindCodecForFile(file);
        if (_codec == nullptr)
        {
            amLogError("Cannot load the sound: unable to find codec for '{}'.", filename);
            return;
        }

        _decoder = _codec->CreateDecoder();
        if (!_decoder->Open(file))
        {
            amLogError("Cannot load the sound: unable to initialize a decoder for '{}'.", filename);
            return;
        }

        _format = _decoder->GetFormat();
    }

    bool Sound::LoadDefinition(const SoundDefinition* definition, EngineInternalState* state)
    {
        if (definition->id() == kAmInvalidObjectId)
        {
            amLogError("Sound definition is invalid: no ID defined.");
            return false;
        }

        if (definition->bus() == kAmInvalidObjectId)
        {
            amLogError("Sound definition is invalid: no bus ID defined.");
            return false;
        }

        m_bus = FindBusInternalState(state, definition->bus());
        if (!m_bus)
        {
            amLogError("Sound {} specifies an unknown bus ID: {}.", definition->name()->str(), definition->bus());
            return false;
        }

        if (definition->effect() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->effect_map.find(definition->effect()); findIt != state->effect_map.end())
            {
                m_effect = findIt->second.get();
            }
            else
            {
                amLogError("Sound definition is invalid: invalid effect ID \"{}\"", definition->effect());
                return false;
            }
        }

        if (definition->attenuation() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->attenuation_map.find(definition->attenuation()); findIt != state->attenuation_map.end())
            {
                m_attenuation = findIt->second.get();
            }
            else
            {
                amLogError("Sound definition is invalid: invalid attenuation ID \"{}\"", definition->attenuation());
                return false;
            }
        }

        _id = definition->id();
        _name = definition->name()->str();

        auto* fs = amEngine->GetFileSystem();

        _stream = definition->stream();
        _loop = definition->loop() != nullptr && definition->loop()->enabled();
        _loopCount = definition->loop() ? definition->loop()->loop_count() : 0;
        _filename = fs->ResolvePath(fs->Join({ AM_OS_STRING("data"), AM_STRING_TO_OS_STRING(definition->path()->str()) }));

        RtpcValue::Init(m_gain, definition->gain(), 1);
        RtpcValue::Init(m_pitch, definition->pitch(), 1);
        RtpcValue::Init(m_priority, definition->priority(), 1);

        _settings.m_id = definition->id();
        _settings.m_kind = SoundKind::Standalone;
        _settings.m_busID = definition->bus();
        _settings.m_effectID = definition->effect();
        _settings.m_attenuationID = definition->attenuation();
        _settings.m_spatialization = definition->spatialization();
        _settings.m_priority = RtpcValue(m_priority);
        _settings.m_gain = RtpcValue(m_gain);
        _settings.m_pitch = RtpcValue(m_pitch);
        _settings.m_loop = _loop;
        _settings.m_loopCount = _loopCount;

        return true;
    }

    const SoundDefinition* Sound::GetDefinition() const
    {
        return GetSoundDefinition(_source.c_str());
    }

    void Sound::AcquireReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        if (m_effect)
            m_effect->GetRefCounter()->Increment();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Increment();
    }

    void Sound::ReleaseReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(_id != kAmInvalidObjectId);

        if (m_effect)
            m_effect->GetRefCounter()->Decrement();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Decrement();
    }

    SoundInstance::SoundInstance(Sound* parent, SoundInstanceSettings settings, const Effect* effect)
        : _userData(nullptr)
        , _channel(nullptr)
        , _parent(parent)
        , _collection(nullptr)
        , _effect(effect)
        , _effectInstance(nullptr)
        , _decoder(nullptr)
        , _settings(std::move(settings))
        , _currentLoopCount(0)
        , _obstruction(0.0f)
        , _occlusion(0.0f)
        , _id(++gLastSoundInstanceID)
    {
        if (_effect != nullptr)
            _effectInstance = _effect->CreateInstance();

        if (_parent->_stream)
        {
            const auto filename = _parent->GetPath();
            const auto file = amEngine->GetFileSystem()->OpenFile(filename);

            _decoder = _parent->_codec->CreateDecoder();
            if (!_decoder->Open(file))
            {
                amLogError("Cannot load the sound: unable to initialize a decoder for '{}'.", filename);
                return;
            }
        }
    }

    SoundInstance::~SoundInstance()
    {
        Destroy();
    }

    void SoundInstance::Load()
    {
        AMPLITUDE_ASSERT(Valid());

        const AmUInt16 channels = _parent->_format.GetNumChannels();
        const AmUInt64 frames = _parent->_format.GetFramesCount();

        SoundData* data;
        SoundChunk* chunk;

        if (_parent->_stream)
        {
            chunk = SoundChunk::CreateChunk(amEngine->GetSamplesPerStream(), channels);
            data = SoundData::CreateMusic(_parent->_format, chunk, frames, this);
        }
        else
        {
            chunk = _parent->AcquireSoundData();
            data = SoundData::CreateSound(_parent->_format, chunk, frames, this);
        }

        if (data == nullptr)
        {
            amLogError("Could not load a sound instance. Unable to read data from the parent sound.");

            if (chunk != nullptr)
            {
                if (_parent->_stream)
                    SoundChunk::DestroyChunk(chunk);
                else
                    _parent->ReleaseSoundData();
            }

            return;
        }

        SetUserData(data);
    }

    const SoundInstanceSettings& SoundInstance::GetSettings() const
    {
        return _settings;
    }

    AmVoidPtr SoundInstance::GetUserData() const
    {
        return _userData;
    }

    void SoundInstance::SetUserData(AmVoidPtr userData)
    {
        _userData = userData;
    }

    AmUInt64 SoundInstance::GetAudio(AmUInt64 offset, AmUInt64 frames) const
    {
        AMPLITUDE_ASSERT(Valid());

        if (!_parent->_stream || _userData == nullptr)
            return 0;

        const auto* data = static_cast<SoundData*>(_userData);

        const AmUInt16 channels = _parent->_format.GetNumChannels();

        AmUInt64 l = frames, o = offset, r = 0;
        auto b = reinterpret_cast<AmAudioSampleBuffer>(data->chunk->buffer);

        bool needFill = true;
        do
        {
            const AmUInt64 n = _decoder->Stream(b, o, l);
            r += n;

            // If we reached the end of the file but looping is enabled, then
            // seek back to the beginning of the file and fill the remaining part of the buffer.
            if (needFill = n < l && _parent->_loop && _decoder->Seek(0); needFill)
            {
                b += n * channels;
                l -= n;
                o = 0;
            }
        } while (needFill);

        return r;
    }

    void SoundInstance::Destroy()
    {
        AMPLITUDE_ASSERT(Valid());

        if (_userData != nullptr)
        {
            SoundData::Destroy(static_cast<SoundData*>(_userData), _parent->_stream);

            if (!_parent->_stream)
                _parent->ReleaseSoundData();
        }

        _userData = nullptr;

        _effect->DestroyInstance(_effectInstance);
        _effectInstance = nullptr;

        if (_parent->_stream)
        {
            _decoder->Close();
            _parent->_codec->DestroyDecoder(_decoder);
        }

        _decoder = nullptr;

        _parent = nullptr;
    }

    bool SoundInstance::Valid() const
    {
        return _parent != nullptr;
    }

    void SoundInstance::SetChannel(RealChannel* channel)
    {
        _channel = channel;
    }

    RealChannel* SoundInstance::GetChannel() const
    {
        return _channel;
    }

    const Sound* SoundInstance::GetSound() const
    {
        return _parent;
    }

    const Collection* SoundInstance::GetCollection() const
    {
        return _collection;
    }

    AmUInt32 SoundInstance::GetCurrentLoopCount() const
    {
        return _currentLoopCount;
    }

    const EffectInstance* SoundInstance::GetEffect() const
    {
        return _effectInstance;
    }

    void SoundInstance::SetObstruction(AmReal32 obstruction)
    {
        _obstruction = obstruction;
    }

    void SoundInstance::SetOcclusion(AmReal32 occlusion)
    {
        _occlusion = occlusion;
    }

    AmReal32 SoundInstance::GetObstruction() const
    {
        return _obstruction;
    }

    AmReal32 SoundInstance::GetOcclusion() const
    {
        return _occlusion;
    }

    AmObjectID SoundInstance::GetId() const
    {
        return _id;
    }
} // namespace SparkyStudios::Audio::Amplitude
