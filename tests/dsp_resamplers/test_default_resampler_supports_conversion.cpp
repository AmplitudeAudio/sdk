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

#include <DSP/Resamplers/DefaultResampler.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        // Test supported conversions
        AM_EXPECT(DefaultResamplerInstance::IsConversionSupported(44100, 48000));
        AM_EXPECT(DefaultResamplerInstance::IsConversionSupported(48000, 44100));
        AM_EXPECT(DefaultResamplerInstance::IsConversionSupported(22050, 44100));
        AM_EXPECT(DefaultResamplerInstance::IsConversionSupported(44100, 22050));

        // Same rate should be supported
        AM_EXPECT(DefaultResamplerInstance::IsConversionSupported(44100, 44100));
        AM_EXPECT(DefaultResamplerInstance::IsConversionSupported(48000, 48000));
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
