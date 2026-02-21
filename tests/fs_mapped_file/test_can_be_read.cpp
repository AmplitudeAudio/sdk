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

#include "PlatformTestCase.h"
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, can_read_data)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);

            char buf[2] = {};
            AM_EXPECT(file.Read(reinterpret_cast<AmUInt8Buffer>(buf), 2) == 2);
            AM_EXPECT(buf[0] == 'O');
            AM_EXPECT(buf[1] == 'K');
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, can_read_with_seek)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);

            // Seek from start
            file.Seek(1, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 1);
            AM_EXPECT(file.Read8() == 'K');

            // Seek from end
            file.Seek(-2, eFileSeekOrigin_End);
            AM_EXPECT(file.Position() == 0);
            AM_EXPECT(file.Read8() == 'O');

            // Seek from current (back one byte)
            file.Seek(-1, eFileSeekOrigin_Current);
            AM_EXPECT(file.Position() == 0);
            AM_EXPECT(file.Read8() == 'O');

            // Seek past end clamps
            file.Seek(1234, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 2);
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, reports_eof)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);

            AM_EXPECT_NOT(file.Eof());

            // Read all data
            char buf[2] = {};
            file.Read(reinterpret_cast<AmUInt8Buffer>(buf), 2);
            AM_EXPECT(file.Position() == file.Length());
            AM_EXPECT(file.Eof());
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, read_clamps_at_eof)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);

            // Request more bytes than available
            char buf[16] = {};
            const AmSize bytesRead = file.Read(reinterpret_cast<AmUInt8Buffer>(buf), 16);
            AM_EXPECT(bytesRead == 2);
            AM_EXPECT(buf[0] == 'O');
            AM_EXPECT(buf[1] == 'K');
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, getptr_returns_buffer)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);

            auto* ptr = static_cast<const AmUInt8*>(file.GetPtr());
            AM_EXPECT(ptr != nullptr);
            AM_EXPECT(ptr[0] == 'O');
            AM_EXPECT(ptr[1] == 'K');
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, read_on_invalid_file_returns_zero)
    {
    public:
        void Run() override
        {
            MappedFile file;
            AM_EXPECT_NOT(file.IsValid());

            char buf[4] = {};
            AM_EXPECT(file.Read(reinterpret_cast<AmUInt8Buffer>(buf), 4) == 0);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, can_read_data);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, can_read_with_seek);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, reports_eof);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, read_clamps_at_eof);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, getptr_returns_buffer);
    AM_REGISTER_TEST(fs_mapped_file, read_on_invalid_file_returns_zero);
} // namespace SparkyStudios::Audio::Amplitude::Tests
