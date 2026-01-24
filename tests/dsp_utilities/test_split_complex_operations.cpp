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
    AM_TEST_CASE(DSPTestCase, dsp_utilities, split_complex_operations)
    {
    public:
        void Run() override
        {
            // Test creation and initialization
            constexpr AmSize size = 256;
            SplitComplex splitComplex(size);

            AM_EXPECT(splitComplex.GetSize() == size);
            AM_EXPECT(splitComplex.re() != nullptr);
            AM_EXPECT(splitComplex.im() != nullptr);

            // Test filling with data
            for (AmSize i = 0; i < size; ++i)
        {
            splitComplex.re()[i] = static_cast<AmReal32>(i);
            splitComplex.im()[i] = static_cast<AmReal32>(i) * 2.0f;
            }

            // Test accessing via operator[]
            const auto complex0 = splitComplex[0];
            AM_EXPECT(std::abs(complex0.real() - 0.0f) < kEpsilon);
            AM_EXPECT(std::abs(complex0.imag() - 0.0f) < kEpsilon);

            const auto complex10 = splitComplex[10];
            AM_EXPECT(std::abs(complex10.real() - 10.0f) < kEpsilon);
            AM_EXPECT(std::abs(complex10.imag() - 20.0f) < kEpsilon);

            // Test clear
            splitComplex.Clear();
            AM_EXPECT(std::abs(splitComplex.re()[0]) < kEpsilon);
            AM_EXPECT(std::abs(splitComplex.im()[0]) < kEpsilon);

            // Test copy
            SplitComplex source(size);
            for (AmSize i = 0; i < size; ++i)
        {
            source.re()[i] = 1.0f;
            source.im()[i] = 2.0f;
            }

            splitComplex.CopyFrom(source);
            AM_EXPECT(std::abs(splitComplex.re()[5] - 1.0f) < kEpsilon);
            AM_EXPECT(std::abs(splitComplex.im()[5] - 2.0f) < kEpsilon);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, split_complex_operations);
} // namespace SparkyStudios::Audio::Amplitude::Tests
