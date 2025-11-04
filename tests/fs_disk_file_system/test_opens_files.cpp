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
    DiskFileSystem fileSystem;
    fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

    const auto& cp = std::filesystem::current_path() / AM_OS_STRING("samples/assets");
    AM_EXPECT(cp == fileSystem.GetBasePath());

    AM_EXPECT(fileSystem.OpenFile(AM_OS_STRING("tests.config.amconfig"), eFileOpenMode_Read)->IsValid());
    AM_EXPECT_NOT(fileSystem.OpenFile(AM_OS_STRING("some_random_file.ext"), eFileOpenMode_Read)->IsValid());
}
