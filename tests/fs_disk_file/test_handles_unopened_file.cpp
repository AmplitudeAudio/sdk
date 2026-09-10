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
    AM_TEST_CASE(ComponentTestCase, fs_disk_file, handles_unopened_file)
    {
    public:
        void Run() override
        {
            DiskFile file;
            AM_EXPECT(!file.IsValid());
            AM_EXPECT(file.GetPtr() == nullptr);
            AM_EXPECT(file.Length() == 0);
            AM_EXPECT(file.Position() == 0);
            AM_EXPECT(file.Eof());

            AmUInt8 buffer[16];
            AM_EXPECT(file.Read(buffer, sizeof(buffer)) == 0);
            AM_EXPECT(file.Write(buffer, sizeof(buffer)) == 0);

            // Seek should safely no-op without crashing
            file.Seek(10, eFileSeekOrigin_Start);
            AM_EXPECT(file.Position() == 0);

            file.Close();
            AM_EXPECT(!file.IsValid());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fs_disk_file, handles_unopened_file);
} // namespace SparkyStudios::Audio::Amplitude::Tests
