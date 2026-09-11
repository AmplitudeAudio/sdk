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

#include <Core/Codecs/AMS/Codec.h>

#include "ComponentTestCase.h"
#include "PlatformTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_codec_ams, can_seek_with_dynamic_data_offset)
    {
    public:
        void Run() override
        {
            auto fileSystem = CreatePlatformFileSystem();
            fileSystem->SetBasePath(GetPlatformAssetsBasePath());

            auto file = fileSystem->OpenFile(AM_OS_STRING("test_data/retro_synth_adpcm.wav"), eFileOpenMode_Read);
            if (!file || !file->IsValid())
                return;

            AMSCodec codec;
            auto decoder = codec.CreateDecoder();
            AM_EXPECT(decoder->Open(file));

            // Seeking should succeed and land accurately on data block boundaries
            AM_EXPECT(decoder->Seek(0));
            AM_EXPECT(decoder->Seek(500));

            decoder->Close();
        }
    };

    AM_REGISTER_TEST(core_codec_ams, can_seek_with_dynamic_data_offset);
} // namespace SparkyStudios::Audio::Amplitude::Tests
