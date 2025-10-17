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
        // Create an impulse response
        constexpr AmSize irLen = 512;
        std::vector<AmAudioSample> ir(irLen);

        for (AmSize i = 0; i < irLen; ++i)
            ir[i] = 0.5f;

        Convolver convolver;
        constexpr AmSize blockSize = 256;
        AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));

        // Verify initialized state
        AM_EXPECT(convolver.GetSegmentCount() > 0);
        AM_EXPECT(convolver.GetSegmentSize() > 0);

        // Reset
        convolver.Reset();

        // Verify reset state
        AM_EXPECT(convolver.GetSegmentCount() == 0);
        AM_EXPECT(convolver.GetSegmentSize() == 0);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
