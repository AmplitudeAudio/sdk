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

#include "TestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void TestCase::SetUp()
{}

void TestCase::TearDown()
{}

void TestCase::Run()
{
    const auto& version = amVersion;
    ExpectEqual<AmUInt8>(version.major, AM_VERSION_MAJOR, "Major version mismatch");
}