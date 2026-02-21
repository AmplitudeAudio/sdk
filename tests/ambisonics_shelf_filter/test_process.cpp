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
    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_bformat_in_place)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 sampleCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            AmbisonicShelfFilter filter;
            AM_EXPECT(filter.Configure(1, true, sampleCount, sampleRate));

            BFormat buffer;
            AM_EXPECT(buffer.Configure(1, true, sampleCount));

            AudioBuffer& audioBuffer = *buffer.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer, sampleRate, 440.0f, 0.5f);

            filter.Process(&buffer, sampleCount);

            ExpectNotSilent(audioBuffer, 0.01f);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_bformat_in_place);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_null_buffer_safe)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;
            filter.Configure(1, true, 512, 48000);

            filter.Process(nullptr, 512);

            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_null_buffer_safe);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_zero_samples_safe)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;
            filter.Configure(1, true, 512, 48000);

            BFormat buffer;
            buffer.Configure(1, true, 512);

            filter.Process(&buffer, 0);

            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_zero_samples_safe);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_exceeds_max_block_size_safe)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 maxBlockSize = 512;
            constexpr AmUInt32 sampleRate = 48000;

            AmbisonicShelfFilter filter;
            filter.Configure(1, true, maxBlockSize, sampleRate);

            BFormat buffer;
            buffer.Configure(1, true, maxBlockSize * 2);

            AudioBuffer& audioBuffer = *buffer.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer, sampleRate, 440.0f, 0.5f);

            filter.Process(&buffer, maxBlockSize * 2);

            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_exceeds_max_block_size_safe);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_without_configure_safe)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;

            BFormat buffer;
            buffer.Configure(1, true, 512);

            filter.Process(&buffer, 512);

            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_without_configure_safe);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_multiple_orders)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 sampleCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            for (AmUInt32 order = 1; order <= 3; ++order)
            {
                AmbisonicShelfFilter filter;
                AM_EXPECT(filter.Configure(order, true, sampleCount, sampleRate));

                BFormat buffer;
                AM_EXPECT(buffer.Configure(order, true, sampleCount));

                AudioBuffer& audioBuffer = *buffer.GetBuffer();
                GenerateSineWaveAtFrequency(audioBuffer, sampleRate, 440.0f, 0.5f);

                filter.Process(&buffer, sampleCount);

                ExpectNotSilent(audioBuffer, 0.01f);
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_multiple_orders);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_2d_vs_3d)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 sampleCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            AmbisonicShelfFilter filter2D;
            AM_EXPECT(filter2D.Configure(2, false, sampleCount, sampleRate));

            BFormat buffer2D;
            AM_EXPECT(buffer2D.Configure(2, false, sampleCount));

            AudioBuffer& audioBuffer2D = *buffer2D.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer2D, sampleRate, 440.0f, 0.5f);

            filter2D.Process(&buffer2D, sampleCount);
            ExpectNotSilent(audioBuffer2D, 0.01f);

            AmbisonicShelfFilter filter3D;
            AM_EXPECT(filter3D.Configure(2, true, sampleCount, sampleRate));

            BFormat buffer3D;
            AM_EXPECT(buffer3D.Configure(2, true, sampleCount));

            AudioBuffer& audioBuffer3D = *buffer3D.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer3D, sampleRate, 440.0f, 0.5f);

            filter3D.Process(&buffer3D, sampleCount);
            ExpectNotSilent(audioBuffer3D, 0.01f);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_2d_vs_3d);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_applies_max_re_gains)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 sampleCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            AmbisonicShelfFilter filter;
            AM_EXPECT(filter.Configure(1, true, sampleCount, sampleRate));

            auto gains = filter.GetMaxReGains();

            BFormat buffer;
            AM_EXPECT(buffer.Configure(1, true, sampleCount));

            AudioBuffer& audioBuffer = *buffer.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer, sampleRate, 440.0f, 0.5f);

            filter.Process(&buffer, sampleCount);

            ExpectNotSilent(audioBuffer, 0.01f);

            AM_EXPECT(gains[0] > gains[1]);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_applies_max_re_gains);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_preserves_dc_free_signal)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 sampleCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            AmbisonicShelfFilter filter;
            AM_EXPECT(filter.Configure(1, true, sampleCount, sampleRate));

            BFormat buffer;
            AM_EXPECT(buffer.Configure(1, true, sampleCount));

            AudioBuffer& audioBuffer = *buffer.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer, sampleRate, 440.0f, 0.5f);

            filter.Process(&buffer, sampleCount);

            ExpectNoDCOffset(audioBuffer, 0.1f);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_preserves_dc_free_signal);
} // namespace SparkyStudios::Audio::Amplitude::Tests
