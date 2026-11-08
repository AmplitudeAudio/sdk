// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#include <atomic>

using namespace SparkyStudios::Audio::Amplitude;

namespace
{
    // Records when each frame callback ran, so the tests can check the call order against the frame.
    std::atomic<AmUInt64> gSequence{ 0 };
    std::atomic<AmUInt64> gBeforeCalls{ 0 };
    std::atomic<AmUInt64> gAfterCalls{ 0 };
    std::atomic<AmUInt64> gLastBeforeSequence{ 0 };
    std::atomic<AmUInt64> gLastAfterSequence{ 0 };

    void OnBeforeUpdate(AmTime, AmVoidPtr)
    {
        gLastBeforeSequence = ++gSequence;
        ++gBeforeCalls;
    }

    void OnAfterUpdate(AmTime, AmVoidPtr)
    {
        gLastAfterSequence = ++gSequence;
        ++gAfterCalls;
    }

    bool Register(const PluginHost* host)
    {
        return host->RegisterFrameCallback(host, ePluginFrameStage_BeforeUpdate, OnBeforeUpdate, nullptr) &&
            host->RegisterFrameCallback(host, ePluginFrameStage_AfterUpdate, OnAfterUpdate, nullptr);
    }

    bool Unregister(const PluginHost*)
    {
        return true;
    }

    const PluginDescriptor kDescriptor = {
        .abiVersion = AM_PLUGIN_ABI_VERSION,
        .structSize = sizeof(PluginDescriptor),
        .name = "Test Plugin",
        .version = "1.0.0",
        .description = "Test plugin for Amplitude.",
        .author = "Sparky Studios",
        .copyright = "Copyright (c) 2025-present Sparky Studios. All rights Reserved.",
        .license = "Apache License, Version 2.0",
        .Register = Register,
        .Unregister = Unregister,
    };
} // namespace

AM_DEFINE_PLUGIN(TestPlugin, kDescriptor)

extern "C" AM_LIB_EXPORT void TestPluginHookState(
    AmUInt64* beforeCalls, AmUInt64* afterCalls, AmUInt64* lastBeforeSequence, AmUInt64* lastAfterSequence)
{
    *beforeCalls = gBeforeCalls;
    *afterCalls = gAfterCalls;
    *lastBeforeSequence = gLastBeforeSequence;
    *lastAfterSequence = gLastAfterSequence;
}
