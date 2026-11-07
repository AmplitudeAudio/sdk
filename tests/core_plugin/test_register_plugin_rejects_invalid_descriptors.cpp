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
        int gRegisterCalls = 0;
        int gUnregisterCalls = 0;

        bool CountingRegister(const PluginHost*)
        {
            ++gRegisterCalls;
            return true;
        }

        bool FailingRegister(const PluginHost*)
        {
            ++gRegisterCalls;
            return false;
        }

        bool CountingUnregister(const PluginHost*)
        {
            ++gUnregisterCalls;
            return true;
        }

        PluginDescriptor ValidDescriptor(const char* name)
        {
            return PluginDescriptor{
                .abiVersion = AM_PLUGIN_ABI_VERSION,
                .structSize = sizeof(PluginDescriptor),
                .name = name,
                .version = "1.0.0",
                .Register = CountingRegister,
                .Unregister = CountingUnregister,
            };
        }

        PluginDescriptor With(PluginDescriptor descriptor, void (*change)(PluginDescriptor&))
        {
            change(descriptor);
            return descriptor;
        }
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, core_plugin, register_plugin_rejects_invalid_descriptors)
    {
    public:
        void Run() override
        {
            gRegisterCalls = gUnregisterCalls = 0;

            AM_EXPECT(!Engine::RegisterPlugin(nullptr));

            // Each descriptor is invalid in one way; none of them reaches its Register function.
            static const PluginDescriptor kInvalid[] = {
                With(ValidDescriptor("Wrong ABI"), [](PluginDescriptor& d) { d.abiVersion = AM_PLUGIN_ABI_VERSION + 1; }),
                With(ValidDescriptor("Zero ABI"), [](PluginDescriptor& d) { d.abiVersion = 0; }),
                With(ValidDescriptor("Wrong Size"), [](PluginDescriptor& d) { d.structSize = sizeof(PluginDescriptor) - 1; }),
                With(ValidDescriptor("No Name"), [](PluginDescriptor& d) { d.name = nullptr; }),
                With(ValidDescriptor(""), [](PluginDescriptor&) {}),
                With(ValidDescriptor("No Version"), [](PluginDescriptor& d) { d.version = nullptr; }),
                With(ValidDescriptor("No Register"), [](PluginDescriptor& d) { d.Register = nullptr; }),
                With(ValidDescriptor("No Unregister"), [](PluginDescriptor& d) { d.Unregister = nullptr; }),
            };

            for (const auto& descriptor : kInvalid)
                AM_EXPECT(!Engine::RegisterPlugin(&descriptor));

            AM_EXPECT(gRegisterCalls == 0);
            AM_EXPECT(!Engine::IsPluginRegistered("Wrong ABI"));

            // A plugin whose Register fails is not kept, and not unregistered.
            static const PluginDescriptor kFailing =
                With(ValidDescriptor("Failing"), [](PluginDescriptor& d) { d.Register = FailingRegister; });
            AM_EXPECT(!Engine::RegisterPlugin(&kFailing));
            AM_EXPECT(gRegisterCalls == 1);
            AM_EXPECT(!Engine::IsPluginRegistered("Failing"));

            // Names are unique: a second plugin with the same name is refused without being called.
            static const PluginDescriptor kFirst = ValidDescriptor("Duplicate");
            static const PluginDescriptor kSecond = ValidDescriptor("Duplicate");
            AM_EXPECT(Engine::RegisterPlugin(&kFirst));
            AM_EXPECT(gRegisterCalls == 2);
            AM_EXPECT(!Engine::RegisterPlugin(&kSecond));
            AM_EXPECT(!Engine::RegisterPlugin(&kFirst));
            AM_EXPECT(gRegisterCalls == 2);

            amEngine->DestroyInstance();
            AM_EXPECT(gUnregisterCalls == 1);
        }
    };

    AM_REGISTER_TEST(core_plugin, register_plugin_rejects_invalid_descriptors);
} // namespace SparkyStudios::Audio::Amplitude::Tests
