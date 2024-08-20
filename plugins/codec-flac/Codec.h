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

#pragma once

#ifndef _AM_PLUGIN_CODEC_FLAC_CODEC_H
#define _AM_PLUGIN_CODEC_FLAC_CODEC_H

#include <Plugin.h>

#include <FLAC++/decoder.h>

class FlacCodec final : public Codec
{
public:
    class FlacDecoder;
    class FlacDecoderInternal;

    class FlacDecoderInternal final : public FLAC::Decoder::Stream
    {
    public:
        explicit FlacDecoderInternal(FlacDecoder* decoder)
            : FLAC::Decoder::Stream()
            , _decoder(decoder)
            , _need_more_frames(false)
            , _current_output_buffer(nullptr)
            , _read_frame_count(0)
            , _current_output_buffer_size(0)
        {}

        FlacDecoderInternal(const FlacDecoderInternal&) = delete;
        FlacDecoderInternal& operator=(const FlacDecoderInternal&) = delete;

        void set_current_output_buffer(AudioBuffer* output, AmUInt64 size, AmUInt64 offset);

        [[nodiscard]] bool need_more_frames() const
        {
            return _need_more_frames;
        }

        [[nodiscard]] AmUInt64 read_frame_count() const
        {
            return _read_frame_count;
        }

        void reset_read_frame_count()
        {
            _read_frame_count = 0;
        }

    protected:
        ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame* frame, const FLAC__int32* const buffer[]) override;
        void metadata_callback(const ::FLAC__StreamMetadata* metadata) override;
        void error_callback(::FLAC__StreamDecoderErrorStatus status) override;
        ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t* bytes) override;
        ::FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset) override;
        ::FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64* absolute_byte_offset) override;
        ::FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64* stream_length) override;
        bool eof_callback() override;

    private:
        FlacDecoder* _decoder;

        bool _need_more_frames;
        AmUInt64 _read_frame_count;

        AudioBuffer* _current_output_buffer;
        AmUInt64 _current_output_buffer_size;
        AmUInt64 _current_output_buffer_offset;
    };

    class FlacDecoder final : public Codec::Decoder
    {
    public:
        explicit FlacDecoder(const Codec* codec)
            : Codec::Decoder(codec)
            , _initialized(false)
            , _flac(this)
        {}

        bool Open(std::shared_ptr<File> file) override;

        bool Close() override;

        AmUInt64 Load(AudioBuffer* out) override;

        AmUInt64 Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length) override;

        bool Seek(AmUInt64 offset) override;

    private:
        friend class FlacDecoderInternal;

        bool _initialized;

        std::shared_ptr<File> _file;
        FlacDecoderInternal _flac;
    };

    class FlacEncoder final : public Codec::Encoder
    {
    public:
        explicit FlacEncoder(const Codec* codec)
            : Codec::Encoder(codec)
            , _initialized(false)
        {}

        bool Open(std::shared_ptr<File> file) override;

        bool Close() override;

        AmUInt64 Write(AudioBuffer* in, AmUInt64 offset, AmUInt64 length) override;

    private:
        bool _initialized;
    };

    FlacCodec();

    ~FlacCodec() override = default;

    [[nodiscard]] Decoder* CreateDecoder() override;

    void DestroyDecoder(Decoder* decoder) override;

    [[nodiscard]] Encoder* CreateEncoder() override;

    void DestroyEncoder(Encoder* encoder) override;

    [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override;
};
#endif // _AM_PLUGIN_CODEC_FLAC_CODEC_H
