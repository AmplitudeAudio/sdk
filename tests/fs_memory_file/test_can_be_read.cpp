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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, fs_memory_file, can_be_read)
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

            file.Seek(1, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 1);
            AM_EXPECT(file.Read8() == 'K');
            file.Seek(-2, eFileSeekOrigin_End);
            AM_EXPECT(file.Position() == 0);
            AM_EXPECT(file.Read8() == 'O');
            file.Seek(-1, eFileSeekOrigin_Current);
            AM_EXPECT(file.Position() == 0);
            AM_EXPECT(file.Read8() == 'O');
            file.Seek(1234, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 2);
            AM_EXPECT(file.Read8() == 0);

            file.Seek(0, eFileSeekOrigin_Start);
            auto* content = static_cast<AmUInt8Buffer>(ammalloc(2));
            AM_EXPECT(file.Read(content, 2) == 2);
            AM_EXPECT(content[0] == 'O');
            AM_EXPECT(content[1] == 'K');
            AM_EXPECT(file.Position() == file.Length());
            AM_EXPECT(file.Eof());
            amfree(content);
        }
    };

    AM_REGISTER_TEST(fs_memory_file, can_be_read);
} // namespace SparkyStudios::Audio::Amplitude::Tests
