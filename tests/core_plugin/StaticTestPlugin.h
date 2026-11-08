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

#pragma once

#ifndef _AM_TESTS_CORE_PLUGIN_STATIC_TEST_PLUGIN_H
#define _AM_TESTS_CORE_PLUGIN_STATIC_TEST_PLUGIN_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

// The plugin linked statically into the test runner, defined in test_static_plugins_are_registered.cpp. Its state lives here so
// every test can drive and inspect it; each test resets it first.
namespace SparkyStudios::Audio::Amplitude::Tests::StaticTestPlugin
{
    inline constexpr const char* kName = "Static Test Plugin";

    struct State
    {
        // What the plugin does when registered.
        bool failRegister = false;
        bool registerFrameCallbacks = false;

        // What happened to it.
        int registerCalls = 0;
        int unregisterCalls = 0;
        const PluginHost* host = nullptr;
        AmUInt32 hostAbiVersion = 0;
        AmUInt32 hostStructSize = 0;
        Engine* hostEngine = nullptr;
        MemoryManager* hostMemory = nullptr;

        // Frame callbacks: when each one ran last, and with which cookie.
        AmUInt64 sequence = 0;
        AmUInt64 beforeCalls = 0;
        AmUInt64 afterCalls = 0;
        AmUInt64 lastBeforeSequence = 0;
        AmUInt64 lastAfterSequence = 0;
        AmVoidPtr lastBeforeCookie = nullptr;
        AmVoidPtr lastAfterCookie = nullptr;
    };

    inline State gState;

    // The cookies the plugin registers its frame callbacks with.
    inline int gBeforeCookie = 0;
    inline int gAfterCookie = 0;

    inline void Reset()
    {
        gState = State{};
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests::StaticTestPlugin

#endif // _AM_TESTS_CORE_PLUGIN_STATIC_TEST_PLUGIN_H
