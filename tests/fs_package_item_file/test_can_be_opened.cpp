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
    AM_TEST_CASE(ComponentTestCase, fs_package_item_file, can_be_opened)
    {
    public:
        void Run() override
        {
            auto platformFileSystem = CreatePlatformFileSystem();
            platformFileSystem->SetBasePath(platformFileSystem->Join({ GetPlatformAssetsBasePath(), AM_OS_STRING("..") }));

            // Uncompressed file
            {
                PackageFileSystem fileSystem;
                fileSystem.SetPlatformFileSystem(platformFileSystem);
                fileSystem.SetBasePath(AM_OS_STRING("./assets_uncompressed.ampk"));

                fileSystem.StartOpenFileSystem();
                while (!fileSystem.TryFinalizeOpenFileSystem())
                    Thread::Sleep(1);

                auto file = fileSystem.OpenFile(AM_OS_STRING("data/tests/file_read_test.txt"), eFileOpenMode_Read);

                AM_EXPECT(file->IsValid());
                AM_EXPECT(file->GetPath() == fileSystem.ResolvePath(AM_OS_STRING("data/tests/file_read_test.txt")));
                AM_EXPECT(file->Length() == 2);

                AM_EXPECT(fileSystem.OpenFile(AM_OS_STRING("some_random_file.ext")) == nullptr);
            }

            // Compressed file
            {
                PackageFileSystem fileSystem;
                fileSystem.SetPlatformFileSystem(platformFileSystem);
                fileSystem.SetBasePath(AM_OS_STRING("./assets_compressed.ampk"));

                fileSystem.StartOpenFileSystem();
                while (!fileSystem.TryFinalizeOpenFileSystem())
                    Thread::Sleep(1);

                auto file = fileSystem.OpenFile(AM_OS_STRING("data/tests/file_read_test.txt"), eFileOpenMode_Read);

                AM_EXPECT(file->IsValid());
                AM_EXPECT(file->GetPath() == fileSystem.ResolvePath(AM_OS_STRING("data/tests/file_read_test.txt")));
                AM_EXPECT(file->Length() == 2);

                AM_EXPECT(fileSystem.OpenFile(AM_OS_STRING("some_random_file.ext")) == nullptr);
            }
        }
    };

    AM_REGISTER_TEST(fs_package_item_file, can_be_opened);
} // namespace SparkyStudios::Audio::Amplitude::Tests
