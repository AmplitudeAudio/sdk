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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_audio_buffer, can_be_cloned_and_copied)
    {
    public:
        void Run() override
        {
            // Create and populate buffer1
            AudioBuffer buffer1(123, 1);
            for (AmSize i = 0; i < 123; ++i)
                buffer1[0][i] = i;

            // Test cloning
            AudioBuffer buffer2;
            buffer2 = buffer1.Clone();
            for (AmSize i = 0; i < 123; ++i)
                AM_EXPECT(buffer2[0][i] == buffer1[0][i]);

            // Test partial copy
            AudioBuffer buffer3(23, 1);
            AudioBuffer::Copy(buffer2, 100, buffer3, 0, 23);
            for (AmSize i = 0; i < 23; ++i)
                AM_EXPECT(buffer3[0][i] == buffer2[0][100 + i]);

            // Test vector assignment
            AudioBuffer buffer4(123, 1);
            const std::vector<AmReal32> data(123, 1.0f);
            buffer4[0] = data;
            for (AmSize i = 0; i < 123; ++i)
                AM_EXPECT(buffer4[0][i] == 1.0f);
        }
    };

    AM_REGISTER_TEST(core_audio_buffer, can_be_cloned_and_copied);
} // namespace SparkyStudios::Audio::Amplitude::Tests
