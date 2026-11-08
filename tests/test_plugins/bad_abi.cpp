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

using namespace SparkyStudios::Audio::Amplitude;

// A plugin built for another plugin ABI version: the engine must refuse it before calling into it.
namespace
{
    bool Register(const PluginHost*)
    {
        return true;
    }

    bool Unregister(const PluginHost*)
    {
        return true;
    }

    const PluginDescriptor kDescriptor = {
        .abiVersion = AM_PLUGIN_ABI_VERSION + 1,
        .structSize = sizeof(PluginDescriptor),
        .name = "Test Plugin With Another ABI",
        .version = "1.0.0",
        .Register = Register,
        .Unregister = Unregister,
    };
} // namespace

AM_DEFINE_PLUGIN(TestPluginBadAbi, kDescriptor)
