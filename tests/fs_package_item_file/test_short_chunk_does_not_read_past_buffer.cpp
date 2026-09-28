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
    AM_TEST_CASE(ComponentTestCase, fs_package_item_file, short_chunk_does_not_read_past_buffer)
    {
    public:
        void Run() override
        {
            // A single LZ4 block holding the 3 literals "abc".
            AmUInt8 compressed[] = { 0x30, 'a', 'b', 'c' };

            auto memoryFile = ampoolshared(eMemoryPoolKind_IO, MemoryFile);
            memoryFile->OpenMem(compressed, sizeof(compressed), true);

            // The chunk claims a 10-byte block but only decompresses to 3 bytes.
            PackageFileItemDescription desc;
            desc.m_Name = "short_chunk_item";
            desc.m_Offset = 0;
            desc.m_Size = 10;
            desc.m_CompressedBlockSize = 10;

            auto& chunk = desc.m_CompressedChunks.emplace_back();
            chunk.m_Offset = 0;
            chunk.m_Size = 3;
            chunk.m_CompressedSize = sizeof(compressed);

            PackageItemFile itemFile(&desc, memoryFile, 0);

            AmUInt8 buffer[5] = {};
            AM_EXPECT(itemFile.Read(buffer, 3) == 3);
            AM_EXPECT(buffer[0] == 'a' && buffer[1] == 'b' && buffer[2] == 'c');

            // Past the chunk's 3 bytes nothing is left to copy.
            itemFile.Seek(5, eFileSeekOrigin_Start);
            AM_EXPECT(itemFile.Read(buffer, sizeof(buffer)) == 0);
        }
    };

    AM_REGISTER_TEST(fs_package_item_file, short_chunk_does_not_read_past_buffer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
