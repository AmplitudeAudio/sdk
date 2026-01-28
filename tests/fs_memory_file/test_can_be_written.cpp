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
    AM_TEST_CASE(SimpleTestCase, fs_memory_file, can_be_written)
    {
    public:
        void Run() override
        {
            MemoryFile file;
            file.Open(32);

            AM_EXPECT(file.IsValid());
            AM_EXPECT(file.GetPath().empty());
            AM_EXPECT(file.Length() == 32);

            file.Seek(0, eFileSeekOrigin_Start);
            file.Write8('O');
            file.Write8('K');
            file.Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT(file.Read8() == 'O');
            AM_EXPECT(file.Read8() == 'K');

            char ok[] = "OKOKOKOK";

            file.Seek(0, eFileSeekOrigin_Start);
            file.Write16(reinterpret_cast<AmUInt16*>(ok)[0]);
            file.Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT(file.Read8() == 'O');
            AM_EXPECT(file.Read8() == 'K');

            file.Seek(0, eFileSeekOrigin_Start);
            file.Write64(reinterpret_cast<AmUInt64*>(ok)[0]);
            file.Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT(file.Read8() == 'O');
            AM_EXPECT(file.Read8() == 'K');
            AM_EXPECT(file.Read8() == 'O');
            AM_EXPECT(file.Read8() == 'K');
            AM_EXPECT(file.Read8() == 'O');
            AM_EXPECT(file.Read8() == 'K');
            AM_EXPECT(file.Read8() == 'O');
            AM_EXPECT(file.Read8() == 'K');

            file.Seek(0, eFileSeekOrigin_Start);
            file.WriteString(ok);
            file.Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT(file.ReadString() == ok);
        }
    };

    AM_REGISTER_TEST(fs_memory_file, can_be_written);
} // namespace SparkyStudios::Audio::Amplitude::Tests
