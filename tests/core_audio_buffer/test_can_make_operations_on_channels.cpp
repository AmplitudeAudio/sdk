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
#include <Utils/Utils.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_audio_buffer, can_make_operations_on_channels)
    {
    public:
        void Run() override
        {
            // Create and populate buffer1 using raw buffer access
            AudioBuffer buffer1(123, 2);
#ifdef AM_SIMD_INTRINSICS
            const AmSize alignedSize = FindNextAlignedArrayIndex<AmReal32>(123, AM_SIMD_ALIGNMENT);
#else
            const AmSize alignedSize = 123;
#endif
            for (AmSize i = 0; i < 2; ++i)
                for (AmSize j = 0; j < 123; j++)
                    buffer1.GetData().GetBuffer()[alignedSize * i + j] = (123.0f * i) + j;

            // Create and populate buffer2 using channel indexing
            AudioBuffer buffer2(123, 2);
            for (AmSize i = 0; i < 2; ++i)
                for (AmSize j = 0; j < 123; ++j)
                    buffer2[i][j] = (123.0f * i) + j;

            // Create empty buffer3 and verify it's zeroed
            AudioBuffer buffer3(123, 2);
            for (AmSize i = 0; i < 2; ++i)
                for (AmSize j = 0; j < 123; ++j)
                    AM_EXPECT(buffer3[i][j] == 0);

            // Test addition operation
            buffer1 += buffer2;
            for (AmSize i = 0; i < 2; ++i)
                for (AmSize j = 0; j < 123; ++j)
                    AM_EXPECT(buffer1[i][j] == ((123.0f * i) + j) * 2.0f);

            // Test subtraction operation
            buffer2 -= buffer1;
            for (AmSize i = 0; i < 2; ++i)
                for (AmSize j = 0; j < 123; ++j)
                    AM_EXPECT(buffer2[i][j] == ((123.0f * i) + j) * -1.0f);

            // Test multiplication with buffer (should zero out buffer1)
            buffer1 *= buffer3;
            for (AmSize i = 0; i < 2; ++i)
                for (AmSize j = 0; j < 123; ++j)
                    AM_EXPECT(buffer1[i][j] == 0);

            // Test scalar multiplication
            buffer2 *= -1;
            for (AmSize i = 0; i < 2; ++i)
                for (AmSize j = 0; j < 123; ++j)
                    AM_EXPECT(buffer2[i][j] == (123.0f * i) + j);
        }
    };

    AM_REGISTER_TEST(core_audio_buffer, can_make_operations_on_channels);
} // namespace SparkyStudios::Audio::Amplitude::Tests
