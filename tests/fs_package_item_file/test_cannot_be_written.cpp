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

#include "PlatformTestCase.h"
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, fs_package_item_file, cannot_be_written)
    {
    public:
        void Run() override
        {
            auto platformFileSystem = CreatePlatformFileSystem();
            platformFileSystem->SetBasePath(platformFileSystem->Join({ GetPlatformAssetsBasePath(), AM_OS_STRING("..") }));

            PackageFileSystem fileSystem;
            fileSystem.SetPlatformFileSystem(platformFileSystem);
            fileSystem.SetBasePath(AM_OS_STRING("./assets_uncompressed.ampk"));

            fileSystem.StartOpenFileSystem();
            while (!fileSystem.TryFinalizeOpenFileSystem())
                Thread::Sleep(1);

            auto file = fileSystem.OpenFile(AM_OS_STRING("data/tests/file_read_test.txt"), eFileOpenMode_Read);

            AM_EXPECT(file->Write8('O') == 0);
            AM_EXPECT(file->Write8('K') == 0);
        }
    };

    AM_REGISTER_TEST(fs_package_item_file, cannot_be_written);
} // namespace SparkyStudios::Audio::Amplitude::Tests
