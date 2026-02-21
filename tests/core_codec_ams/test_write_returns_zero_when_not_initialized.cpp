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

#include <Core/Codecs/AMS/Codec.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_codec_ams, write_returns_zero_when_not_initialized)
    {
    public:
        void Run() override
        {
            AMSCodec codec;
            auto encoder = codec.CreateEncoder();

            AudioBuffer buffer(1024, 1);
            AM_EXPECT(encoder->Write(&buffer, 0, 1024) == 0);
        }
    };

    AM_REGISTER_TEST(core_codec_ams, write_returns_zero_when_not_initialized);
} // namespace SparkyStudios::Audio::Amplitude::Tests
