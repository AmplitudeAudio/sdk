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

#include "DSPTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void DSPTestCase::Run()
    {
        constexpr AmSize blockSize = 256;
        constexpr AmSize irLen = 512;

        {
            // Create an impulse response
            std::vector<AmAudioSample> ir(irLen, 1.0f);

            Convolver convolver;

            // Test initialization with valid parameters
            AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));

            // Verify segment size and count
            AM_EXPECT(convolver.GetSegmentSize() == 512); // 2 * blockSize
            AM_EXPECT(convolver.GetSegmentCount() == 2); // ceil(512 / 256)
        }

        {
            // Create an impulse response
            std::vector<AmAudioSample> ir(irLen, 0.0f);

            Convolver convolver;

            // Test initialization with valid parameters
            AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));

            // Verify segment size and count
            AM_EXPECT(convolver.GetSegmentSize() == 0); // Empty IR
            AM_EXPECT(convolver.GetSegmentCount() == 0); // Empty IR
        }

        {
            // Create an impulse response
            std::vector<AmAudioSample> ir(irLen, 0.0f);

            for (AmSize i = 0; i < irLen / 2; ++i)
                ir[i] = 1.0f;

            Convolver convolver;

            // Test initialization with valid parameters
            AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));

            // Verify segment size and count
            AM_EXPECT(convolver.GetSegmentSize() == 512); // 2 * blockSize
            AM_EXPECT(convolver.GetSegmentCount() == 1); // ceil(512 / 2 / 256)
        }
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
