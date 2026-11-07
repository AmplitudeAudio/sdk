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

#include <stdexcept>

#include "LightEngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        int gOrder = 0;
        int gThrowingUnregisterAt = 0;
        int gFailingUnregisterAt = 0;
        int gHealthyUnregisterAt = 0;

        bool RegisterOk(const PluginHost*)
        {
            return true;
        }

        bool RegisterThrowsStdException(const PluginHost*)
        {
            throw std::runtime_error("register failed");
        }

        bool RegisterThrowsAnything(const PluginHost*)
        {
            throw 42;
        }

        bool UnregisterOk(const PluginHost*)
        {
            return true;
        }

        bool UnregisterThrows(const PluginHost*)
        {
            gThrowingUnregisterAt = ++gOrder;
            throw std::runtime_error("unregister failed");
        }

        bool UnregisterFails(const PluginHost*)
        {
            gFailingUnregisterAt = ++gOrder;
            return false;
        }

        bool UnregisterHealthy(const PluginHost*)
        {
            gHealthyUnregisterAt = ++gOrder;
            return true;
        }

        PluginDescriptor Descriptor(const char* name, bool (*reg)(const PluginHost*), bool (*unreg)(const PluginHost*))
        {
            return PluginDescriptor{
                .abiVersion = AM_PLUGIN_ABI_VERSION,
                .structSize = sizeof(PluginDescriptor),
                .name = name,
                .version = "1.0.0",
                .Register = reg,
                .Unregister = unreg,
            };
        }
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, core_plugin, plugin_exceptions_and_failures_are_contained)
    {
    public:
        void Run() override
        {
            gOrder = gThrowingUnregisterAt = gFailingUnregisterAt = gHealthyUnregisterAt = 0;

            // A Register that throws counts as a failure; nothing escapes into the engine.
            static const PluginDescriptor kThrowsStd = Descriptor("Throws Std", RegisterThrowsStdException, UnregisterOk);
            static const PluginDescriptor kThrowsAny = Descriptor("Throws Anything", RegisterThrowsAnything, UnregisterOk);
            AM_EXPECT(!Engine::RegisterPlugin(&kThrowsStd));
            AM_EXPECT(!Engine::RegisterPlugin(&kThrowsAny));
            AM_EXPECT(!Engine::IsPluginRegistered(kThrowsStd.name));
            AM_EXPECT(!Engine::IsPluginRegistered(kThrowsAny.name));

            // An Unregister that throws or fails is reported, and the other plugins are still unregistered.
            static const PluginDescriptor kHealthy = Descriptor("Healthy", RegisterOk, UnregisterHealthy);
            static const PluginDescriptor kFailing = Descriptor("Failing Unregister", RegisterOk, UnregisterFails);
            static const PluginDescriptor kThrowing = Descriptor("Throwing Unregister", RegisterOk, UnregisterThrows);
            AM_EXPECT(Engine::RegisterPlugin(&kHealthy));
            AM_EXPECT(Engine::RegisterPlugin(&kFailing));
            AM_EXPECT(Engine::RegisterPlugin(&kThrowing));

            amEngine->DestroyInstance();

            AM_EXPECT(gThrowingUnregisterAt == 1);
            AM_EXPECT(gFailingUnregisterAt == 2);
            AM_EXPECT(gHealthyUnregisterAt == 3);
            AM_EXPECT(!Engine::IsPluginRegistered(kHealthy.name));
            AM_EXPECT(!Engine::IsPluginRegistered(kFailing.name));
            AM_EXPECT(!Engine::IsPluginRegistered(kThrowing.name));
        }
    };

    AM_REGISTER_TEST(core_plugin, plugin_exceptions_and_failures_are_contained);
} // namespace SparkyStudios::Audio::Amplitude::Tests
