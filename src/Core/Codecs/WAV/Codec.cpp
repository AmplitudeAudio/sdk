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
    bool WAVCodec::WAVDecoder::Open(AmOsString filePath)
    {
        if (!m_codec->CanHandleFile(filePath))
        {
            CallLogFunc("The WAV codec cannot handle the file: '%s'\n", filePath);
            return false;
        }

#if defined(AM_WCHAR_SUPPORTED)
        if (drwav_init_file_w(&_wav, filePath, nullptr) == DRWAV_FALSE)
#else
        if (drwav_init_file(&_wav, filePath, nullptr) == DRWAV_FALSE)
#endif
        {
            CallLogFunc("Cannot load the WAV file: '%s'\n.", filePath);
            return false;
        }

        m_format.SetAll(
            _wav.sampleRate, _wav.channels, _wav.bitsPerSample, _wav.totalPCMFrameCount, _wav.channels * sizeof(AmInt16),
            AM_SAMPLE_FORMAT_INT, // This codec always read frames as int16 values
            AM_SAMPLE_INTERLEAVED // dr_wav always read interleaved frames
        );

        _initialized = true;

        return true;
    }

    bool WAVCodec::WAVDecoder::Close()
    {
        if (_initialized)
        {
            m_format = SoundFormat();
            _initialized = false;
            return drwav_uninit(&_wav) == DRWAV_TRUE;
        }

        // true because it is already closed
        return true;
    }

    AmUInt64 WAVCodec::WAVDecoder::Load(AmVoidPtr out)
    {
        if (!_initialized)
        {
            return 0;
        }

        if (drwav_seek_to_pcm_frame(&_wav, 0) != DRWAV_TRUE)
        {
            return 0;
        }

        return drwav_read_pcm_frames_s16(&_wav, _wav.totalPCMFrameCount, static_cast<AmInt16Buffer>(out));
    }

    AmUInt64 WAVCodec::WAVDecoder::Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length)
    {
        if (!_initialized)
        {
            return 0;
        }

        if (!Seek(offset))
        {
            return 0;
        }

        return drwav_read_pcm_frames_s16(&_wav, length, static_cast<AmInt16Buffer>(out));
    }

    bool WAVCodec::WAVDecoder::Seek(AmUInt64 offset)
    {
        return drwav_seek_to_pcm_frame(&_wav, offset) == DRWAV_TRUE;
    }

    bool WAVCodec::WAVEncoder::Open(AmOsString filePath)
    {
        _initialized = true;
        return false;
    }

    bool WAVCodec::WAVEncoder::Close()
    {
        if (_initialized)
        {
            return true;
        }

        return true;
    }

    AmUInt64 WAVCodec::WAVEncoder::Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length)
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

    bool WAVCodec::CanHandleFile(AmOsString filePath) const
    {
        // TODO: Maybe check by extension instead?

        drwav dummy;
#if defined(AM_WCHAR_SUPPORTED)
        const bool can = drwav_init_file_w(&dummy, filePath, nullptr) == DRWAV_TRUE;
#else
        const bool can = drwav_init_file(&dummy, filePath, nullptr) == DRWAV_TRUE;
#endif

        if (can)
        {
            drwav_uninit(&dummy);
        }

        return can;
    }
} // namespace SparkyStudios::Audio::Amplitude::Codecs