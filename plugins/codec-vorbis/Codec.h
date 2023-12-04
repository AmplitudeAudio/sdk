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

#include <vorbis/vorbisfile.h>

class VorbisCodec final : public Codec
{
public:
    class VorbisDecoder final : public Decoder
    {
    public:
        explicit VorbisDecoder(const Codec* codec)
            : Decoder(codec)
            , _initialized(false)
            , _vorbis()
            , _file(nullptr)
            , _current_section(0)
        {}

        bool Open(std::shared_ptr<File> file) override;

        bool Close() override;

        AmUInt64 Load(AmVoidPtr out) override;

        AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) override;

        bool Seek(AmUInt64 offset) override;

    private:
        bool _initialized;

        OggVorbis_File _vorbis;
        std::shared_ptr<File> _file;
        AmInt32 _current_section;
    };

    class VorbisEncoder final : public Encoder
    {
    public:
        explicit VorbisEncoder(const Codec* codec)
            : Encoder(codec)
            , _initialized(false)
        {}

        bool Open(std::shared_ptr<File> file) override;

        bool Close() override;

        AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) override;

    private:
        bool _initialized;
    };

    VorbisCodec()
        : Codec("vorbis")
    {}

    ~VorbisCodec() override = default;

    [[nodiscard]] Decoder* CreateDecoder() override;

    void DestroyDecoder(Decoder* decoder) override;

    [[nodiscard]] Encoder* CreateEncoder() override;

    void DestroyEncoder(Encoder* encoder) override;

    [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override;
};
#endif // SS_AMPLITUDE_AUDIO_VORBIS_CODEC_CODEC_H
