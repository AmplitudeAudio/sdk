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
    AM_TEST_CASE(SimpleTestCase, fs_disk_file, can_be_closed)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto& file = fileSystem->OpenFile(AM_OS_STRING("test_data/diskfile_read_test.txt"), eFileOpenMode_Read);

            file->Close();
            AM_EXPECT_NOT(file->IsValid());
            AM_EXPECT(file->Length() == 0);
            AM_EXPECT(file->GetPtr() == nullptr);
        }
    };

    AM_REGISTER_TEST(fs_disk_file, can_be_closed);
} // namespace SparkyStudios::Audio::Amplitude::Tests
