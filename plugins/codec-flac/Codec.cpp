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

#include <Codec.h>

void FlacCodec::FlacDecoderInternal::set_current_output_buffer(AudioBuffer* output, AmUInt64 size, AmUInt64 offset)
{
    _current_output_buffer = output;
    _current_output_buffer_size = size;
    _current_output_buffer_offset = offset;
}

::FLAC__StreamDecoderWriteStatus FlacCodec::FlacDecoderInternal::write_callback(
    const ::FLAC__Frame* frame, const FLAC__int32* const buffer[])
{
    if (_current_output_buffer == nullptr)
        return ::FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    AmUInt64 sample = _read_frame_count % _current_output_buffer_size;

    _need_more_frames = true;

    for (AmUInt32 i = 0; i < frame->header.blocksize; i++)
    {
        for (AmUInt32 j = 0; j < frame->header.channels; j++)
        {
            auto& channel = _current_output_buffer->GetChannel(j);
            channel[sample + _current_output_buffer_offset] = AmInt16ToReal32(buffer[j][i]);
        }

        sample++;

        if (sample >= _current_output_buffer_size)
        {
            _need_more_frames = false;
            break;
        }
    }

    _read_frame_count = sample;

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacCodec::FlacDecoderInternal::metadata_callback(const ::FLAC__StreamMetadata* metadata)
{
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        auto total_samples = metadata->data.stream_info.total_samples;
        auto sample_rate = metadata->data.stream_info.sample_rate;
        auto channels = metadata->data.stream_info.channels;
        auto bps = metadata->data.stream_info.bits_per_sample;
        auto frame_size = metadata->data.stream_info.min_framesize;

        _decoder->m_format.SetAll(sample_rate, channels, bps, total_samples, channels * sizeof(AmAudioSample), AM_SAMPLE_FORMAT_FLOAT);
    }
}

void FlacCodec::FlacDecoderInternal::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
    amLogError("Got error callback: %s", FLAC__StreamDecoderErrorStatusString[status]);
}

::FLAC__StreamDecoderReadStatus FlacCodec::FlacDecoderInternal::read_callback(FLAC__byte buffer[], size_t* bytes)
{
    if (_decoder->_file == nullptr)
        return ::FLAC__STREAM_DECODER_READ_STATUS_ABORT;

    *bytes = _decoder->_file->Read(buffer, *bytes);
    return _decoder->_file->Eof() ? ::FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM : ::FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

::FLAC__StreamDecoderSeekStatus FlacCodec::FlacDecoderInternal::seek_callback(FLAC__uint64 absolute_byte_offset)
{
    if (_decoder->_file == nullptr)
        return ::FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;

    _decoder->_file->Seek(absolute_byte_offset, eFSO_START);
    return ::FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

::FLAC__StreamDecoderTellStatus FlacCodec::FlacDecoderInternal::tell_callback(FLAC__uint64* absolute_byte_offset)
{
    if (_decoder->_file == nullptr)
        return ::FLAC__STREAM_DECODER_TELL_STATUS_ERROR;

    *absolute_byte_offset = _decoder->_file->Position();
    return ::FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

::FLAC__StreamDecoderLengthStatus FlacCodec::FlacDecoderInternal::length_callback(FLAC__uint64* stream_length)
{
    if (_decoder->_file == nullptr)
        return ::FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;

    *stream_length = _decoder->_file->Length();
    return ::FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

bool FlacCodec::FlacDecoderInternal::eof_callback()
{
    return _decoder->_file != nullptr ? _decoder->_file->Eof() : true;
}

FlacCodec::FlacCodec()
    : Codec("flac")
{}

bool FlacCodec::FlacDecoder::Open(std::shared_ptr<File> file)
{
    _file = file;

    if (const FLAC__StreamDecoderInitStatus init_status = _flac.init(); init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        _file.reset();
        amLogError("Initializing FLAC decoder: %s", FLAC__StreamDecoderInitStatusString[init_status]);
        return false;
    }

    _flac.set_md5_checking(true);
    _flac.set_metadata_respond(::FLAC__METADATA_TYPE_STREAMINFO);

    if (!_flac.process_until_end_of_metadata())
    {
        _file.reset();
        amLogError("Unable to read metadata for FLAC file: " AM_OS_CHAR_FMT, file->GetPath().c_str());
        return false;
    }

    _initialized = true;

    return true;
}

bool FlacCodec::FlacDecoder::Close()
{
    if (_initialized)
    {
        _flac.finish();
        _file.reset();

        m_format = SoundFormat();
        _initialized = false;
    }

    // true because it is already closed
    return true;
}

AmUInt64 FlacCodec::FlacDecoder::Load(AudioBuffer* out)
{
    if (!_initialized)
        return 0;

    _flac.set_current_output_buffer(out, m_format.GetFramesCount(), 0);

    if (!Seek(0))
        return 0;

    if (_flac.process_until_end_of_stream())
        return _flac.read_frame_count();

    return 0;
}

AmUInt64 FlacCodec::FlacDecoder::Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length)
{
    if (!_initialized)
        return 0;

    _flac.set_current_output_buffer(out, length, bufferOffset);

    bool seeked = Seek(seekOffset);

    while (_flac.need_more_frames() && _flac.get_state() != FLAC__STREAM_DECODER_END_OF_STREAM)
        _flac.process_single();

    AmUInt64 read = _flac.read_frame_count();
    _flac.reset_read_frame_count();

    return read;
}

bool FlacCodec::FlacDecoder::Seek(AmUInt64 offset)
{
    return _flac.seek_absolute(offset);
}

bool FlacCodec::FlacEncoder::Open(std::shared_ptr<File> file)
{
    _initialized = true;
    return false;
}

bool FlacCodec::FlacEncoder::Close()
{
    if (_initialized)
        return true;

    return true;
}

AmUInt64 FlacCodec::FlacEncoder::Write(AudioBuffer* in, AmUInt64 offset, AmUInt64 length)
{
    return 0;
}

Codec::Decoder* FlacCodec::CreateDecoder()
{
    return ampoolnew(MemoryPoolKind::Codec, FlacDecoder, this);
}

void FlacCodec::DestroyDecoder(Decoder* decoder)
{
    ampooldelete(MemoryPoolKind::Codec, FlacDecoder, (FlacDecoder*)decoder);
}

Codec::Encoder* FlacCodec::CreateEncoder()
{
    return ampoolnew(MemoryPoolKind::Codec, FlacEncoder, this);
}

void FlacCodec::DestroyEncoder(Encoder* encoder)
{
    ampooldelete(MemoryPoolKind::Codec, FlacEncoder, (FlacEncoder*)encoder);
}

bool FlacCodec::CanHandleFile(std::shared_ptr<File> file) const
{
    const auto& path = file->GetPath();
    return path.find(AM_OS_STRING(".flac")) != AmOsString::npos;
}
