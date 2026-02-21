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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_audio_buffer, cross_fader_functionality)
    {
    public:
        void Run() override
        {
            // Create input and output buffers
            AudioBuffer in(10, 1);
            AudioBuffer out(10, 1);

            // Initialize both buffers with 1.0f
            for (size_t i = 0; i < 10; ++i)
            {
                in[0][i] = 1.0f;
                out[0][i] = 1.0f;
            }

            // Create fade buffer
            AudioBuffer fade(10, 1);

            // Test cross-fading functionality
            AudioBufferCrossFader crossfader(10);
            crossfader.CrossFade(in, out, fade);

            // Verify the result
            for (size_t i = 0; i < 10; ++i)
                AM_EXPECT(std::abs(1.0f - fade[0][i]) < kEpsilon);
        }
    };

    AM_REGISTER_TEST(core_audio_buffer, cross_fader_functionality);
} // namespace SparkyStudios::Audio::Amplitude::Tests
