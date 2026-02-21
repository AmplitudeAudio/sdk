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
    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, can_be_opened_with_small_file)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Length() == 2);
            AM_EXPECT(file.GetPtr() != nullptr);
            AM_EXPECT_NOT(file.IsMapped()); // Below 1 MB threshold, should use heap fallback

            file.Close();
            AM_EXPECT_NOT(file.IsValid());
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, can_be_opened_with_constructor)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file(resolvedPath);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Length() == 2);
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, rejects_empty_path)
    {
    public:
        void Run() override
        {
            MappedFile file;
            AM_EXPECT(file.Open("") == eErrorCode_InvalidParameter);
            AM_EXPECT_NOT(file.IsValid());
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, rejects_nonexistent_file)
    {
    public:
        void Run() override
        {
            MappedFile file;
            AM_EXPECT(file.Open("/nonexistent/path/to/file.bin") == eErrorCode_FileNotFound);
            AM_EXPECT_NOT(file.IsValid());
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, can_reopen_after_close)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Length() == 2);

            file.Close();
            AM_EXPECT_NOT(file.IsValid());

            // Reopen the same file
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Length() == 2);
        }
    };

    AM_TEST_CASE(ComponentTestCase, fs_mapped_file, open_cleans_previous_state)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);

            // Advance the read cursor
            file.Seek(1, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 1);

            // Opening again should reset state
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);
            AM_EXPECT(file.Position() == 0);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, can_be_opened_with_small_file);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, can_be_opened_with_constructor);
    AM_REGISTER_TEST(fs_mapped_file, rejects_empty_path);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, rejects_nonexistent_file);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, can_reopen_after_close);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, open_cleans_previous_state);
} // namespace SparkyStudios::Audio::Amplitude::Tests
