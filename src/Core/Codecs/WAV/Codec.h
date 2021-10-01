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
        class WAVDecoder final : public Codec::Decoder
        {
        public:
            explicit WAVDecoder(const Codec* codec)
                : Codec::Decoder(codec)
                , _initialized(false)
                , _wav()
            {}

            bool Open(AmOsString filePath) final;

            bool Close() final;

            AmUInt64 Load(AmVoidPtr out) final;

            AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) final;

            bool Seek(AmUInt64 offset) final;

        private:
            bool _initialized;
            drwav _wav;
        };

        class WAVEncoder final : public Codec::Encoder
        {
        public:
            explicit WAVEncoder(const Codec* codec)
                : Codec::Encoder(codec)
                , _initialized(false)
                , _wav()
            {}

            bool Open(AmOsString filePath) final;

            bool Close() final;

            AmUInt64 Write(AudioBuffer in, AmUInt64 offset, AmUInt64 length) final;

        private:
            bool _initialized;
            drwav _wav;
        };

        WAVCodec()
            : Codec("wav")
        {}

        ~WAVCodec() final = default;

        [[nodiscard]] Decoder* CreateDecoder() const final;

        [[nodiscard]] Encoder* CreateEncoder() const final;

        bool CanHandleFile(AmOsString filePath) const final;
    } wav_codec; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude::Codecs

#endif // SS_AMPLITUDE_AUDIO_WAV_CODEC_H
