// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include "LightEngineTestCase.h"
#include "StaticTestPlugin.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, core_plugin, plugin_frame_callbacks_run_every_frame_in_order)
    {
    public:
        void Run() override
        {
            constexpr AmTime kFrameDelta = 0.016;
            constexpr int kFrames = 5;

            StaticTestPlugin::Reset();
            StaticTestPlugin::gState.registerFrameCallbacks = true;

            AM_EXPECT(Engine::RegisterStaticPlugins());
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
            AM_EXPECT(amEngine->IsInitialized());

            const auto& state = StaticTestPlugin::gState;
            for (int i = 0; i < kFrames; ++i)
            {
                amEngine->AdvanceFrame(kFrameDelta);

                // Within a frame the before-update callback runs first.
                AM_EXPECT(state.lastBeforeSequence < state.lastAfterSequence);
            }

            AM_EXPECT(state.beforeCalls == kFrames);
            AM_EXPECT(state.afterCalls == kFrames);
            AM_EXPECT(state.lastBeforeCookie == &StaticTestPlugin::gBeforeCookie);
            AM_EXPECT(state.lastAfterCookie == &StaticTestPlugin::gAfterCookie);
        }
    };

    AM_REGISTER_TEST(core_plugin, plugin_frame_callbacks_run_every_frame_in_order);

    // The callbacks belong to the plugin: once it is unregistered, a new engine instance never calls them.
    AM_TEST_CASE(LightEngineTestCase, core_plugin, plugin_frame_callbacks_are_removed_with_the_plugin)
    {
    public:
        void Run() override
        {
            StaticTestPlugin::Reset();
            StaticTestPlugin::gState.registerFrameCallbacks = true;

            AM_EXPECT(Engine::RegisterStaticPlugins());
            amEngine->DestroyInstance();
            AM_EXPECT(StaticTestPlugin::gState.unregisterCalls == 1);

            // A fresh instance, set up like the fixture does, without the plugin.
            amEngine->SetFileSystem(_fileSystem);
            amEngine->StartOpenFileSystem();
            while (!amEngine->TryFinalizeOpenFileSystem())
                Thread::Sleep(1);

            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
            AM_EXPECT(amEngine->IsInitialized());

            amEngine->AdvanceFrame(0.016);

            AM_EXPECT(StaticTestPlugin::gState.beforeCalls == 0);
            AM_EXPECT(StaticTestPlugin::gState.afterCalls == 0);
        }
    };

    AM_REGISTER_TEST(core_plugin, plugin_frame_callbacks_are_removed_with_the_plugin);
} // namespace SparkyStudios::Audio::Amplitude::Tests
