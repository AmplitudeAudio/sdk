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

#include "PlatformTestCase.h"
#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, fs_mapped_file, write_returns_zero)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            const auto resolvedPath = fileSystem->ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"));

            MappedFile file;
            AM_EXPECT(file.Open(resolvedPath) == eErrorCode_Success);

            const AmUInt8 data[] = { 0xAB, 0xCD };
            AM_EXPECT(file.Write(data, 2) == 0);

            // Data should be unchanged
            auto* ptr = static_cast<const AmUInt8*>(file.GetPtr());
            AM_EXPECT(ptr[0] == 'O');
            AM_EXPECT(ptr[1] == 'K');
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_mapped_file, write_returns_zero);
} // namespace SparkyStudios::Audio::Amplitude::Tests
