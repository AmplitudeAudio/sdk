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
    AM_TEST_CASE(SimpleTestCase, fs_memory_file, can_be_opened)
    {
    public:
        void Run() override
        {
            MemoryFile file;
            file.Open(2);

            file.Seek(0, eFileSeekOrigin_Start);
            file.Write8('O');
            file.Write8('K');
            file.Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.GetPath().empty());
            AM_EXPECT(file.Length() == 2);
            AM_EXPECT(file.GetPtr() != nullptr);

            {
                auto fileSystem = CreatePlatformFileSystem();
                fileSystem->SetBasePath(GetPlatformAssetsBasePath());

                char ok[] = "OK";

                file.Close();
                AM_EXPECT(file.OpenMem(nullptr, 2) == eErrorCode_InvalidParameter);
                AM_EXPECT_NOT(file.IsValid());
                AM_EXPECT(file.OpenMem(reinterpret_cast<AmConstUInt8Buffer>(ok), 2, false, false) == eErrorCode_Success);
                AM_EXPECT(file.IsValid());
                AM_EXPECT(file.GetPtr() == ok);

                file.Close();
                AM_EXPECT(ok[0] == 'O');
                AM_EXPECT(ok[1] == 'K');
                AM_EXPECT(file.OpenMem(reinterpret_cast<AmConstUInt8Buffer>(ok), 2, true, true) == eErrorCode_Success);
                AM_EXPECT(file.IsValid());
                AM_EXPECT(file.Read(reinterpret_cast<AmUInt8Buffer>(ok), 2) == 2);
                AM_EXPECT(ok[0] == 'O');
                AM_EXPECT(ok[1] == 'K');

                file.Close();
                AM_EXPECT(file.OpenToMem("") == eErrorCode_InvalidParameter);
                AM_EXPECT_NOT(file.IsValid());
                AM_EXPECT(file.OpenFileToMem(fileSystem->OpenFile(AM_OS_STRING("test_data/diskfile_read_test.txt"))) == eErrorCode_Success);
                AM_EXPECT(file.IsValid());
                AM_EXPECT(file.Read(reinterpret_cast<AmUInt8Buffer>(ok), 2) == 2);
                AM_EXPECT(ok[0] == 'O');
                AM_EXPECT(ok[1] == 'K');
            }
        }
    };

    AM_TEST_CASE(SimpleTestCase, fs_memory_file, can_be_opened_with_disk_file)
    {
    public:
        void Run() override
        {
            MemoryFile file;

            char ok[] = "OK";

            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            file.Close();
            AM_EXPECT(file.OpenToMem("") == eErrorCode_InvalidParameter);
            AM_EXPECT_NOT(file.IsValid());
            AM_EXPECT(file.OpenToMem(fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"))) == eErrorCode_Success);
            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.Read(reinterpret_cast<AmUInt8Buffer>(ok), 2) == 2);
            AM_EXPECT(ok[0] == 'O');
            AM_EXPECT(ok[1] == 'K');
        }
    };

    AM_REGISTER_TEST(fs_memory_file, can_be_opened);
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_memory_file, can_be_opened_with_disk_file);
} // namespace SparkyStudios::Audio::Amplitude::Tests
