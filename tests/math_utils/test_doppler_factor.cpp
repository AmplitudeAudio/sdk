// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <Math/LinearAlgebra.h>
#include <Utils/Utils.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, math_utils, doppler_factor)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 soundSpeed = 343.0f;
            constexpr AmVector3 source = { 10.0f, 25.0f, 1.0f };
            constexpr AmVector3 listener = { 0.0f, 0.0f, 0.0f };

            const AmReal32 dopplerFactor = ComputeDopplerFactor(Sub(source, listener), source, listener, soundSpeed, 1.0f);

            AM_EXPECT(std::abs(dopplerFactor - 0.927166343f) < kEpsilon);
        }
    };

    AM_REGISTER_TEST(math_utils, doppler_factor);
} // namespace SparkyStudios::Audio::Amplitude::Tests
