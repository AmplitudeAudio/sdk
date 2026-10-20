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

#ifndef AM_PLUGINS_UNSUPPORTED

#if AM_PLATFORM_WIN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "LightEngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmTime kFrameDelta = 0.016;

        using HookStateFn = void (*)(AmUInt64*, AmUInt64*, AmUInt64*, AmUInt64*);

        HookStateFn FindHookState(AmVoidPtr handle)
        {
#if AM_PLATFORM_WIN
            return reinterpret_cast<HookStateFn>(GetProcAddress(static_cast<HMODULE>(handle), "TestPluginHookState"));
#else
            return reinterpret_cast<HookStateFn>(dlsym(handle, "TestPluginHookState"));
#endif
        }
    } // namespace

    // Plugins can only be loaded before the engine is initialized, so these tests load them first and drive the frames by hand.
    AM_TEST_CASE(LightEngineTestCase, core_engine, plugin_frame_hooks_run_every_frame_in_order)
    {
    public:
        void Run() override
        {
            Engine::AddPluginSearchPath(AM_OS_STRING(AM_TEST_PLUGIN_DIR));
            const AmVoidPtr handle = Engine::LoadPlugin(AM_OS_STRING("test_plugin"));
            AM_EXPECT(handle != nullptr);

            const HookStateFn state = FindHookState(handle);
            AM_EXPECT(state != nullptr);
            if (state == nullptr)
                return;

            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));

            // The plugin library may stay mapped between tests, so count from where it stands now.
            AmUInt64 beforeStart = 0, afterStart = 0, before = 0, after = 0, lastBefore = 0, lastAfter = 0;
            state(&beforeStart, &afterStart, &lastBefore, &lastAfter);

            constexpr int kFrames = 5;
            for (int i = 0; i < kFrames; ++i)
            {
                amEngine->AdvanceFrame(kFrameDelta);

                AmUInt64 lastBeforeSoFar = 0, lastAfterSoFar = 0, ignoredBefore = 0, ignoredAfter = 0;
                state(&ignoredBefore, &ignoredAfter, &lastBeforeSoFar, &lastAfterSoFar);

                // Within a frame the before-hook runs first.
                AM_EXPECT(lastBeforeSoFar < lastAfterSoFar);
            }

            state(&before, &after, &lastBefore, &lastAfter);
            AM_EXPECT(before == beforeStart + kFrames);
            AM_EXPECT(after == afterStart + kFrames);
        }
    };

    AM_REGISTER_TEST_REQUIRES_PLUGINS(core_engine, plugin_frame_hooks_run_every_frame_in_order);

    AM_TEST_CASE(LightEngineTestCase, core_engine, plugin_without_frame_hooks_still_loads_and_frames_advance)
    {
    public:
        void Run() override
        {
            Engine::AddPluginSearchPath(AM_OS_STRING(AM_TEST_PLUGIN_DIR));
            AM_EXPECT(Engine::LoadPlugin(AM_OS_STRING("test_plugin_no_hooks")) != nullptr);

            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));

            for (int i = 0; i < 3; ++i)
                amEngine->AdvanceFrame(kFrameDelta);

            AM_EXPECT(amEngine->IsInitialized());
        }
    };

    AM_REGISTER_TEST_REQUIRES_PLUGINS(core_engine, plugin_without_frame_hooks_still_loads_and_frames_advance);

    AM_TEST_CASE(LightEngineTestCase, core_engine, cannot_load_plugin_after_initialization)
    {
    public:
        void Run() override
        {
            Engine::AddPluginSearchPath(AM_OS_STRING(AM_TEST_PLUGIN_DIR));
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
            AM_EXPECT(amEngine->IsInitialized());

            // Loading a plugin while the engine runs is refused, so the frame hooks never change under a running frame.
            AM_EXPECT(Engine::LoadPlugin(AM_OS_STRING("test_plugin")) == nullptr);
        }
    };

    AM_REGISTER_TEST_REQUIRES_PLUGINS(core_engine, cannot_load_plugin_after_initialization);
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif
