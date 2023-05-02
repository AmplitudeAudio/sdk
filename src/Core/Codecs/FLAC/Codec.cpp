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

#define DR_FLAC_IMPLEMENTATION

#include <Core/Codecs/FLAC/Codec.h>

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

    FLACCodec::FLACCodec()
        : Codec("flac")
        , m_allocationCallbacks()
    {
        m_allocationCallbacks.onFree = onFree;
        m_allocationCallbacks.onMalloc = onMalloc;
        m_allocationCallbacks.onRealloc = onRealloc;
    }

    bool FLACCodec::FLACDecoder::Open(const AmOsString& filePath)
    {
        if (!m_codec->CanHandleFile(filePath))
        {
            CallLogFunc("The FLAC codec cannot handle the file: '" AM_OS_CHAR_FMT "'\n", filePath.c_str());
            return false;
        }

        const auto* codec = static_cast<const FLACCodec*>(m_codec);

#if defined(AM_WCHAR_SUPPORTED)
        _flac = drflac_open_file_w(filePath.c_str(), &codec->m_allocationCallbacks);
#else
        _flac = drflac_open_file(filePath.c_str(), &codec->m_allocationCallbacks);
#endif
        if (_flac == nullptr)
        {
            CallLogFunc("Cannot load the FLAC file: '" AM_OS_CHAR_FMT "'\n", filePath.c_str());
            return false;
        }

        m_format.SetAll(
            _flac->sampleRate, _flac->channels, _flac->bitsPerSample, _flac->totalPCMFrameCount, _flac->channels * sizeof(AmAudioSample),
            AM_SAMPLE_FORMAT_FLOAT, // This codec always read frames as float32 values
            AM_SAMPLE_INTERLEAVED // dr_flac always read interleaved frames
        );

        _initialized = true;

        return true;
    }

    bool FLACCodec::FLACDecoder::Close()
    {
        if (_initialized)
        {
            m_format = SoundFormat();
            _initialized = false;
            delete _flac;
            _flac = nullptr;
        }

        // true because it is already closed
        return true;
    }

    AmUInt64 FLACCodec::FLACDecoder::Load(AmVoidPtr out)
    {
        if (!_initialized)
            return 0;

        if (!Seek(0))
            return 0;

        return drflac_read_pcm_frames_f32(_flac, _flac->totalPCMFrameCount, static_cast<AmAudioSampleBuffer>(out));
    }

    AmUInt64 FLACCodec::FLACDecoder::Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length)
    {
        if (!_initialized)
            return 0;

        if (!Seek(offset))
            return 0;

        return drflac_read_pcm_frames_f32(_flac, length, static_cast<AmAudioSampleBuffer>(out));
    }

    bool FLACCodec::FLACDecoder::Seek(AmUInt64 offset)
    {
        return drflac_seek_to_pcm_frame(_flac, offset) == DRFLAC_TRUE;
    }

    bool FLACCodec::FLACEncoder::Open(const AmOsString& filePath)
    {
        _initialized = true;
        return false;
    }

    bool FLACCodec::FLACEncoder::Close()
    {
        if (_initialized)
            return true;

        return true;
    }

    AmUInt64 FLACCodec::FLACEncoder::Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length)
    {
        return 0;
    }

    Codec::Decoder* FLACCodec::CreateDecoder() const
    {
        return new FLACDecoder(this);
    }

    Codec::Encoder* FLACCodec::CreateEncoder() const
    {
        return new FLACEncoder(this);
    }

    bool FLACCodec::CanHandleFile(const AmOsString& filePath) const
    {
        // TODO: Maybe check by extension instead?

#if defined(AM_WCHAR_SUPPORTED)
        drflac* flac = drflac_open_file_w(filePath.c_str(), &m_allocationCallbacks);
#else
        drflac* flac = drflac_open_file(filePath.c_str(), &m_allocationCallbacks);
#endif

        if (flac != nullptr)
        {
            delete flac;
            return true;
        }

        return false;
    }
} // namespace SparkyStudios::Audio::Amplitude::Codecs
