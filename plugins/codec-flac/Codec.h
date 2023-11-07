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

#ifndef SS_AMPLITUDE_AUDIO_FLAC_CODEC_CODEC_H
#define SS_AMPLITUDE_AUDIO_FLAC_CODEC_CODEC_H

#include <Plugin.h>

#include <FLAC++/decoder.h>

using namespace SparkyStudios::Audio::Amplitude;

class FlacCodec final : public Codec
{
public:
    class FlacDecoder;
    class FlacDecoderInternal;

    class FlacDecoderInternal : public FLAC::Decoder::File
    {
    public:
        FlacDecoderInternal(FlacDecoder* decoder)
            : FLAC::Decoder::File()
            , _decoder(decoder)
            , _need_more_frames(false)
            , _current_output_buffer(nullptr)
        {}

        void set_current_output_buffer(AmAudioSampleBuffer output, AmUInt64 size);

        [[nodiscard]] bool need_more_frames() const { return _need_more_frames; }
        [[nodiscard]] AmUInt64 read_frame_count() const { return _read_frame_count; }

    protected:
        ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame* frame, const FLAC__int32* const buffer[]) override;
        void metadata_callback(const ::FLAC__StreamMetadata* metadata) override;
        void error_callback(::FLAC__StreamDecoderErrorStatus status) override;

    private:
        FlacDecoderInternal(const FlacDecoderInternal&);
        FlacDecoderInternal& operator=(const FlacDecoderInternal&);

        FlacDecoder* _decoder;

        bool _need_more_frames;
        AmUInt64 _read_frame_count;

        AmAudioSampleBuffer _current_output_buffer;
        AmUInt64 _current_output_buffer_size;
    };

    class FlacDecoder final : public Codec::Decoder
    {
    public:
        explicit FlacDecoder(const Codec* codec)
            : Codec::Decoder(codec)
            , _initialized(false)
            , _flac(this)
        {}

        bool Open(const AmOsString& filePath) final;

        bool Close() final;

        AmUInt64 Load(AmVoidPtr out) final;

        AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) final;

        bool Seek(AmUInt64 offset) final;

    private:
        friend class FlacDecoderInternal;

        bool _initialized;

        FlacDecoderInternal _flac;
    };

    class FlacEncoder final : public Codec::Encoder
    {
    public:
        explicit FlacEncoder(const Codec* codec)
            : Codec::Encoder(codec)
            , _initialized(false)
        {}

        bool Open(const AmOsString& filePath) final;

        bool Close() final;

        AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) final;

    private:
        bool _initialized;
    };

    FlacCodec();

    ~FlacCodec() final = default;

    [[nodiscard]] Decoder* CreateDecoder() const final;

    [[nodiscard]] Encoder* CreateEncoder() const final;

    [[nodiscard]] bool CanHandleFile(const AmOsString& filePath) const final;
};
#endif // SS_AMPLITUDE_AUDIO_FLAC_CODEC_CODEC_H
