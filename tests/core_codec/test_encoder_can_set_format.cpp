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

#include "MockCodec.h"
#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_codec, encoder_can_set_format)
    {
    public:
        void Run() override
        {
            MockCodec codec("test_encoder_format");
            auto encoder = codec.CreateEncoder();

            SoundFormat format;
            format.SetAll(48000, 2, 24, 5000, 8, eAudioSampleFormat_Float32);

            encoder->SetFormat(format);

            auto file = std::make_shared<MemoryFile>();
            file->Open(1024);

            AM_EXPECT(encoder->Open(file));

            encoder->Close();
        }
    };

    AM_REGISTER_TEST(core_codec, encoder_can_set_format);
} // namespace SparkyStudios::Audio::Amplitude::Tests
