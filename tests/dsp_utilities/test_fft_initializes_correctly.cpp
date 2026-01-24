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
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, fft_initializes_correctly)
    {
    public:
        void Run() override
        {
            FFT fft;

            // Initialize with power-of-2 size
            constexpr AmSize size = 1024;
            fft.Initialize(size);

            // Verify output size calculation
            const AmUInt64 outputSize = FFT::GetOutputSize(size);
            AM_EXPECT(outputSize > 0);
            AM_EXPECT(outputSize == size / 2 + 1); // Complex output size for real FFT
        }
    };

    AM_REGISTER_TEST(dsp_utilities, fft_initializes_correctly);
} // namespace SparkyStudios::Audio::Amplitude::Tests
