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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_codec_mp3, can_create_decoder)
    {
    public:
        void Run() override
        {
            MP3Codec codec;

            auto decoder = codec.CreateDecoder();

            AM_EXPECT(decoder != nullptr);
            AM_EXPECT(dynamic_cast<MP3Codec::MP3Decoder*>(decoder.get()) != nullptr);
        }
    };

    AM_REGISTER_TEST(core_codec_mp3, can_create_decoder);
} // namespace SparkyStudios::Audio::Amplitude::Tests
