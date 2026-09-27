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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, fs_package_file_system, fails_cleanly_without_platform_file_system)
    {
    public:
        void Run() override
        {
            // No SetPlatformFileSystem(): opening must finish as an invalid file system, not crash.
            PackageFileSystem packageFS;
            packageFS.SetBasePath(AM_OS_STRING("./assets_compressed.ampk"));

            packageFS.StartOpenFileSystem();
            while (!packageFS.TryFinalizeOpenFileSystem())
                Thread::Sleep(1);

            AM_EXPECT_NOT(packageFS.IsValid());
            AM_EXPECT_NOT(packageFS.Exists(AM_OS_STRING("some_item.txt")));
            AM_EXPECT(packageFS.OpenFile(AM_OS_STRING("some_item.txt"), eFileOpenMode_Read) == nullptr);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_package_file_system, fails_cleanly_without_platform_file_system);

    AM_TEST_CASE(ComponentTestCase, fs_package_file_system, platform_file_system_template_is_released_safely)
    {
    public:
        void Run() override
        {
            // The templated setter allocates the platform file system itself; destroying the package
            // file system must release it through the same memory pool.
            {
                PackageFileSystem packageFS;
                packageFS.SetPlatformFileSystem<DiskFileSystem>();
                packageFS.SetBasePath(AM_OS_STRING("non_existent_package.ampk"));
                AM_EXPECT_NOT(packageFS.GetBasePath().empty());
            }

            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_package_file_system, platform_file_system_template_is_released_safely);
} // namespace SparkyStudios::Audio::Amplitude::Tests
