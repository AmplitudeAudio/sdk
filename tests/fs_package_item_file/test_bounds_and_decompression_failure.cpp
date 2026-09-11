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
    AM_TEST_CASE(ComponentTestCase, fs_package_item_file, bounds_and_decompression_failure)
    {
    public:
        void Run() override
        {
            PackageFileItemDescription desc;
            desc.m_Name = "test_item";
            desc.m_Offset = 0;
            desc.m_Size = 1000;
            desc.m_CompressedBlockSize = 500;

            // Empty chunk list should safely return 0 read bytes
            auto memoryFile = ampoolshared(eMemoryPoolKind_IO, MemoryFile);
            PackageItemFile itemFile(&desc, memoryFile, 0);

            AmUInt8 buffer[64];
            AM_EXPECT(itemFile.Read(buffer, sizeof(buffer)) == 0);
            AM_EXPECT(itemFile.Position() == 0);
        }
    };

    AM_REGISTER_TEST(fs_package_item_file, bounds_and_decompression_failure);
} // namespace SparkyStudios::Audio::Amplitude::Tests
