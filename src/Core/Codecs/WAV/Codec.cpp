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

#include <cstring>

#include <Core/Codecs/WAV/Codec.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    static void* onMalloc(size_t sz, void* pUserData)
    {
        return ampoolmalloc(eMemoryPoolKind_Codec, sz);
    }

    static void* onRealloc(void* p, size_t sz, void* pUserData)
    {
        return ampoolrealloc(eMemoryPoolKind_Codec, p, sz);
    }

    static void onFree(void* p, void* pUserData)
    {
        ampoolfree(eMemoryPoolKind_Codec, p);
    }

    static size_t onRead(void* pUserData, void* pBufferOut, size_t bytesToRead)
    {
        auto* file = static_cast<File*>(pUserData);
        return file->Read(static_cast<AmUInt8Buffer>(pBufferOut), bytesToRead);
    }

    static drwav_bool32 onSeek(void* pUserData, int offset, drwav_seek_origin origin)
    {
        auto* file = static_cast<File*>(pUserData);
        file->Seek(
            offset,
            origin == drwav_seek_origin_start         ? eFileSeekOrigin_Start
                : origin == drwav_seek_origin_current ? eFileSeekOrigin_Current
                                                      : eFileSeekOrigin_Start);
        return DRWAV_TRUE;
    }

    static size_t onWrite(void* pUserData, const void* pData, size_t bytesToWrite)
    {
        auto* file = static_cast<File*>(pUserData);
        return file->Write(static_cast<AmConstUInt8Buffer>(pData), bytesToWrite);
    }

    WAVCodec::WAVCodec()
        : Codec("wav")
        , m_allocationCallbacks()
    {
        m_allocationCallbacks.onFree = onFree;
        m_allocationCallbacks.onMalloc = onMalloc;
        m_allocationCallbacks.onRealloc = onRealloc;
    }

    bool WAVCodec::WAVDecoder::Open(std::shared_ptr<File> file)
    {
        if (!m_codec->CanHandleFile(file))
        {
            amLogError("The WAV codec cannot handle the file: '" AM_OS_CHAR_FMT "'.", file->GetPath().c_str());
            return false;
        }

        _file = file;
        const auto* codec = static_cast<const WAVCodec*>(m_codec);

        if (drwav_init(&_wav, onRead, onSeek, _file.get(), &codec->m_allocationCallbacks) == DRWAV_FALSE)
        {
            amLogError("Cannot load the WAV file: '" AM_OS_CHAR_FMT "'.", file->GetPath().c_str());
            return false;
        }

        m_format.SetAll(
            _wav.sampleRate, _wav.channels, _wav.bitsPerSample, _wav.totalPCMFrameCount, _wav.channels * sizeof(AmAudioSample),
            eAudioSampleFormat_Float32 // This codec always read frames as float32 values
        );

        _initialized = true;

        return true;
    }

    bool WAVCodec::WAVDecoder::Close()
    {
        if (!_initialized)
            return true; // true because it is already closed

        _file.reset();

        m_format = SoundFormat();
        _initialized = false;

        return drwav_uninit(&_wav) == DRWAV_SUCCESS;
    }

    AmUInt64 WAVCodec::WAVDecoder::Load(AudioBuffer* out)
    {
        return Stream(out, 0, 0, _wav.totalPCMFrameCount);
    }

    AmUInt64 WAVCodec::WAVDecoder::Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length)
    {
        if (!_initialized)
            return 0;

        if (!Seek(seekOffset))
            return 0;

        AmAlignedReal32Buffer buffer;
        buffer.Init(length * _wav.channels);

        const AmUInt64 read = drwav_read_pcm_frames_f32(&_wav, length, buffer.GetBuffer());

        Deinterleave(buffer.GetBuffer(), 0, out->GetData().GetBuffer(), bufferOffset, length, _wav.channels);

        return read;
    }

    bool WAVCodec::WAVDecoder::Seek(AmUInt64 offset)
    {
        return drwav_seek_to_pcm_frame(&_wav, offset) == DRWAV_TRUE;
    }

    bool WAVCodec::WAVEncoder::Open(std::shared_ptr<File> file)
    {
        if (!_isFormatSet)
        {
            amLogError(
                "The WAV codec cannot open the file '" AM_OS_CHAR_FMT "' without a format set. Have you missed to call SetFormat()?",
                file->GetPath().c_str());
            return false;
        }

        drwav_data_format format;
        format.container = drwav_container_riff;
        format.channels = m_format.GetNumChannels();
        format.sampleRate = m_format.GetSampleRate();
        format.bitsPerSample = m_format.GetBitsPerSample();

        // Select the WAV format tag based on the sample type
        if (m_format.GetSampleType() == eAudioSampleFormat_Float32 && m_format.GetBitsPerSample() == 32)
            format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        else
            format.format = DR_WAVE_FORMAT_PCM;

        _file = file;
        const auto* codec = static_cast<const WAVCodec*>(m_codec);

        if (drwav_init_write_sequential_pcm_frames(
                &_wav, &format, m_format.GetFramesCount(), onWrite, _file.get(), &codec->m_allocationCallbacks) == DRWAV_FALSE)
        {
            amLogError("Cannot load the WAV file: '" AM_OS_CHAR_FMT "'.", file->GetPath().c_str());
            return false;
        }

        _initialized = true;

        return true;
    }

    void WAVCodec::WAVEncoder::SetFormat(const SoundFormat& format)
    {
        if (_initialized)
        {
            amLogError("Cannot set the format on an initialized decoder.");
            return;
        }

        m_format = format;
        _isFormatSet = true;
    }

    bool WAVCodec::WAVEncoder::Close()
    {
        if (!_initialized)
            return true;

        m_format = SoundFormat();
        _isFormatSet = false;
        _initialized = false;

        return drwav_uninit(&_wav) == DRWAV_SUCCESS;
    }

    AmUInt64 WAVCodec::WAVEncoder::Write(AudioBuffer* in, AmUInt64 offset, AmUInt64 length)
    {
        if (!_initialized)
            return 0;

        const AmUInt64 totalSamples = length * _wav.channels;

        AmAlignedReal32Buffer buffer;
        buffer.Init(totalSamples);

        Interleave(in, 0, buffer.GetBuffer(), 0, length, _wav.channels);

        // For IEEE float format, write float32 data directly
        if (_wav.translatedFormatTag == DR_WAVE_FORMAT_IEEE_FLOAT)
        {
            return drwav_write_pcm_frames(&_wav, length, buffer.GetBuffer());
        }

        // For PCM format, convert float32 to the appropriate integer format before writing
        if (_wav.bitsPerSample == 16)
        {
            std::vector<drwav_int16> pcmBuffer(totalSamples);
            drwav_f32_to_s16(pcmBuffer.data(), buffer.GetBuffer(), static_cast<size_t>(totalSamples));
            return drwav_write_pcm_frames(&_wav, length, pcmBuffer.data());
        }

        if (_wav.bitsPerSample == 32)
        {
            std::vector<drwav_int32> pcmBuffer(totalSamples);
            drwav_f32_to_s32(pcmBuffer.data(), buffer.GetBuffer(), static_cast<size_t>(totalSamples));
            return drwav_write_pcm_frames(&_wav, length, pcmBuffer.data());
        }

        // For other bit depths (8, 24, etc.), write float data as-is (best-effort)
        return drwav_write_pcm_frames(&_wav, length, buffer.GetBuffer());
    }

    std::shared_ptr<Codec::Decoder> WAVCodec::CreateDecoder()
    {
        return ampoolshared(eMemoryPoolKind_Codec, WAVDecoder, this);
    }

    std::shared_ptr<Codec::Encoder> WAVCodec::CreateEncoder()
    {
        return ampoolshared(eMemoryPoolKind_Codec, WAVEncoder, this);
    }

    bool WAVCodec::CanHandleFile(std::shared_ptr<File> file) const
    {
        if (!file)
            return false;

        const auto pos = file->Position();
        file->Seek(0, eFileSeekOrigin_Start);

        // Read RIFF header (12) + fmt chunk tag (4) + fmt chunk size (4) + format tag (2) = 22 bytes
        AmUInt8 header[22] = {};
        const bool hasEnoughData = file->Read(header, sizeof(header)) == sizeof(header);

        file->Seek(static_cast<AmInt64>(pos), eFileSeekOrigin_Start);

        if (!hasEnoughData)
            return false;

        // Verify RIFF WAVE container
        if (std::memcmp(header, "RIFF", 4) != 0 || std::memcmp(header + 8, "WAVE", 4) != 0)
            return false;

        // Verify "fmt " sub-chunk follows the RIFF header
        if (std::memcmp(header + 12, "fmt ", 4) != 0)
            return false;

        // Read the format tag (little-endian uint16 at offset 20)
        const auto formatTag = static_cast<AmUInt16>(header[20] | (header[21] << 8));

        // Accept standard WAV formats; reject IMA/DVI ADPCM (0x11) which is handled by the AMS codec
        return formatTag != DR_WAVE_FORMAT_DVI_ADPCM;
    }
} // namespace SparkyStudios::Audio::Amplitude
