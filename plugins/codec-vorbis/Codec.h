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

#ifndef SS_AMPLITUDE_AUDIO_VORBIS_CODEC_CODEC_H
#define SS_AMPLITUDE_AUDIO_VORBIS_CODEC_CODEC_H

#include <Plugin.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

using namespace SparkyStudios::Audio::Amplitude;

class VorbisCodec final : public Codec
{
public:
    class VorbisDecoder final : public Codec::Decoder
    {
    public:
        explicit VorbisDecoder(const Codec* codec)
            : Codec::Decoder(codec)
            , _initialized(false)
            , _file()
            , _vorbis()
            , _current_section(0)
        {}

        bool Open(const AmOsString& filePath) final;

        bool Close() final;

        AmUInt64 Load(AmVoidPtr out) final;

        AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) final;

        bool Seek(AmUInt64 offset) final;

    private:
        bool _initialized;

        OggVorbis_File _vorbis;
        DiskFile _file;
        AmInt32 _current_section;
    };

    class VorbisEncoder final : public Codec::Encoder
    {
    public:
        explicit VorbisEncoder(const Codec* codec)
            : Codec::Encoder(codec)
            , _initialized(false)
        {}

        bool Open(const AmOsString& filePath) final;

        bool Close() final;

        AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) final;

    private:
        bool _initialized;
    };

    VorbisCodec()
        : Codec("vorbis")
    {}

    ~VorbisCodec() final = default;

    [[nodiscard]] Decoder* CreateDecoder() const final;

    [[nodiscard]] Encoder* CreateEncoder() const final;

    [[nodiscard]] bool CanHandleFile(const AmOsString& filePath) const final;
};
#endif // SS_AMPLITUDE_AUDIO_VORBIS_CODEC_CODEC_H
