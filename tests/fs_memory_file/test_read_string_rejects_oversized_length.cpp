// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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
    AM_TEST_CASE(ComponentTestCase, fs_memory_file, read_string_rejects_oversized_length)
    {
    public:
        void Run() override
        {
            MemoryFile file;
            file.Open(16);

            file.WriteString("abc");
            file.Seek(0, eFileSeekOrigin_Start);
            AM_EXPECT(file.ReadString() == "abc");

            // A length prefix larger than the rest of the file must not allocate it.
            file.Seek(0, eFileSeekOrigin_Start);
            file.Write32(0xFFFFFFF0u);
            file.Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT(file.ReadString().empty());
            AM_EXPECT(file.Position() == file.Length());
        }
    };

    AM_REGISTER_TEST(fs_memory_file, read_string_rejects_oversized_length);
} // namespace SparkyStudios::Audio::Amplitude::Tests
