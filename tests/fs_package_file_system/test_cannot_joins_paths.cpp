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
#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, fs_package_file_system, cannot_joins_paths)
    {
    public:
        void Run() override
        {
            PackageFileSystem fileSystem;
            fileSystem.SetPlatformFileSystem(CreatePlatformFileSystem());

            AM_EXPECT(fileSystem.Join({ AM_OS_STRING("sounds"), AM_OS_STRING("test.wav") }) == AM_OS_STRING("sounds/test.wav"));
            AM_EXPECT(
                fileSystem.Join({ AM_OS_STRING("../sample_project/sounds/../test.wav") }) == AM_OS_STRING("../sample_project/test.wav"));
            AM_EXPECT(
                fileSystem.Join({ AM_OS_STRING("./sounds"), AM_OS_STRING("../sounds/"), AM_OS_STRING("./test.wav") }) ==
                AM_OS_STRING("sounds/test.wav"));
        }
    };

    AM_REGISTER_TEST(fs_package_file_system, cannot_joins_paths);
} // namespace SparkyStudios::Audio::Amplitude::Tests
