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

        struct Results
        {
            const PluginHost* host = nullptr;
            bool nullHost = true;
            bool nullCallback = true;
            bool invalidStage = true;
            bool first = false;
            bool duplicate = true;
            bool sameCallbackOtherCookie = false;
            bool sameCallbackOtherStage = false;
            bool unregister = false;
            bool unregisterAgain = true;
        };

        Results gResults;
        int gCookieA = 0;
        int gCookieB = 0;

        bool Register(const PluginHost* host)
        {
            gResults.host = host;
            gResults.nullHost = host->RegisterFrameCallback(nullptr, ePluginFrameStage_BeforeUpdate, Callback, nullptr);
            gResults.nullCallback = host->RegisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, nullptr, nullptr);
            gResults.invalidStage = host->RegisterFrameCallback(host, ePluginFrameStage_Count, Callback, nullptr);
            gResults.first = host->RegisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, &gCookieA);
            gResults.duplicate = host->RegisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, &gCookieA);
            gResults.sameCallbackOtherCookie = host->RegisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, &gCookieB);
            gResults.sameCallbackOtherStage = host->RegisterFrameCallback(host, ePluginFrameStage_AfterUpdate, Callback, &gCookieA);
            gResults.unregister = host->UnregisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, &gCookieB);
            gResults.unregisterAgain = host->UnregisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, &gCookieB);
            return true;
        }

        bool Unregister(const PluginHost*)
        {
            return true;
        }

        const PluginDescriptor kDescriptor = {
            .abiVersion = AM_PLUGIN_ABI_VERSION,
            .structSize = sizeof(PluginDescriptor),
            .name = "Frame Callback Rules",
            .version = "1.0.0",
            .Register = Register,
            .Unregister = Unregister,
        };
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, core_plugin, plugin_host_frame_callback_rules)
    {
    public:
        void Run() override
        {
            gResults = Results{};

            AM_EXPECT(Engine::RegisterPlugin(&kDescriptor));

            AM_EXPECT(!gResults.nullHost);
            AM_EXPECT(!gResults.nullCallback);
            AM_EXPECT(!gResults.invalidStage);
            AM_EXPECT(gResults.first);
            AM_EXPECT(!gResults.duplicate);
            AM_EXPECT(gResults.sameCallbackOtherCookie);
            AM_EXPECT(gResults.sameCallbackOtherStage);
            AM_EXPECT(gResults.unregister);
            AM_EXPECT(!gResults.unregisterAgain);

            // The frame reads the callbacks without locking, so they cannot change once the engine runs.
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
            AM_EXPECT(amEngine->IsInitialized());

            const PluginHost* host = gResults.host;
            AM_EXPECT(host != nullptr);
            if (host == nullptr)
                return;

            AM_EXPECT(!host->RegisterFrameCallback(host, ePluginFrameStage_AfterUpdate, Callback, &gCookieB));
            AM_EXPECT(!host->UnregisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, Callback, &gCookieA));
        }
    };

    AM_REGISTER_TEST(core_plugin, plugin_host_frame_callback_rules);
} // namespace SparkyStudios::Audio::Amplitude::Tests
