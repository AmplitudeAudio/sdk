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

#include <Utils/ScopedDenormalFlush.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h>
#endif

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_utilities, denormal_flush_guard)
    {
    public:
        void Run() override
        {
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
            const unsigned int stateBefore = _mm_getcsr();

            {
                ScopedDenormalFlush guard;

                // FTZ (bit 15) and DAZ (bit 6) must be set while the guard is active
                AM_EXPECT((_mm_getcsr() & 0x8040) == 0x8040);

                // 1e-38f * 1e-6f = 1e-44 is subnormal: must flush to zero with FTZ+DAZ
                volatile AmReal32 a = 1e-38f;
                volatile AmReal32 b = 1e-6f;
                AM_EXPECT(a * b == 0.0f);
            }

            // Previous FP state must be restored when the guard goes out of scope
            AM_EXPECT(_mm_getcsr() == stateBefore);
#endif
        }
    };

    AM_REGISTER_TEST(dsp_utilities, denormal_flush_guard);
} // namespace SparkyStudios::Audio::Amplitude::Tests
