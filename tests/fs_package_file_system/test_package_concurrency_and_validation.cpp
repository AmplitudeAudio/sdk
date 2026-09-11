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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "ComponentTestCase.h"
#include "PlatformTestCase.h"
#include "TestRegistry.h"

#include <filesystem>

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, fs_package_file_system, concurrency_and_validation)
    {
    public:
        void Run() override
        {
            auto platformFS = CreatePlatformFileSystem();

            PackageFileSystem packageFS;
            packageFS.SetPlatformFileSystem(platformFS);
            packageFS.SetBasePath(AM_OS_STRING("non_existent_package.ampk"));

            // Rapid start and finalize should safely handle missing packages
            packageFS.StartOpenFileSystem();
            while (!packageFS.TryFinalizeOpenFileSystem())
            {
                Thread::Sleep(1);
            }

            AM_EXPECT(!packageFS.IsValid());
            AM_EXPECT(!packageFS.Exists(AM_OS_STRING("some_item.txt")));
            AM_EXPECT(packageFS.OpenFile(AM_OS_STRING("some_item.txt"), eFileOpenMode_Read) == nullptr);

            // Reopening non-existent package should also finalize cleanly and remain invalid
            packageFS.StartOpenFileSystem();
            while (!packageFS.TryFinalizeOpenFileSystem())
            {
                Thread::Sleep(1);
            }

            AM_EXPECT(!packageFS.IsValid());

            // Validate out-of-bounds item detection
            const AmOsString badItemPackage = AM_OS_STRING("./test_item_oob.ampk");
            {
                auto file = platformFS->OpenFile(badItemPackage, eFileOpenMode_Write);
                if (file != nullptr)
                {
                    const AmUInt8 tag[4] = { 'A', 'M', 'P', 'K' };
                    file->Write(tag, 4);
                    file->Write16(1);
                    file->Write8(ePackageFileCompressionMode_Uncompressed);
                    file->Write64(1); // items count
                    file->WriteString("bad_item.txt");
                    file->Write64(500000); // offset (out of bounds)
                    file->Write64(100); // size
                    file->Write64(0); // compressed block size
                    file->Write64(0); // chunks count
                    file->Close();

                    PackageFileSystem badItemFS;
                    badItemFS.SetPlatformFileSystem(platformFS);
                    badItemFS.SetBasePath(badItemPackage);
                    badItemFS.StartOpenFileSystem();
                    while (!badItemFS.TryFinalizeOpenFileSystem())
                    {
                        Thread::Sleep(1);
                    }

                    AM_EXPECT(!badItemFS.IsValid());
                    AM_EXPECT(!badItemFS.Exists(AM_OS_STRING("bad_item.txt")));
                    AM_EXPECT(badItemFS.OpenFile(AM_OS_STRING("bad_item.txt"), eFileOpenMode_Read) == nullptr);

                    std::filesystem::remove(std::filesystem::path(badItemPackage));
                }
            }

            // Validate out-of-bounds chunk detection
            const AmOsString badChunkPackage = AM_OS_STRING("./test_chunk_oob.ampk");
            {
                auto file = platformFS->OpenFile(badChunkPackage, eFileOpenMode_Write);
                if (file != nullptr)
                {
                    const AmUInt8 tag[4] = { 'A', 'M', 'P', 'K' };
                    file->Write(tag, 4);
                    file->Write16(1);
                    file->Write8(ePackageFileCompressionMode_Compressed);
                    file->Write64(1); // items count
                    file->WriteString("bad_chunk_item.txt");
                    file->Write64(0); // item offset
                    file->Write64(10); // item size
                    file->Write64(10); // compressed block size
                    file->Write64(1); // chunks count
                    file->Write64(500000); // chunk offset (out of bounds)
                    file->Write64(10); // chunk size
                    file->Write64(10); // chunk compressed size
                    file->Close();

                    PackageFileSystem badChunkFS;
                    badChunkFS.SetPlatformFileSystem(platformFS);
                    badChunkFS.SetBasePath(badChunkPackage);
                    badChunkFS.StartOpenFileSystem();
                    while (!badChunkFS.TryFinalizeOpenFileSystem())
                    {
                        Thread::Sleep(1);
                    }

                    AM_EXPECT(!badChunkFS.IsValid());
                    AM_EXPECT(!badChunkFS.Exists(AM_OS_STRING("bad_chunk_item.txt")));
                    AM_EXPECT(badChunkFS.OpenFile(AM_OS_STRING("bad_chunk_item.txt"), eFileOpenMode_Read) == nullptr);

                    std::filesystem::remove(std::filesystem::path(badChunkPackage));
                }
            }
        }
    };

    AM_REGISTER_TEST(fs_package_file_system, concurrency_and_validation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
