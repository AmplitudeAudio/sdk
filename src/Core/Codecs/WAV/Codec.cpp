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
    static void* onMalloc(size_t sz, void* pUserData)
    {
        return amMemory->Malloc(MemoryPoolKind::Codec, sz);
    }

    static void* onRealloc(void* p, size_t sz, void* pUserData)
    {
        return amMemory->Realloc(MemoryPoolKind::Codec, p, sz);
    }

    static void onFree(void* p, void* pUserData)
    {
        amMemory->Free(MemoryPoolKind::Codec, p);
    }

    WAVCodec::WAVCodec()
        : Codec("wav")
        , m_allocationCallbacks()
    {
        m_allocationCallbacks.onFree = onFree;
        m_allocationCallbacks.onMalloc = onMalloc;
        m_allocationCallbacks.onRealloc = onRealloc;
    }

    bool WAVCodec::WAVDecoder::Open(AmOsString filePath)
    {
        if (!m_codec->CanHandleFile(filePath))
        {
            CallLogFunc("The WAV codec cannot handle the file: '%s'\n", filePath);
            return false;
        }

        const auto* codec = static_cast<const WAVCodec*>(m_codec);

#if defined(AM_WCHAR_SUPPORTED)
        if (drwav_init_file_w(&_wav, filePath, &codec->m_allocationCallbacks) == DRWAV_FALSE)
#else
        if (drwav_init_file(&_wav, filePath, &codec->m_allocationCallbacks) == DRWAV_FALSE)
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

        if (!Seek(0))
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
        if (!m_codec->CanHandleFile(filePath))
        {
            CallLogFunc("The WAV codec cannot handle the file: '%s'\n", filePath);
            return false;
        }

        if (!_isFormatSet)
        {
            CallLogFunc("The WAV codec cannot open the file '%s' without a format set. Have you missed to call SetFormat()?\n", filePath);
            return false;
        }

        drwav_data_format format;
        format.container = drwav_container_riff; // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
        format.format = DR_WAVE_FORMAT_PCM; // <-- Any of the DR_WAVE_FORMAT_* codes.
        format.channels = m_format.GetNumChannels(); // <-- Only 2 channels are supported for now.
        format.sampleRate = m_format.GetSampleRate();
        format.bitsPerSample = m_format.GetBitsPerSample();

        const auto* codec = static_cast<const WAVCodec*>(m_codec);

#if defined(AM_WCHAR_SUPPORTED)
        if (drwav_init_file_write_sequential_pcm_frames_w(
                &_wav, filePath, &format, m_format.GetFramesCount(), &codec->m_allocationCallbacks) == DRWAV_FALSE)
#else
        if (drwav_init_file_write_sequential_pcm_frames(
                &_wav, filePath, &format, m_format.GetFramesCount(), &codec->m_allocationCallbacks) == DRWAV_FALSE)
#endif
        {
            CallLogFunc("Cannot load the WAV file: '%s'\n.", filePath);
            return false;
        }

        _initialized = true;

        return true;
    }

    void WAVCodec::WAVEncoder::SetFormat(const SoundFormat& format)
    {
        if (_initialized)
        {
            CallLogFunc("Cannot set the format on an initialized decoder.\n");
            return;
        }

        m_format = format;
        _isFormatSet = true;
    }

    bool WAVCodec::WAVEncoder::Close()
    {
        if (_initialized)
        {
            m_format = SoundFormat();
            _isFormatSet = false;
            _initialized = false;
            return drwav_uninit(&_wav) == DRWAV_TRUE;
        }

        return true;
    }

    AmUInt64 WAVCodec::WAVEncoder::Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length)
    {
        if (!_initialized)
        {
            return 0;
        }

        return drwav_write_pcm_frames(&_wav, length, in);
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
        const bool can = drwav_init_file_w(&dummy, filePath, &m_allocationCallbacks) == DRWAV_TRUE;
#else
        const bool can = drwav_init_file(&dummy, filePath, &m_allocationCallbacks) == DRWAV_TRUE;
#endif

        if (can)
        {
            drwav_uninit(&dummy);
        }

        return can;
    }
} // namespace SparkyStudios::Audio::Amplitude::Codecs