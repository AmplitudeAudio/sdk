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
using namespace SparkyStudios::Audio::Amplitude::Tests;

namespace
{
    void OnBeforeUpdate(AmTime, AmVoidPtr cookie)
    {
        auto& state = StaticTestPlugin::gState;
        state.lastBeforeSequence = ++state.sequence;
        state.lastBeforeCookie = cookie;
        ++state.beforeCalls;
    }

    void OnAfterUpdate(AmTime, AmVoidPtr cookie)
    {
        auto& state = StaticTestPlugin::gState;
        state.lastAfterSequence = ++state.sequence;
        state.lastAfterCookie = cookie;
        ++state.afterCalls;
    }

    bool RegisterStaticTestPlugin(const PluginHost* host)
    {
        auto& state = StaticTestPlugin::gState;
        ++state.registerCalls;

        state.host = host;
        state.hostAbiVersion = host->abiVersion;
        state.hostStructSize = host->structSize;
        state.hostEngine = host->engine;
        state.hostMemory = host->memory;

        if (state.failRegister)
            return false;

        if (state.registerFrameCallbacks)
        {
            host->RegisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, OnBeforeUpdate, &StaticTestPlugin::gBeforeCookie);
            host->RegisterFrameCallback(host, ePluginFrameStage_AfterUpdate, OnAfterUpdate, &StaticTestPlugin::gAfterCookie);
        }

        return true;
    }

    bool UnregisterStaticTestPlugin(const PluginHost*)
    {
        ++StaticTestPlugin::gState.unregisterCalls;
        return true;
    }

    const PluginDescriptor kStaticTestPluginDescriptor = {
        .abiVersion = AM_PLUGIN_ABI_VERSION,
        .structSize = sizeof(PluginDescriptor),
        .name = StaticTestPlugin::kName,
        .version = "1.0.0",
        .description = "The plugin linked statically into the test runner.",
        .author = "Sparky Studios",
        .copyright = "Copyright (c) 2026-present Sparky Studios. All rights Reserved.",
        .license = "Apache License, Version 2.0",
        .Register = RegisterStaticTestPlugin,
        .Unregister = UnregisterStaticTestPlugin,
    };
} // namespace

// What a statically linked plugin writes, and what the game writes to keep it.
AM_DEFINE_STATIC_PLUGIN(StaticTestPlugin, kStaticTestPluginDescriptor)
AM_STATIC_LINK_PLUGIN(StaticTestPlugin)

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, core_plugin, static_plugins_are_registered)
    {
    public:
        void Run() override
        {
            StaticTestPlugin::Reset();

            bool linked = false;
            for (auto* it = StaticPluginRegistration::GetFirst(); it != nullptr; it = it->GetNext())
                linked |= it->GetDescriptor() == &kStaticTestPluginDescriptor;

            AM_EXPECT(linked);

            AM_EXPECT(!Engine::IsPluginRegistered(StaticTestPlugin::kName));
            AM_EXPECT(Engine::RegisterStaticPlugins());
            AM_EXPECT(Engine::IsPluginRegistered(StaticTestPlugin::kName));

            const auto& state = StaticTestPlugin::gState;
            AM_EXPECT(state.registerCalls == 1);
            AM_EXPECT(state.host != nullptr);
            AM_EXPECT(state.hostAbiVersion == AM_PLUGIN_ABI_VERSION);
            AM_EXPECT(state.hostStructSize == sizeof(PluginHost));
            AM_EXPECT(state.hostEngine == amEngine);
            AM_EXPECT(state.hostMemory != nullptr);

            // Plugins already registered are skipped.
            AM_EXPECT(Engine::RegisterStaticPlugins());
            AM_EXPECT(state.registerCalls == 1);
            AM_EXPECT(state.unregisterCalls == 0);
        }
    };

    AM_REGISTER_TEST(core_plugin, static_plugins_are_registered);
} // namespace SparkyStudios::Audio::Amplitude::Tests
