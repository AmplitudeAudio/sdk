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
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_codec, can_create_encoder)
    {
    public:
        void Run() override
        {
            MockCodec codec("test_create_encoder");

            auto encoder = codec.CreateEncoder();

            AM_EXPECT(encoder != nullptr);
            AM_EXPECT(dynamic_cast<MockEncoder*>(encoder.get()) != nullptr);
        }
    };

    AM_REGISTER_TEST(core_codec, can_create_encoder);
} // namespace SparkyStudios::Audio::Amplitude::Tests
