// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        // Test loading a valid plugin
        AmVoidPtr handle = Engine::LoadPlugin(AM_OS_STRING("test_plugin"));
        AM_EXPECT(handle != nullptr);

        // Test loading a non-existent plugin (should return nullptr)
        AmVoidPtr invalidHandle = Engine::LoadPlugin(AM_OS_STRING("non_existent_plugin"));
        AM_EXPECT(invalidHandle == nullptr);

        // Test loading with empty plugin name (should return nullptr)
        AmVoidPtr emptyHandle = Engine::LoadPlugin(AM_OS_STRING(""));
        AM_EXPECT(emptyHandle == nullptr);

        // Test plugin search paths functionality
        Engine::AddPluginSearchPath(AM_OS_STRING("test_plugins"));

        // Try loading the test plugin again (should still work)
        AmVoidPtr handleWithPath = Engine::LoadPlugin(AM_OS_STRING("test_plugin"));
        AM_EXPECT(handleWithPath != nullptr);

        // Clean up - remove the search path
        Engine::RemovePluginSearchPath(AM_OS_STRING("test_plugins"));

        // Test removing non-existent search path (should not crash)
        Engine::RemovePluginSearchPath(AM_OS_STRING("non_existent_path"));

        // Test adding empty search path (should not crash)
        Engine::AddPluginSearchPath(AM_OS_STRING(""));
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
