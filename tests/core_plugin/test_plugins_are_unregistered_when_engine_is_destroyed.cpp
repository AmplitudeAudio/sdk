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
    namespace
    {
        int gOrder = 0;
        int gFirstUnregisteredAt = 0;
        int gSecondUnregisteredAt = 0;

        bool RegisterOk(const PluginHost*)
        {
            return true;
        }

        bool UnregisterFirst(const PluginHost*)
        {
            gFirstUnregisteredAt = ++gOrder;
            return true;
        }

        bool UnregisterSecond(const PluginHost*)
        {
            gSecondUnregisteredAt = ++gOrder;
            return true;
        }

        const PluginDescriptor kFirst = {
            .abiVersion = AM_PLUGIN_ABI_VERSION,
            .structSize = sizeof(PluginDescriptor),
            .name = "Unregister Order First",
            .version = "1.0.0",
            .Register = RegisterOk,
            .Unregister = UnregisterFirst,
        };

        const PluginDescriptor kSecond = {
            .abiVersion = AM_PLUGIN_ABI_VERSION,
            .structSize = sizeof(PluginDescriptor),
            .name = "Unregister Order Second",
            .version = "1.0.0",
            .Register = RegisterOk,
            .Unregister = UnregisterSecond,
        };
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, core_plugin, plugins_are_unregistered_in_reverse_order_when_engine_is_destroyed)
    {
    public:
        void Run() override
        {
            StaticTestPlugin::Reset();
            gOrder = gFirstUnregisteredAt = gSecondUnregisteredAt = 0;

            AM_EXPECT(Engine::RegisterPlugin(&kFirst));
            AM_EXPECT(Engine::RegisterPlugin(&kSecond));
            AM_EXPECT(Engine::RegisterStaticPlugins());

            amEngine->DestroyInstance();

            AM_EXPECT(!Engine::IsPluginRegistered(kFirst.name));
            AM_EXPECT(!Engine::IsPluginRegistered(kSecond.name));
            AM_EXPECT(!Engine::IsPluginRegistered(StaticTestPlugin::kName));
            AM_EXPECT(StaticTestPlugin::gState.unregisterCalls == 1);

            // The static plugin was registered last, so it was unregistered first.
            AM_EXPECT(gSecondUnregisteredAt == 1);
            AM_EXPECT(gFirstUnregisteredAt == 2);
        }
    };

    AM_REGISTER_TEST(core_plugin, plugins_are_unregistered_in_reverse_order_when_engine_is_destroyed);
} // namespace SparkyStudios::Audio::Amplitude::Tests
