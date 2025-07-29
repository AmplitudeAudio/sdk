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

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    DiskFileSystem fileSystem;
    fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

    const auto& file = fileSystem.OpenFile(AM_OS_STRING("test_data/diskfile_write_test.txt"), eFileOpenMode_ReadWrite);

    file->Seek(0, eFileSeekOrigin_Start);
    AM_EXPECT(file->Write8('K') == 1);
    AM_EXPECT(file->Write8('O') == 1);

    file->Seek(0, eFileSeekOrigin_Start);
    AM_EXPECT(file->Read8() == 'K');
    AM_EXPECT(file->Read8() == 'O');

    {
        file->Seek(0, eFileSeekOrigin_Start);
        auto* content = static_cast<AmUInt8Buffer>(ammalloc(2));
        content[0] = 'O';
        content[1] = 'K';
        AM_EXPECT(file->Write(content, 2) == 2);
        AM_EXPECT(file->Position() == file->Length());
        AM_EXPECT(file->Eof());
        amfree(content);
    }

    {
        file->Seek(0);
        auto* content = static_cast<AmUInt8Buffer>(ammalloc(2));
        AM_EXPECT(file->Read(content, file->Length()) == file->Length());
        AM_EXPECT(content[0] == 'O');
        AM_EXPECT(content[1] == 'K');
        AM_EXPECT(file->Position() == file->Length());
        AM_EXPECT(file->Eof());
        amfree(content);
    }
}
