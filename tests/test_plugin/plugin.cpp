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

using namespace SparkyStudios::Audio::Amplitude;

extern "C" {

AM_API_PLUGIN const char* PluginName()
{
    return "Test Plugin";
}

AM_API_PLUGIN const char* PluginVersion()
{
    return "1.0.0";
}

AM_API_PLUGIN const char* PluginDescription()
{
    return "Test plugin for Amplitude.";
}

AM_API_PLUGIN const char* PluginAuthor()
{
    return "Sparky Studios";
}

AM_API_PLUGIN const char* PluginCopyright()
{
    return "Copyright (c) 2025-present Sparky Studios. All rights Reserved.";
}

AM_API_PLUGIN const char* PluginLicense()
{
    return "Apache License, Version 2.0";
}

AM_API_PLUGIN bool RegisterPlugin(Engine* engine, MemoryManager* memoryManager)
{
    amLogSuccess("Test plugin registered successfully");
    return true;
}

AM_API_PLUGIN bool UnregisterPlugin()
{
    amLogSuccess("Test plugin unregistered successfully");
    return true;
}

} // extern "C"
