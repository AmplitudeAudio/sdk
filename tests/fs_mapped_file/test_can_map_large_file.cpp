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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <cstdio>
#include <filesystem>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        // Creates a temporary file filled with a repeating byte pattern.
        // Returns the path to the created file.
        std::filesystem::path CreateTempFile(AmSize size, AmUInt8 fillByte)
        {
            auto path = std::filesystem::temp_directory_path() / "am_mapped_file_test.bin";

            AmFileHandle fp = nullptr;
            #if AM_PLATFORM_WIN
                    _wfopen_s(&fp, path.c_str(), AM_OS_STRING("wb"));
            #else
                    fp = fopen(path.c_str(), AM_OS_STRING("wb"));
            #endif

            if (fp == nullptr)
                return {};

            constexpr AmSize kChunkSize = 4096;
            AmUInt8 chunk[kChunkSize];
            std::memset(chunk, fillByte, kChunkSize);

            AmSize remaining = size;
            while (remaining > 0)
            {
                const AmSize toWrite = remaining < kChunkSize ? remaining : kChunkSize;
                fwrite(chunk, 1, toWrite, fp);
                remaining -= toWrite;
            }

            fclose(fp);
            return path;
        }

        void RemoveTempFile(const std::filesystem::path& path)
        {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    } // anonymous namespace

    AM_TEST_CASE(SimpleTestCase, fs_mapped_file, maps_large_file_on_desktop)
    {
    public:
        void Run() override
        {
            constexpr AmSize kFileSize = 2 * 1024 * 1024; // 2 MB
            constexpr AmUInt8 kFillByte = 0xAB;

            const auto tempPath = CreateTempFile(kFileSize, kFillByte);
            AM_EXPECT_NOT(tempPath.empty());

            MappedFile file;
            AM_EXPECT(file.Open(tempPath) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Length() == kFileSize);

#if AM_PLATFORM_WIN || AM_PLATFORM_LINUX || AM_PLATFORM_APPLE
            AM_EXPECT(file.IsMapped());
#else
            AM_EXPECT_NOT(file.IsMapped()); // Fallback on unsupported platforms
#endif

            AM_EXPECT(file.GetPtr() != nullptr);

            // Verify content via GetPtr()
            auto* ptr = static_cast<const AmUInt8*>(file.GetPtr());
            AM_EXPECT(ptr[0] == kFillByte);
            AM_EXPECT(ptr[kFileSize / 2] == kFillByte);
            AM_EXPECT(ptr[kFileSize - 1] == kFillByte);

            // Verify content via Read()
            file.Seek(0, eFileSeekOrigin_Start);
            AmUInt8 buf[4] = {};
            AM_EXPECT(file.Read(buf, 4) == 4);
            AM_EXPECT(buf[0] == kFillByte);
            AM_EXPECT(buf[1] == kFillByte);
            AM_EXPECT(buf[2] == kFillByte);
            AM_EXPECT(buf[3] == kFillByte);

            file.Close();
            AM_EXPECT_NOT(file.IsValid());
            AM_EXPECT_NOT(file.IsMapped());

            RemoveTempFile(tempPath);
        }
    };

    AM_TEST_CASE(SimpleTestCase, fs_mapped_file, small_file_uses_heap_fallback)
    {
    public:
        void Run() override
        {
            constexpr AmSize kFileSize = 512 * 1024; // 512 KB (below 1 MB threshold)
            constexpr AmUInt8 kFillByte = 0xCD;

            const auto tempPath = CreateTempFile(kFileSize, kFillByte);
            AM_EXPECT_NOT(tempPath.empty());

            MappedFile file;
            AM_EXPECT(file.Open(tempPath) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Length() == kFileSize);
            AM_EXPECT_NOT(file.IsMapped()); // Below threshold, always heap

            // Verify content
            auto* ptr = static_cast<const AmUInt8*>(file.GetPtr());
            AM_EXPECT(ptr[0] == kFillByte);
            AM_EXPECT(ptr[kFileSize - 1] == kFillByte);

            file.Close();
            RemoveTempFile(tempPath);
        }
    };

    AM_TEST_CASE(SimpleTestCase, fs_mapped_file, seek_and_read_large_file)
    {
    public:
        void Run() override
        {
            // Create a file with sequential byte values (modulo 256)
            constexpr AmSize kFileSize = 2 * 1024 * 1024; // 2 MB
            auto path = std::filesystem::temp_directory_path() / "am_mapped_file_seq_test.bin";

            AmFileHandle fp = nullptr;
            #if AM_PLATFORM_WIN
                    _wfopen_s(&fp, path.c_str(), AM_OS_STRING("wb"));
            #else
                    fp = fopen(path.c_str(), AM_OS_STRING("wb"));
            #endif
            AM_EXPECT(fp != nullptr);

            constexpr AmSize kChunkSize = 4096;
            AmUInt8 chunk[kChunkSize];
            AmSize written = 0;
            while (written < kFileSize)
            {
                const AmSize toWrite = (kFileSize - written) < kChunkSize ? (kFileSize - written) : kChunkSize;
                for (AmSize i = 0; i < toWrite; ++i)
                    chunk[i] = static_cast<AmUInt8>((written + i) & 0xFF);
                fwrite(chunk, 1, toWrite, fp);
                written += toWrite;
            }
            fclose(fp);

            MappedFile file;
            AM_EXPECT(file.Open(path) == eErrorCode_Success);

            // Read from start
            file.Seek(0, eFileSeekOrigin_Start);
            AM_EXPECT(file.Read8() == 0x00);
            AM_EXPECT(file.Read8() == 0x01);
            AM_EXPECT(file.Read8() == 0x02);

            // Seek to a known offset and verify
            file.Seek(256, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 256);
            AM_EXPECT(file.Read8() == 0x00); // 256 & 0xFF == 0

            // Seek from end
            file.Seek(-1, eFileSeekOrigin_End);
            AM_EXPECT(file.Position() == kFileSize - 1);
            AM_EXPECT(file.Read8() == static_cast<AmUInt8>((kFileSize - 1) & 0xFF));

            file.Close();

            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    };

    AM_TEST_CASE(SimpleTestCase, fs_mapped_file, write_returns_zero_on_mapped)
    {
    public:
        void Run() override
        {
            constexpr AmSize kFileSize = 2 * 1024 * 1024; // 2 MB
            constexpr AmUInt8 kFillByte = 0x42;

            const auto tempPath = CreateTempFile(kFileSize, kFillByte);
            AM_EXPECT_NOT(tempPath.empty());

            MappedFile file;
            AM_EXPECT(file.Open(tempPath) == eErrorCode_Success);

            const AmUInt8 data[] = { 0xFF, 0xFF };
            AM_EXPECT(file.Write(data, 2) == 0);

            // Content unchanged
            auto* ptr = static_cast<const AmUInt8*>(file.GetPtr());
            AM_EXPECT(ptr[0] == kFillByte);

            file.Close();
            RemoveTempFile(tempPath);
        }
    };

    AM_TEST_CASE(SimpleTestCase, fs_mapped_file, can_reopen_large_file)
    {
    public:
        void Run() override
        {
            constexpr AmSize kFileSize = 2 * 1024 * 1024;
            constexpr AmUInt8 kFillByte = 0xEF;

            const auto tempPath = CreateTempFile(kFileSize, kFillByte);
            AM_EXPECT_NOT(tempPath.empty());

            MappedFile file;

            // First open
            AM_EXPECT(file.Open(tempPath) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            file.Seek(1024, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 1024);

            // Second open should reset state
            AM_EXPECT(file.Open(tempPath) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Position() == 0);
            AM_EXPECT(file.Length() == kFileSize);

            file.Close();
            RemoveTempFile(tempPath);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, maps_large_file_on_desktop);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, small_file_uses_heap_fallback);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, seek_and_read_large_file);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, write_returns_zero_on_mapped);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, can_reopen_large_file);
} // namespace SparkyStudios::Audio::Amplitude::Tests
