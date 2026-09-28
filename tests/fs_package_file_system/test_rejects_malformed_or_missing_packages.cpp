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

#include "ComponentTestCase.h"
#include "PlatformTestCase.h"
#include "TestRegistry.h"

#include <filesystem>

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        void WritePackageHeader(const std::shared_ptr<File>& file, AmUInt64 itemsCount)
        {
            const AmUInt8 tag[4] = { 'A', 'M', 'P', 'K' };
            file->Write(tag, 4);
            file->Write16(1);
            file->Write8(ePackageFileCompressionMode_Uncompressed);
            file->Write64(itemsCount);
        }

        void OpenPackage(PackageFileSystem& packageFS, const std::shared_ptr<FileSystem>& platformFS, const AmOsString& path)
        {
            packageFS.SetPlatformFileSystem(platformFS);
            packageFS.SetBasePath(path);
            packageFS.StartOpenFileSystem();
            while (!packageFS.TryFinalizeOpenFileSystem())
                Thread::Sleep(1);
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, fs_package_file_system, rejects_counts_beyond_package_size)
    {
    public:
        void Run() override
        {
            auto platformFS = CreatePlatformFileSystem();

            // Five items declared, only one written: the package must be rejected, not padded with garbage items.
            const AmOsString path = AM_OS_STRING("./test_truncated_items.ampk");
            {
                auto file = platformFS->OpenFile(path, eFileOpenMode_Write);
                AM_EXPECT(file != nullptr);

                WritePackageHeader(file, 5);
                file->WriteString("item.txt");
                file->Write64(0); // offset
                file->Write64(0); // size
                file->Write64(0); // compressed block size
                file->Write64(0); // chunks count
                file->Close();
            }

            {
                PackageFileSystem packageFS;
                OpenPackage(packageFS, platformFS, path);
                AM_EXPECT_NOT(packageFS.IsValid());
            }

            // An item count above the sanity limit must be rejected, not clamped.
            {
                auto file = platformFS->OpenFile(path, eFileOpenMode_Write);
                AM_EXPECT(file != nullptr);

                WritePackageHeader(file, 0xFFFFFFFFFFFFull);
                file->Close();
            }

            {
                PackageFileSystem packageFS;
                OpenPackage(packageFS, platformFS, path);
                AM_EXPECT_NOT(packageFS.IsValid());
            }

            std::filesystem::remove(std::filesystem::path(path));
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_package_file_system, rejects_counts_beyond_package_size);

    AM_TEST_CASE(ComponentTestCase, fs_package_file_system, open_file_fails_when_package_is_gone)
    {
    public:
        void Run() override
        {
            auto platformFS = CreatePlatformFileSystem();

            const AmOsString path = AM_OS_STRING("./test_vanishing_package.ampk");
            {
                auto file = platformFS->OpenFile(path, eFileOpenMode_Write);
                AM_EXPECT(file != nullptr);

                WritePackageHeader(file, 1);
                file->WriteString("item.txt");
                file->Write64(0); // offset
                file->Write64(0); // size
                file->Write64(0); // compressed block size
                file->Write64(0); // chunks count
                file->Close();
            }

            PackageFileSystem packageFS;
            OpenPackage(packageFS, platformFS, path);
            AM_EXPECT(packageFS.IsValid());

            // Each item reopens the package: once it is gone, OpenFile must fail instead of returning an unusable item.
            std::filesystem::remove(std::filesystem::path(path));
            AM_EXPECT(packageFS.OpenFile(AM_OS_STRING("item.txt"), eFileOpenMode_Read) == nullptr);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_package_file_system, open_file_fails_when_package_is_gone);
} // namespace SparkyStudios::Audio::Amplitude::Tests
