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

#ifndef SS_AMPLITUDE_AUDIO_AMS_CODEC_H
#define SS_AMPLITUDE_AUDIO_AMS_CODEC_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Utils/Audio/Compression/ADPCM/ADPCM.h>

namespace SparkyStudios::Audio::Amplitude::Codecs
{
    [[maybe_unused]] static class AMSCodec final : public Codec
    {
    public:
        class AMSDecoder final : public Decoder
        {
        public:
            explicit AMSDecoder(const Codec* codec)
                : Decoder(codec)
                , _initialized(false)
                , _file()
                , _blockSize(0)
            {}

            bool Open(std::shared_ptr<File> file) override;

            bool Close() override;

            AmUInt64 Load(AmVoidPtr out) override;

            AmUInt64 Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length) override;

            bool Seek(AmUInt64 offset) override;

        private:
            bool _initialized;
            std::shared_ptr<File> _file;
            AmUInt16 _blockSize;
        };

        class AMSEncoder final : public Encoder
        {
        public:
            explicit AMSEncoder(const Codec* codec)
                : Encoder(codec)
                , _initialized(false)
                , _file()
                , _blockSize(2048)
                , _samplesPerBlock(2041)
                , _lookAhead(3)
                , _noiseShaping(Compression::ADPCM::eNSM_OFF)
            {}

            bool Open(std::shared_ptr<File> file) override;

            bool Close() override;

            AmUInt64 Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length) override;

            void SetEncodingParams(
                AmUInt32 blockSize, AmUInt32 samplesPerBlock, AmUInt32 lookAhead, Compression::ADPCM::NoiseShapingMode noiseShaping);

        private:
            bool _initialized;
            std::shared_ptr<File> _file;
            AmUInt32 _blockSize;
            AmUInt32 _samplesPerBlock;
            AmUInt32 _lookAhead;
            Compression::ADPCM::NoiseShapingMode _noiseShaping;
        };

        AMSCodec();

        ~AMSCodec() override = default;

        [[nodiscard]] Decoder* CreateDecoder() override;

        void DestroyDecoder(Decoder* decoder) override;

        [[nodiscard]] Encoder* CreateEncoder() override;

        void DestroyEncoder(Encoder* encoder) override;

        [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override;
    } ams_codec; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude::Codecs

#endif // SS_AMPLITUDE_AUDIO_AMS_CODEC_H
