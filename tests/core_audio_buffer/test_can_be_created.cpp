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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_audio_buffer, can_be_created)
    {
    public:
        void Run() override
        {
            // Test default constructor
            AudioBuffer buffer1;
            AM_EXPECT(buffer1.IsEmpty());

            // Test parameterized constructor
            AudioBuffer buffer2(12345, 3);
            AM_EXPECT_NOT(buffer2.IsEmpty());
            AM_EXPECT(buffer2.GetFrameCount() == 12345);
            AM_EXPECT(buffer2.GetChannelCount() == 3);

            for (AmSize i = 0, l = buffer2.GetChannelCount(); i < l; ++i)
                AM_EXPECT(buffer2[i].enabled());

            // Test move constructor
            AudioBuffer buffer3(std::move(buffer2));
            AM_EXPECT_NOT(buffer3.IsEmpty());
            AM_EXPECT(buffer3.GetFrameCount() == 12345);
            AM_EXPECT(buffer3.GetChannelCount() == 3);
            AM_EXPECT(buffer2.IsEmpty());

            for (AmSize i = 0, l = buffer3.GetChannelCount(); i < l; ++i)
                AM_EXPECT(buffer3[i].enabled());

            // Test copy assignment
            AudioBuffer buffer4;
            buffer4 = buffer3;
            AM_EXPECT_NOT(buffer4.IsEmpty());
            AM_EXPECT(buffer4.GetFrameCount() == buffer3.GetFrameCount());
            AM_EXPECT(buffer4.GetChannelCount() == buffer3.GetChannelCount());

            for (AmSize i = 0, l = buffer4.GetChannelCount(); i < l; ++i)
                AM_EXPECT(buffer4[i].enabled());
        }
    };

    AM_REGISTER_TEST(core_audio_buffer, can_be_created);
} // namespace SparkyStudios::Audio::Amplitude::Tests
