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

#include "PureUnitTestCase.h"
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
            .name = "Too Early",
            .version = "1.0.0",
            .Register = Register,
            .Unregister = Unregister,
        };
    } // namespace

    // Without a memory manager there is no engine instance to give the plugin.
    AM_TEST_CASE(PureUnitTestCase, core_plugin, register_plugin_requires_memory_manager)
    {
    public:
        void Run() override
        {
            gRegisterCalls = 0;

            AM_EXPECT(!MemoryManager::IsInitialized());
            AM_EXPECT(!Engine::RegisterPlugin(&kDescriptor));
            AM_EXPECT(gRegisterCalls == 0);
            AM_EXPECT(!Engine::IsPluginRegistered(kDescriptor.name));
        }
    };

    AM_REGISTER_TEST(core_plugin, register_plugin_requires_memory_manager);
} // namespace SparkyStudios::Audio::Amplitude::Tests
