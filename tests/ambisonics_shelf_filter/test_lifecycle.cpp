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

#include <Ambisonics/AmbisonicShelfFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, reset_clears_filter_state)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;
            filter.Configure(1, true, 512, 48000);

            BFormat buffer;
            buffer.Configure(1, true, 512);

            for (AmUInt32 ch = 0; ch < buffer.GetChannelCount(); ++ch)
            {
                for (AmUInt32 i = 0; i < 512; ++i)
                {
                    buffer.SetSample(ch, i, 1.0f);
                }
            }

            filter.Process(&buffer, 512);

            filter.Reset();

            BFormat newBuffer;
            newBuffer.Configure(1, true, 512);

            for (AmUInt32 ch = 0; ch < newBuffer.GetChannelCount(); ++ch)
            {
                for (AmUInt32 i = 0; i < 512; ++i)
                {
                    newBuffer.SetSample(ch, i, 1.0f);
                }
            }

            filter.Process(&newBuffer, 512);

            // After reset, filter state is cleared, so output should match the first process call
            for (AmUInt32 ch = 0; ch < buffer.GetChannelCount(); ++ch)
            {
                for (AmUInt32 i = 0; i < 512; ++i)
                {
                    ExpectFloatNear(buffer.GetSample(ch, i), newBuffer.GetSample(ch, i), 0.01f);
                }
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, reset_clears_filter_state);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, refresh_is_noop)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;
            filter.Configure(1, true, 512, 48000);

            auto gainsBefore = filter.GetMaxReGains();

            filter.Refresh();

            auto gainsAfter = filter.GetMaxReGains();

            AM_EXPECT_EQ(gainsBefore.size(), gainsAfter.size());
            for (size_t i = 0; i < gainsBefore.size(); ++i)
            {
                ExpectFloatNear(gainsBefore[i], gainsAfter[i]);
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, refresh_is_noop);
} // namespace SparkyStudios::Audio::Amplitude::Tests
