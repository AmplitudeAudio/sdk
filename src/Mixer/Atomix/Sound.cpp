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

#include "atomix.h"
#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    Sound::Sound()
        : m_collection(nullptr)
        , m_format()
        , _decoder(nullptr)
        , _loop(false)
        , _stream(false)
    {}

    Sound::~Sound()
    {
        delete _decoder;
        _decoder = nullptr;
    }

    void Sound::Initialize(SoundCollection* collection)
    {
        m_collection = collection;
        _stream = collection->GetSoundCollectionDefinition()->stream();
        _loop = collection->GetSoundCollectionDefinition()->play_mode() == PlayMode_LoopOne;
    }

    void Sound::Load(FileLoader* loader)
    {
        if (GetFilename() == nullptr)
        {
            CallLogFunc("The filename is empty.\n");
            return;
        }

        AmOsString filename = GetFilename();

        Codec* codec = Codec::FindCodecForFile(filename);
        if (codec == nullptr)
        {
            CallLogFunc("Unable to find codec for '%s'.\n", filename);
            return;
        }

        _decoder = codec->CreateDecoder();
        if (!_decoder->Open(filename))
        {
            CallLogFunc("Unable to initialize decoder for '%s'.\n", filename);
            return;
        }

        m_format = _decoder->GetFormat();
    }

    SoundInstance* Sound::CreateInstance() const
    {
        return new SoundInstance(this);
    }

    SoundInstance::SoundInstance(const Sound* parent)
        : _parent(parent)
        , _userData(nullptr)
        , _channel(nullptr)
        , _streamBuffer()
    {}

    SoundInstance::~SoundInstance()
    {
        if (_userData)
        {
            atomixSoundDestroy(static_cast<atomix_sound*>(_userData));
            _userData = nullptr;
        }

        _parent = nullptr;
    }

    void SoundInstance::Load()
    {
        AMPLITUDE_ASSERT(Valid());

        AmUInt16 channels = _parent->m_format.GetNumChannels();
        AmUInt32 frames = _parent->m_format.GetFramesCount();

        if (_parent->_stream)
        {
            _streamBuffer.Init(ATOMIX_MAX_STREAM_BUFFER_SIZE * _parent->m_format.GetNumChannels());

            atomix_sound* sound = atomixSoundNew(channels, _streamBuffer.GetBuffer(), frames, true, this);
            if (!sound)
            {
                CallLogFunc("Could not load a sound instance. Unable to read data from the parent sound.\n");
                return;
            }

            SetUserData(sound);
        }
        else
        {
            AmAlignedReal32Buffer buffer;
            buffer.Init(frames * channels);

            if (_parent->_decoder->Load(buffer.GetBuffer()) != frames)
            {
                CallLogFunc("Could not load a sound instance. Unable to read data from the parent sound.\n");
                return;
            }

            atomix_sound* sound = atomixSoundNew(channels, buffer.GetBuffer(), frames, false, this);

            if (!sound)
            {
                CallLogFunc("Could not load a sound instance. Unable to read data from the parent sound.\n");
                return;
            }

            SetUserData(sound);
        }
    }

    AmUInt64 SoundInstance::GetAudio(AmUInt64 offset, AmUInt64 frames)
    {
        AMPLITUDE_ASSERT(Valid());

        if (_parent->_stream)
        {
            _streamBuffer.Clear();

            AmUInt64 n, l = frames, o = offset, r = 0;
            AmReal32Buffer b = _streamBuffer.GetBuffer();

        Fill:
            n = _parent->_decoder->Stream(b, o, l);
            r += n;

            // If we reached the end of the file but looping is enabled, then
            // seek back to the beginning of the file and fill the remaining part of the buffer.
            if (n < l && _parent->_loop && _parent->_decoder->Seek(0))
            {
                b += n * _parent->m_format.GetNumChannels();
                l -= n;
                goto Fill;
            }

            return r;
        }

        return 0;
    }

    void SoundInstance::Destroy()
    {
        AMPLITUDE_ASSERT(Valid());

        if (_parent->_stream)
        {
            _parent->_decoder->Close();
            _streamBuffer = AmAlignedReal32Buffer();
        }
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
} // namespace SparkyStudios::Audio::Amplitude