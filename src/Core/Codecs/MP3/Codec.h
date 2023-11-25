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

#ifndef SS_AMPLITUDE_AUDIO_MP3_CODEC_H
#define SS_AMPLITUDE_AUDIO_MP3_CODEC_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "dr_mp3.h"

namespace SparkyStudios::Audio::Amplitude::Codecs
{
    [[maybe_unused]] static class MP3Codec final : public Codec
    {
    public:
        class MP3Decoder final : public Decoder
        {
        public:
            explicit MP3Decoder(const Codec* codec)
                : Decoder(codec)
                , _initialized(false)
                , _mp3()
            {}

            bool Open(std::shared_ptr<File> file) override;

            bool Close() override;

            AmUInt64 Load(AmVoidPtr out) override;

            AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) override;

            bool Seek(AmUInt64 offset) override;

        private:
            std::shared_ptr<File> _file;
            bool _initialized;
            drmp3 _mp3;
        };

        class MP3Encoder final : public Encoder
        {
        public:
            explicit MP3Encoder(const Codec* codec)
                : Encoder(codec)
                , _initialized(false)
                , _mp3()
            {}

            bool Open(std::shared_ptr<File> file) override;

            bool Close() override;

            AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) override;

        private:
            bool _initialized;
            drmp3 _mp3;
        };

        MP3Codec();

        ~MP3Codec() override = default;

        [[nodiscard]] Decoder* CreateDecoder() override;

        void DestroyDecoder(Decoder* decoder) override;

        [[nodiscard]] Encoder* CreateEncoder() override;

        void DestroyEncoder(Encoder* encoder) override;

        [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override;

        drmp3_allocation_callbacks m_allocationCallbacks;
    } mp3_codec; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude::Codecs

#endif // SS_AMPLITUDE_AUDIO_MP3_CODEC_H
