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

            AudioBuffer& audioBuffer = *buffer.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer, 48000, 440.0f, 0.5f);

            filter.Process(&buffer, 0);

            ExpectNotSilent(audioBuffer, 0.01f);
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

            ExpectNotSilent(audioBuffer, 0.01f);
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

            AudioBuffer& audioBuffer = *buffer.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer, 48000, 440.0f, 0.5f);

            filter.Process(&buffer, 512);

            ExpectNotSilent(audioBuffer, 0.01f);
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

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_dc_signal_stable)
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
            for (AmUInt16 c = 0; c < audioBuffer.GetChannelCount(); ++c)
                for (AmUInt64 i = 0; i < audioBuffer.GetFrameCount(); ++i)
                    audioBuffer[c][i] = 1.0f;

            filter.Process(&buffer, sampleCount);

            // With DC input, the low-pass passes through (gain ~1) and high-pass is attenuated (gain ~0).
            // The max-rE gain for order 0 is ~1.0, for order 1 is ~0.666.
            // Output should be approximately lp + gain * hp ≈ 1.0 + gain * 0 = 1.0 for all channels.
            // We skip the first 100 samples to allow the 4th-order IIR filter transients to settle.
            for (AmUInt16 c = 0; c < audioBuffer.GetChannelCount(); ++c)
            {
                for (AmUInt64 i = 100; i < audioBuffer.GetFrameCount(); ++i)
                {
                    ExpectFloatNear(1.0f, audioBuffer[c][i], 0.1f);
                }
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_dc_signal_stable);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, process_multi_block_continuity)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 blockSize = 256;
            constexpr AmUInt32 sampleRate = 48000;
            constexpr AmUInt32 totalSamples = 512;

            AmbisonicShelfFilter filter;
            AM_EXPECT(filter.Configure(1, true, blockSize, sampleRate));

            BFormat inputBuffer;
            AM_EXPECT(inputBuffer.Configure(1, true, totalSamples));

            AudioBuffer& audioBuffer = *inputBuffer.GetBuffer();
            GenerateSineWaveAtFrequency(audioBuffer, sampleRate, 440.0f, 0.5f);

            // Process in two blocks
            BFormat block1, block2;
            AM_EXPECT(block1.Configure(1, true, blockSize));
            AM_EXPECT(block2.Configure(1, true, blockSize));

            for (AmUInt16 c = 0; c < audioBuffer.GetChannelCount(); ++c)
            {
                std::memcpy(block1.GetBufferChannel(c).begin(), audioBuffer[c].begin(), sizeof(AmReal32) * blockSize);
                std::memcpy(block2.GetBufferChannel(c).begin(), audioBuffer[c].begin() + blockSize, sizeof(AmReal32) * blockSize);
            }

            filter.Process(&block1, blockSize);
            filter.Process(&block2, blockSize);

            // Output should not be silent and should be continuous (no massive jumps)
            ExpectNotSilent(*block1.GetBuffer(), 0.01f);
            ExpectNotSilent(*block2.GetBuffer(), 0.01f);

            // Check that the last sample of block1 and first sample of block2 are reasonably close
            // (they won't be exact due to filter state, but with the buffer corruption bug they differ wildly)
            for (AmUInt16 c = 0; c < block1.GetBuffer()->GetChannelCount(); ++c)
            {
                AmReal32 endBlock1 = block1.GetBufferChannel(c)[blockSize - 1];
                AmReal32 startBlock2 = block2.GetBufferChannel(c)[0];
                AM_EXPECT(std::abs(endBlock1 - startBlock2) < 0.5f);
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, process_multi_block_continuity);
} // namespace SparkyStudios::Audio::Amplitude::Tests
