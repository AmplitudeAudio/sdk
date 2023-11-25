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

#ifndef SS_AMPLITUDE_AUDIO_WAV_CODEC_H
#define SS_AMPLITUDE_AUDIO_WAV_CODEC_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "dr_wav.h"

namespace SparkyStudios::Audio::Amplitude::Codecs
{
    [[maybe_unused]] static class WAVCodec final : public Codec
    {
    public:
        class WAVDecoder final : public Decoder
        {
        public:
            explicit WAVDecoder(const Codec* codec)
                : Decoder(codec)
                , _initialized(false)
                , _wav()
            {}

            bool Open(std::shared_ptr<File> file) override;

            bool Close() override;

            AmUInt64 Load(AmVoidPtr out) override;

            AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) override;

            bool Seek(AmUInt64 offset) override;

        private:
            std::shared_ptr<File> _file;
            bool _initialized;
            drwav _wav;
        };

        class WAVEncoder final : public Encoder
        {
        public:
            explicit WAVEncoder(const Codec* codec)
                : Encoder(codec)
                , _file(nullptr)
                , _initialized(false)
                , _isFormatSet(false)
                , _wav()
            {}

            bool Open(std::shared_ptr<File> file) override;

            void SetFormat(const SoundFormat& format) override;

            bool Close() override;

            AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) override;

        private:
            std::shared_ptr<File> _file;
            bool _initialized;
            bool _isFormatSet;
            drwav _wav;
        };

        WAVCodec();

        ~WAVCodec() override = default;

        [[nodiscard]] Decoder* CreateDecoder() override;

        void DestroyDecoder(Decoder* decoder) override;

        [[nodiscard]] Encoder* CreateEncoder() override;

        void DestroyEncoder(Encoder* encoder) override;

        [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override;

        drwav_allocation_callbacks m_allocationCallbacks;
    } wav_codec; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude::Codecs

#endif // SS_AMPLITUDE_AUDIO_WAV_CODEC_H
