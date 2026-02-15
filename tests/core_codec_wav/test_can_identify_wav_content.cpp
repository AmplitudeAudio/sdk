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

#include <cstring>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Codecs/WAV/Codec.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_codec_wav, can_identify_wav_content)
    {
    public:
        void Run() override
        {
            WAVCodec codec;

            // Build a minimal RIFF WAVE PCM header (22 bytes)
            AmUInt8 wavHeader[22] = {};
            std::memcpy(wavHeader, "RIFF", 4);
            std::memcpy(wavHeader + 8, "WAVE", 4);
            std::memcpy(wavHeader + 12, "fmt ", 4);
            wavHeader[16] = 16; // fmt chunk size
            wavHeader[20] = 0x01; // format tag = PCM (0x0001 LE)
            wavHeader[21] = 0x00;

            auto file = std::make_shared<MemoryFile>();
            file->Open(sizeof(wavHeader));
            file->Write(wavHeader, sizeof(wavHeader));
            file->Seek(0, eFileSeekOrigin_Start);

            AM_EXPECT(codec.CanHandleFile(file));

            // Verify position is restored after CanHandleFile
            AM_EXPECT_EQ(static_cast<AmSize>(0), file->Position());
        }
    };

    AM_REGISTER_TEST(core_codec_wav, can_identify_wav_content);
} // namespace SparkyStudios::Audio::Amplitude::Tests
