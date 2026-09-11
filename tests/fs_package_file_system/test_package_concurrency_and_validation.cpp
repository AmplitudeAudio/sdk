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
    AM_TEST_CASE(ComponentTestCase, fs_package_file_system, concurrency_and_validation)
    {
    public:
        void Run() override
        {
            PackageFileSystem packageFS;
            packageFS.SetPlatformFileSystem(CreatePlatformFileSystem());
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
        }
    };

    AM_REGISTER_TEST(fs_package_file_system, concurrency_and_validation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
