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

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <Core/Codecs/WAV/Codec.h>

namespace SparkyStudios::Audio::Amplitude::Codecs
{
    bool WAVCodec::WAVDecoder::Initialize(AmString filePath)
    {
        if (!m_codec->CanHandleFile(filePath))
        {
            CallLogFunc("The WAV codec cannot handle the file: %s\n", filePath);
            return false;
        }

        if (drwav_init_file(&_wav, filePath, nullptr) == DRWAV_FALSE)
        {
            CallLogFunc("Cannot load the WAV file: %s\n", filePath);
            return false;
        }

        m_format.SetAll(
            _wav.sampleRate, _wav.channels, _wav.bitsPerSample, _wav.channels * _wav.bitsPerSample >> 3,
            AM_SAMPLE_FORMAT_FLOAT, // This codec always read frames as float32 values
            AM_SAMPLE_INTERLEAVED // dr_wav always read interleaved frames
        );

        _initialized = true;

        return true;
    }

    AmUInt64 WAVCodec::WAVDecoder::Load(AmFloat32Buffer out)
    {
        if (!_initialized)
        {
            return 0;
        }

        if (drwav_seek_to_pcm_frame(&_wav, 0) != DRWAV_TRUE)
        {
            return 0;
        }

        return drwav_read_pcm_frames_f32(&_wav, _wav.totalPCMFrameCount, out);
    }

    AmUInt64 WAVCodec::WAVDecoder::Stream(AmFloat32Buffer out, AmUInt64 offset, AmUInt64 length)
    {
        if (!_initialized)
        {
            return 0;
        }

        if (drwav_seek_to_pcm_frame(&_wav, offset) != DRWAV_TRUE)
        {
            return 0;
        }

        return drwav_read_pcm_frames_f32(&_wav, length, out);
    }

    bool WAVCodec::WAVEncoder::Initialize(AmString filePath)
    {
        _initialized = true;
        return false;
    }

    AmUInt64 WAVCodec::WAVEncoder::Write(const float* in, AmUInt64 offset, AmUInt64 length)
    {
        return 0;
    }

    Codec::Decoder* WAVCodec::CreateDecoder() const
    {
        return new WAVDecoder(this);
    }

    Codec::Encoder* WAVCodec::CreateEncoder() const
    {
        return new WAVEncoder(this);
    }

    bool WAVCodec::CanHandleFile(AmString filePath) const
    {
        // TODO: Maybe check by extension instead?

        drwav dummy;
        bool can = drwav_init_file(&dummy, filePath, nullptr) == DRWAV_TRUE;
        drwav_uninit(&dummy);

        return can;
    }
} // namespace SparkyStudios::Audio::Amplitude::Codecs