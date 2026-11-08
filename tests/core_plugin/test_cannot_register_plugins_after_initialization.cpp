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
        int gRegisterCalls = 0;

        bool Register(const PluginHost*)
        {
            ++gRegisterCalls;
            return true;
        }

        bool Unregister(const PluginHost*)
        {
            return true;
        }

        const PluginDescriptor kDescriptor = {
            .abiVersion = AM_PLUGIN_ABI_VERSION,
            .structSize = sizeof(PluginDescriptor),
            .name = "Late Plugin",
            .version = "1.0.0",
            .Register = Register,
            .Unregister = Unregister,
        };
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, core_plugin, cannot_register_plugins_after_initialization)
    {
    public:
        void Run() override
        {
            gRegisterCalls = 0;
            StaticTestPlugin::Reset();

            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
            AM_EXPECT(amEngine->IsInitialized());

            // Plugins register nodes, callbacks and other state the running engine reads without locking.
            AM_EXPECT(!Engine::RegisterPlugin(&kDescriptor));
            AM_EXPECT(!Engine::RegisterStaticPlugins());

            AM_EXPECT(gRegisterCalls == 0);
            AM_EXPECT(StaticTestPlugin::gState.registerCalls == 0);
            AM_EXPECT(!Engine::IsPluginRegistered(kDescriptor.name));
        }
    };

    AM_REGISTER_TEST(core_plugin, cannot_register_plugins_after_initialization);
} // namespace SparkyStudios::Audio::Amplitude::Tests
