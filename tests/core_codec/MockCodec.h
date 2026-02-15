// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_TESTS_CORE_CODEC_MOCK_CODEC_H
#define _AM_TESTS_CORE_CODEC_MOCK_CODEC_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class MockDecoder final : public Codec::Decoder
    {
    public:
        explicit MockDecoder(const Codec* codec)
            : Decoder(codec)
            , _file(nullptr)
            , _isOpen(false)
        {}

        bool Open(std::shared_ptr<File> file) override
        {
            if (!file)
                return false;

            _file = std::move(file);
            _isOpen = true;
            m_format.SetAll(44100, 2, 16, 1000, 4, eAudioSampleFormat_Float32);
            return true;
        }

        bool Close() override
        {
            _file.reset();
            _isOpen = false;
            m_format = SoundFormat();
            return true;
        }

        AmUInt64 Load(AudioBuffer* out) override
        {
            if (!_isOpen || !out)
                return 0;

            return Stream(out, 0, 0, m_format.GetFramesCount());
        }

        AmUInt64 Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length) override
        {
            if (!_isOpen || !out)
                return 0;

            if (seekOffset > m_format.GetFramesCount())
                return 0;

            const AmUInt64 availableFrames = std::min(length, m_format.GetFramesCount() - seekOffset);
            return availableFrames;
        }

        bool Seek(AmUInt64 offset) override
        {
            return _isOpen && offset <= m_format.GetFramesCount();
        }

        [[nodiscard]] bool IsOpen() const
        {
            return _isOpen;
        }

    private:
        std::shared_ptr<File> _file;
        bool _isOpen;
    };

    class MockEncoder final : public Codec::Encoder
    {
    public:
        explicit MockEncoder(const Codec* codec)
            : Encoder(codec)
            , _file(nullptr)
            , _isOpen(false)
        {}

        bool Open(std::shared_ptr<File> file) override
        {
            if (!file)
                return false;

            _file = std::move(file);
            _isOpen = true;
            return true;
        }

        bool Close() override
        {
            _file.reset();
            _isOpen = false;
            return true;
        }

        AmUInt64 Write(AudioBuffer* in, AmUInt64 offset, AmUInt64 length) override
        {
            if (!_isOpen || !in)
                return 0;

            return length;
        }

        [[nodiscard]] bool IsOpen() const
        {
            return _isOpen;
        }

    private:
        std::shared_ptr<File> _file;
        bool _isOpen;
    };

    class MockCodec final : public Codec
    {
    public:
        explicit MockCodec(const AmString& name = "mock")
            : Codec(name)
            , _canHandleFileResult(true)
        {}

        ~MockCodec() override = default;

        [[nodiscard]] std::shared_ptr<Decoder> CreateDecoder() override
        {
            return ampoolshared(eMemoryPoolKind_Codec, MockDecoder, this);
        }

        [[nodiscard]] std::shared_ptr<Encoder> CreateEncoder() override
        {
            return ampoolshared(eMemoryPoolKind_Codec, MockEncoder, this);
        }

        [[nodiscard]] bool CanHandleFile(std::shared_ptr<File> file) const override
        {
            if (!file)
                return false;

            return _canHandleFileResult;
        }

        void SetCanHandleFileResult(bool result)
        {
            _canHandleFileResult = result;
        }

    private:
        bool _canHandleFileResult;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_CORE_CODEC_MOCK_CODEC_H
