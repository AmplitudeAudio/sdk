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

        const PluginHost* gOwnerHost = nullptr;

        bool RegisterOwner(const PluginHost* host)
        {
            gOwnerHost = host;
            return host->RegisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, nullptr);
        }

        struct Results
        {
            bool forgedRegister = true;
            bool forgedUnregister = true;
            bool nullHostUnregister = true;
            bool invalidStageUnregister = true;
            bool otherPluginsCallback = true;
        };

        Results gResults;

        // Another plugin, trying the host functions with hosts that are not its own.
        bool RegisterIntruder(const PluginHost* host)
        {
            // A copy is not the host the engine gave: same content, different address.
            const PluginHost forged = *host;
            gResults.forgedRegister = host->RegisterFrameCallback(&forged, ePluginFrameStage_BeforeUpdate, Callback, nullptr);
            gResults.forgedUnregister = host->UnregisterFrameCallback(&forged, ePluginFrameStage_BeforeUpdate, Callback, nullptr);
            gResults.nullHostUnregister = host->UnregisterFrameCallback(nullptr, ePluginFrameStage_BeforeUpdate, Callback, nullptr);
            gResults.invalidStageUnregister = host->UnregisterFrameCallback(host, ePluginFrameStage_Count, Callback, nullptr);

            // The owner's callback cannot be removed through another plugin's host.
            gResults.otherPluginsCallback = host->UnregisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, nullptr);
            return true;
        }

        bool Unregister(const PluginHost*)
        {
            return true;
        }

        const PluginDescriptor kOwner = {
            .abiVersion = AM_PLUGIN_ABI_VERSION,
            .structSize = sizeof(PluginDescriptor),
            .name = "Host Owner",
            .version = "1.0.0",
            .Register = RegisterOwner,
            .Unregister = Unregister,
        };

        const PluginDescriptor kIntruder = {
            .abiVersion = AM_PLUGIN_ABI_VERSION,
            .structSize = sizeof(PluginDescriptor),
            .name = "Host Intruder",
            .version = "1.0.0",
            .Register = RegisterIntruder,
            .Unregister = Unregister,
        };
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, core_plugin, plugin_host_belongs_to_its_plugin)
    {
    public:
        void Run() override
        {
            gResults = Results{};
            gOwnerHost = nullptr;

            AM_EXPECT(Engine::RegisterPlugin(&kOwner));
            AM_EXPECT(Engine::RegisterPlugin(&kIntruder));

            AM_EXPECT(!gResults.forgedRegister);
            AM_EXPECT(!gResults.forgedUnregister);
            AM_EXPECT(!gResults.nullHostUnregister);
            AM_EXPECT(!gResults.invalidStageUnregister);
            AM_EXPECT(!gResults.otherPluginsCallback);

            // The owner still holds its callback.
            AM_EXPECT(gOwnerHost != nullptr);
            if (gOwnerHost != nullptr)
                AM_EXPECT(gOwnerHost->UnregisterFrameCallback(gOwnerHost, ePluginFrameStage_BeforeUpdate, Callback, nullptr));
        }
    };

    AM_REGISTER_TEST(core_plugin, plugin_host_belongs_to_its_plugin);
} // namespace SparkyStudios::Audio::Amplitude::Tests
