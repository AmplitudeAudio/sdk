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
        , _streamBuffer(nullptr)
        , _stream(false)
        , _userData(nullptr)
    {}

    Sound::~Sound()
    {
        if (_userData)
        {
            atomixSoundDestroy(static_cast<atomix_sound*>(_userData));
            _userData = nullptr;
        }

        delete _streamBuffer;
        _streamBuffer = nullptr;

        delete _decoder;
        _decoder = nullptr;
    }

    void Sound::Initialize(const SoundCollection* collection)
    {
        m_collection = collection;
        _stream = collection->GetSoundCollectionDefinition()->stream();
        _loop = collection->GetSoundCollectionDefinition()->loop();
    }

    void Sound::Load(FileLoader* loader)
    {
        if (GetFilename().empty())
        {
            CallLogFunc("The filename is empty.\n");
            return;
        }

        AmString filename = GetFilename().c_str();
        const EngineConfigDefinition* config = Engine::GetInstance()->GetEngineConfigDefinition();

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

        if (_stream)
        {
            _streamBuffer = new AmAlignedFloat32Buffer();
            _streamBuffer->Init(ATOMIX_MAX_STREAM_BUFFER_SIZE * m_format.GetNumChannels());

            atomix_sound* sound =
                atomixSoundNew(m_format.GetNumChannels(), _streamBuffer->GetBuffer(), m_format.GetFramesCount(), true, this);
            if (!sound)
            {
                CallLogFunc("Could not load sound file '%s'.\n", filename);
                return;
            }

            SetUserData(sound);
        }
        else
        {
            auto data = (AmFloat32Buffer)malloc(m_format.GetFramesCount() * m_format.GetFrameSize());
            if (_decoder->Load(data) != m_format.GetFramesCount())
            {
                CallLogFunc("Could not load sound file '%s'.\n", filename);
                return;
            }

            atomix_sound* sound = atomixSoundNew(m_format.GetNumChannels(), data, m_format.GetFramesCount(), false, this);
            free(data);

            if (!sound)
            {
                CallLogFunc("Could not load sound file '%s'.\n", filename);
                return;
            }

            SetUserData(sound);
        }
    }

    AmUInt64 Sound::GetAudio(AmUInt64 offset, AmUInt64 frames)
    {
        if (_stream)
        {
            _streamBuffer->Clear();

            AmUInt64 n, l = frames, o = offset, r = 0;
            AmFloat32Buffer b = _streamBuffer->GetBuffer();

        fill:
            n = _decoder->Stream(b, o, l);
            r += n;

            // If we reached the end of the file but looping is enabled, then
            // seek back to the beginning of the file and fill the remaining part of the buffer.
            if (n < l && _loop && _decoder->Seek(0))
            {
                b += n * m_format.GetNumChannels();
                l -= n;
                goto fill;
            }

            return r;
        }

        return 0;
    }

    void Sound::Destroy()
    {
        if (_stream)
        {
            _decoder->Close();
            delete _streamBuffer;
        }
    }
} // namespace SparkyStudios::Audio::Amplitude