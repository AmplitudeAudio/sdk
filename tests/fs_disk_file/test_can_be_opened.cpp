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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, fs_disk_file, can_be_opened)
    {
    public:
        void Run() override
        {
            DiskFileSystem fileSystem;
            fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

            auto fileReadPath = fileSystem.ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));
            auto fileWritePath = fileSystem.ResolvePath(AM_OS_STRING("test_data/diskfile_write_test.txt"));

            // File system based
            {
                const auto& file = fileSystem.OpenFile(AM_OS_STRING("test_data/diskfile_read_test.txt"), eFileOpenMode_Read);

                AM_EXPECT(file->IsValid());
                AM_EXPECT(file->GetPath() == fileReadPath);
                AM_EXPECT(file->Length() == 2);
                AM_EXPECT(file->GetPtr() != nullptr);
            }

            {
                DiskFile temp;
                AM_EXPECT(temp.Open(AM_OS_STRING(""), eFileOpenMode_ReadWrite, eFileOpenKind_Binary) == eErrorCode_InvalidParameter);
            }

            {
                DiskFile temp1(fileReadPath, eFileOpenMode_Read);
                AM_EXPECT(temp1.IsValid());
                AM_EXPECT(temp1.GetPath() == fileReadPath);
                AM_EXPECT(temp1.Length() == 2);
                AM_EXPECT(temp1.GetPtr() != nullptr);
            }

            {
                DiskFile temp2(fileWritePath, eFileOpenMode_Write);
                AM_EXPECT(temp2.IsValid());
                AM_EXPECT(temp2.GetPath() == fileWritePath);
                AM_EXPECT(temp2.Length() == 0);
            }

            {
                DiskFile temp3(fileWritePath, eFileOpenMode_ReadWrite);
                AM_EXPECT(temp3.IsValid());
                AM_EXPECT(temp3.GetPath() == fileWritePath);
                AM_EXPECT(temp3.Length() == 0);
            }

            {
                DiskFile temp4(fileReadPath, eFileOpenMode_ReadAppend);
                AM_EXPECT(temp4.IsValid());
                AM_EXPECT(temp4.GetPath() == fileReadPath);
                AM_EXPECT(temp4.Length() == 2);
            }

            {
                DiskFile temp5(fileWritePath, eFileOpenMode_Append);
                AM_EXPECT(temp5.IsValid());
                AM_EXPECT(temp5.GetPath() == fileWritePath);
                AM_EXPECT(temp5.Length() == 0);
            }
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_disk_file, can_be_opened);
} // namespace SparkyStudios::Audio::Amplitude::Tests
