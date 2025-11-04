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

#pragma once

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EntityInternalState.h>
#include <Core/ListenerInternalState.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
#define AM_EXPECT(_condition_) Expect(_condition_, #_condition_, __FILE__, __LINE__)
#define AM_EXPECT_NOT(_condition_) AM_EXPECT(!(_condition_))
#define AM_EXPECT_EQ(_expected_, _actual_) AM_EXPECT((_expected_) == (_actual_))
#define AM_EXPECT_NE(_expected_, _actual_) AM_EXPECT((_expected_) != (_actual_))

    Listener InitTestListener(ListenerInternalState* state);

    Entity InitTestEntity(EntityInternalState* state);
} // namespace SparkyStudios::Audio::Amplitude::Tests
