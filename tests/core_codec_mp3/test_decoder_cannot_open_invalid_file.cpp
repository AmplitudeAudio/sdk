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
    AM_TEST_CASE(ComponentTestCase, core_codec_mp3, decoder_cannot_open_invalid_file)
    {
    public:
        void Run() override
        {
            MP3Codec codec;
            auto decoder = codec.CreateDecoder();

            auto invalidFile = std::make_shared<MemoryFile>();
            invalidFile->Open(16);

            AmUInt8 dummyData[16] = { 0 };
            invalidFile->Write(dummyData, 16);
            invalidFile->Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT_NOT(decoder->Open(invalidFile));
        }
    };

    AM_REGISTER_TEST(core_codec_mp3, decoder_cannot_open_invalid_file);
} // namespace SparkyStudios::Audio::Amplitude::Tests
