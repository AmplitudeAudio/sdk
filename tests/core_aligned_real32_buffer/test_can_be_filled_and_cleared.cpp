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

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    AmAlignedReal32Buffer buffer;
    buffer.Init(1024);

    // Fill buffer
    for (AmUInt32 i = 0; i < 1024; ++i)
        buffer[i] = 1.0f;

    // Verify filled values
    for (AmUInt32 i = 0; i < 1024; ++i)
        AM_EXPECT(buffer[i] == 1.0f);

    // Clear buffer
    buffer.Clear();

    // Verify cleared values
    for (AmUInt32 i = 0; i < 1024; ++i)
        AM_EXPECT(buffer[i] == 0.0f);
}
