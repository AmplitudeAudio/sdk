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

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Filters/BiquadResonantFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, biquad_resonant_copy_state_from)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 256;
            constexpr AmUInt32 sampleRate = 48000;

            BiquadResonantFilter factory;
            factory.InitializeLowPass(1000.0f, 0.707f);

            auto source = std::static_pointer_cast<BiquadResonantFilterInstance>(factory.CreateInstance());
            auto copy = std::static_pointer_cast<BiquadResonantFilterInstance>(factory.CreateInstance());
            auto fresh = std::static_pointer_cast<BiquadResonantFilterInstance>(factory.CreateInstance());

            AudioBuffer input(frames, 1);
            GenerateSineWave(input, sampleRate);

            AudioBuffer scratch(frames, 1);
            source->Process(input, scratch, frames, sampleRate);

            copy->CopyStateFrom(*source);

            AudioBuffer outSource(frames, 1);
            AudioBuffer outCopy(frames, 1);
            AudioBuffer outFresh(frames, 1);
            source->Process(input, outSource, frames, sampleRate);
            copy->Process(input, outCopy, frames, sampleRate);
            fresh->Process(input, outFresh, frames, sampleRate);

            AmReal32 maxCopyDiff = 0.0f;
            AmReal32 maxFreshDiff = 0.0f;
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                maxCopyDiff = std::max(maxCopyDiff, std::abs(outSource[0][i] - outCopy[0][i]));
                maxFreshDiff = std::max(maxFreshDiff, std::abs(outSource[0][i] - outFresh[0][i]));
            }

            // Precondition: history matters for this signal.
            AM_EXPECT(maxFreshDiff > 1e-3f);
            // With copied history the outputs are identical.
            AM_EXPECT(maxCopyDiff < 1e-6f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, biquad_resonant_copy_state_from);
} // namespace SparkyStudios::Audio::Amplitude::Tests
