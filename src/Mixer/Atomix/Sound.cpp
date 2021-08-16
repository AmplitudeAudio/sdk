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
    void SoundFormat::SetAll(
        AmUInt32 sampleRate,
        AmUInt16 numChannels,
        AmUInt32 bitsPerSample,
        AmUInt32 frameSize,
        AM_SAMPLE_FORMAT sampleType,
        AM_INTERLEAVE_TYPE interleaveType)
    {
        _sampleRate = sampleRate;
        _numChannels = numChannels;
        _bitsPerSample = bitsPerSample;
        _frameSize = frameSize;
        _sampleType = sampleType;
        _interleaveType = interleaveType;
    }

    Sound::~Sound()
    {
        if (_userData)
        {
            atomixSoundDestroy(static_cast<atomix_sound*>(_userData));
            _userData = nullptr;
        }
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
            CallLogFunc("The filename is empty");
            return;
        }

        AmString filename = GetFilename().c_str();
        const EngineConfigDefinition* config = Engine::GetInstance()->GetEngineConfigDefinition();

        Codec* codec = Codec::FindCodecForFile(filename);
        codec

        ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, config->output()->channels(), config->output()->frequency());
        ma_uint64 size;

        if (_stream)
        {
            _streamDecoder = {};
            if (ma_decoder_init_file(filename, &cfg, &_streamDecoder) != MA_SUCCESS)
            {
                CallLogFunc("LoadFile fail on %s", filename);
                return;
            }

            size = ma_decoder_get_length_in_pcm_frames(&_streamDecoder);

            _channels = _streamDecoder.outputChannels;
            _streamBuffer = new float[ATOMIX_MAX_STREAM_BUFFER_SIZE * _channels];

            atomix_sound* atmxSound = atomixSoundNew(_channels, _streamBuffer, (int)size, true, this);

            if (!atmxSound)
            {
                CallLogFunc("Could not load sound file: %s.", GetFilename().c_str());
                return;
            }

            SetUserData(atmxSound);
        }
        else
        {
            AmVoidPtr data;

            if (ma_decode_file(filename, &cfg, &size, &data) != MA_SUCCESS)
            {
                CallLogFunc("LoadFile fail on %s", filename);
                return;
            }

            _channels = cfg.channels;

            atomix_sound* atmxSound = atomixSoundNew(_channels, (AmFloat32Buffer)data, (int)size, false, this);
            ma_free(data, nullptr); // We can delete the buffer here since atomix will own the data.

            if (!atmxSound)
            {
                CallLogFunc("Could not load sound file: %s.", GetFilename().c_str());
                return;
            }

            SetUserData(atmxSound);
        }

        _size = size;
    }

    AmInt32 Sound::GetAudio(AmUInt32 offset, AmUInt32 frames)
    {
        if (_stream)
        {
            ma_uint64 n = 0, l = frames, r = 0;
            AmFloat32Buffer b = &_streamBuffer[0];

        fill:
            n = ma_decoder_read_pcm_frames(&_streamDecoder, b, l);
            r += n;

            // If we reached the end of the file but looping is enabled, then
            // seek back to the beginning of the file and fill the remaining part of the buffer.
            if (n < l && _loop && ma_decoder_seek_to_pcm_frame(&_streamDecoder, 0) == MA_SUCCESS)
            {
                b += n * _channels;
                l -= n;
                goto fill;
            }

            return (int)r;
        }

        return 0;
    }

    void Sound::Destroy()
    {
        if (_stream)
        {
            ma_decoder_uninit(&_streamDecoder);
            delete[] _streamBuffer;
        }
    }

} // namespace SparkyStudios::Audio::Amplitude