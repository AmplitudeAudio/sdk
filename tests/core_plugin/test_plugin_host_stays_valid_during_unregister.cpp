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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        void Callback(AmTime, AmVoidPtr)
        {}

        bool gUnregisterCalled = false;
        bool gUnregisteredCallback = false;
        Engine* gEngineDuringUnregister = nullptr;

        bool Register(const PluginHost* host)
        {
            return host->RegisterFrameCallback(host, ePluginFrameStage_AfterUpdate, Callback, nullptr);
        }

        // What a plugin typically does on its way out: remove its callbacks and reach the engine.
        bool Unregister(const PluginHost* host)
        {
            gUnregisterCalled = true;
            gUnregisteredCallback = host->UnregisterFrameCallback(host, ePluginFrameStage_AfterUpdate, Callback, nullptr);
            gEngineDuringUnregister = Engine::GetInstance();
            return true;
        }

        const PluginDescriptor kDescriptor = {
            .abiVersion = AM_PLUGIN_ABI_VERSION,
            .structSize = sizeof(PluginDescriptor),
            .name = "Host During Unregister",
            .version = "1.0.0",
            .Register = Register,
            .Unregister = Unregister,
        };
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, core_plugin, plugin_host_stays_valid_during_unregister)
    {
    public:
        void Run() override
        {
            gUnregisterCalled = gUnregisteredCallback = false;
            gEngineDuringUnregister = nullptr;

            AM_EXPECT(Engine::RegisterPlugin(&kDescriptor));

            Engine* engine = amEngine;
            amEngine->DestroyInstance();

            AM_EXPECT(gUnregisterCalled);
            AM_EXPECT(gUnregisteredCallback);

            // The plugin still sees the instance being destroyed, not a new one, and reaching it does not deadlock.
            AM_EXPECT(gEngineDuringUnregister == engine);
        }
    };

    AM_REGISTER_TEST(core_plugin, plugin_host_stays_valid_during_unregister);
} // namespace SparkyStudios::Audio::Amplitude::Tests
