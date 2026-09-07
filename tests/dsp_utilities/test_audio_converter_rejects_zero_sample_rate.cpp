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

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, audio_converter_rejects_zero_sample_rate)
    {
    public:
        void Run() override
        {
            AudioConverter converter;

            AudioConverter::Settings settings{};
            settings.m_sourceChannelCount = 1;
            settings.m_targetChannelCount = 1;
            settings.m_sourceSampleRate = 0;
            settings.m_targetSampleRate = 48000;

            AM_EXPECT_NOT(converter.Configure(settings));

            settings.m_sourceSampleRate = 48000;
            settings.m_targetSampleRate = 0;

            AM_EXPECT_NOT(converter.Configure(settings));

            // A valid configuration still succeeds, including one that will be approximated.
            settings.m_sourceSampleRate = 44056;
            settings.m_targetSampleRate = 48000;

            AM_EXPECT(converter.Configure(settings));
        }
    };

    AM_REGISTER_TEST(dsp_utilities, audio_converter_rejects_zero_sample_rate);
} // namespace SparkyStudios::Audio::Amplitude::Tests
