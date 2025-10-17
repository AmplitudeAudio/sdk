// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <Ambisonics/BFormat.h>
#include <Core/RoomInternalState.h>
#include <DSP/Filters/MonoPoleFilter.h>
#include <DSP/ReflectionsProcessor.h>

#include "DSPTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void DSPTestCase::Run()
    {
        auto monopoleFilter = Engine::RegisterExtension<MonoPoleFilter>();

        constexpr AmUInt32 sampleRate = 48000;
        constexpr AmSize frameCount = 512;

        ReflectionsProcessor processor(sampleRate, frameCount);

        // Create input buffer (mono)
        AudioBuffer input(frameCount, 1);

        // Generate test signal
        GenerateSineWave(input, sampleRate);

        // Create output BFormat buffer
        BFormat output;
        output.Configure(1, true, frameCount);

        // Process without room state (should produce output but minimal processing)
        processor.Process(input, &output);

        // Verify the processor runs without errors
        AM_EXPECT(output.GetChannelCount() >= kAmFirstOrderAmbisonicChannelCount);
        AM_EXPECT(output.GetSampleCount() == frameCount);

        Engine::UnregisterExtension(monopoleFilter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
