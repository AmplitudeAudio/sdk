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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, fs_package_file_system, can_be_used_when_initialized)
    {
    public:
        void Run() override
        {
            // Uncompressed file
            {
                PackageFileSystem filesystem;
                filesystem.SetPlatformFileSystem<DiskFileSystem>();
                filesystem.SetBasePath(AM_OS_STRING("./samples/assets_uncompressed.ampk"));

                filesystem.StartOpenFileSystem();
                while (!filesystem.TryFinalizeOpenFileSystem())
                    Thread::Sleep(1);

                // can sets the base path
                {
                    AM_EXPECT(
                        filesystem.GetBasePath() == std::filesystem::current_path() / AM_OS_STRING("samples/assets_uncompressed.ampk"));
                }

                // can check if files exists
                {
                    AM_EXPECT(filesystem.Exists(AM_OS_STRING("tests.config.amconfig")));
                    AM_EXPECT_NOT(filesystem.Exists(AM_OS_STRING("some_random_file.ext")));
                }

                // can open files
                {
                    AM_EXPECT(filesystem.OpenFile(AM_OS_STRING("tests.config.amconfig"))->IsValid());
                    AM_EXPECT(filesystem.OpenFile(AM_OS_STRING("some_random_file.ext")) == nullptr);
                }

                // can reopen already opened filesystem
                {
                    AM_EXPECT(filesystem.TryFinalizeOpenFileSystem());

                    filesystem.StartOpenFileSystem();
                    while (!filesystem.TryFinalizeOpenFileSystem())
                        Thread::Sleep(1);

                    AM_EXPECT(filesystem.TryFinalizeOpenFileSystem());
                }

                // can close filesystem
                {
                    filesystem.StartCloseFileSystem();
                    AM_EXPECT(filesystem.TryFinalizeCloseFileSystem());
                }
            }

            // Compressed file
            {
                PackageFileSystem filesystem;
                filesystem.SetPlatformFileSystem<DiskFileSystem>();
                filesystem.SetBasePath(AM_OS_STRING("./samples/assets_compressed.ampk"));

                filesystem.StartOpenFileSystem();
                while (!filesystem.TryFinalizeOpenFileSystem())
                    Thread::Sleep(1);

                // can sets the base path
                {
                    AM_EXPECT(filesystem.GetBasePath() == std::filesystem::current_path() / AM_OS_STRING("samples/assets_compressed.ampk"));
                }

                // can check if files exists
                {
                    AM_EXPECT(filesystem.Exists(AM_OS_STRING("tests.config.amconfig")));
                    AM_EXPECT_NOT(filesystem.Exists(AM_OS_STRING("some_random_file.ext")));
                }

                // can open files
                {
                    AM_EXPECT(filesystem.OpenFile(AM_OS_STRING("tests.config.amconfig"))->IsValid());
                    AM_EXPECT(filesystem.OpenFile(AM_OS_STRING("some_random_file.ext")) == nullptr);
                }

                // can reopen already opened filesystem
                {
                    AM_EXPECT(filesystem.TryFinalizeOpenFileSystem());

                    filesystem.StartOpenFileSystem();
                    while (!filesystem.TryFinalizeOpenFileSystem())
                        Thread::Sleep(1);

                    AM_EXPECT(filesystem.TryFinalizeOpenFileSystem());
                }

                // can close filesystem
                {
                    filesystem.StartCloseFileSystem();
                    AM_EXPECT(filesystem.TryFinalizeCloseFileSystem());
                }
            }
        }
    };

    // PackageFileSystem tests use DiskFileSystem paths that only work on desktop
    AM_REGISTER_TEST_DESKTOP_ONLY(fs_package_file_system, can_be_used_when_initialized);
} // namespace SparkyStudios::Audio::Amplitude::Tests
