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

#include <Core/Codecs/MP3/Codec.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_codec_mp3, can_identify_mp3_content)
    {
    public:
        void Run() override
        {
            MP3Codec codec;

            // Test with ID3v2 tag header
            {
                AmUInt8 id3Header[3] = { 'I', 'D', '3' };

                auto file = std::make_shared<MemoryFile>();
                file->Open(sizeof(id3Header));
                file->Write(id3Header, sizeof(id3Header));
                file->Seek(0, eFileSeekOrigin_Start);

                AM_EXPECT(codec.CanHandleFile(file));

                // Verify position is restored after CanHandleFile
                AM_EXPECT_EQ(static_cast<AmSize>(0), file->Position());
            }

            // Test with MPEG audio frame sync (MPEG1, Layer 3, no CRC)
            {
                AmUInt8 mp3Sync[2] = { 0xFF, 0xFB };

                auto file = std::make_shared<MemoryFile>();
                file->Open(sizeof(mp3Sync));
                file->Write(mp3Sync, sizeof(mp3Sync));
                file->Seek(0, eFileSeekOrigin_Start);

                AM_EXPECT(codec.CanHandleFile(file));
            }
        }
    };

    AM_REGISTER_TEST(core_codec_mp3, can_identify_mp3_content);
} // namespace SparkyStudios::Audio::Amplitude::Tests
