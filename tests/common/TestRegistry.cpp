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

#include "TestRegistry.h"
#include "PlatformTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    TestRegistry& TestRegistry::Instance()
    {
        static TestRegistry instance;
        return instance;
    }

    void TestRegistry::RegisterTest(const TestInfo& info)
    {
        // Use group/name as key to avoid collisions between tests with same name in different groups
        std::string key = info.group + "/" + info.name;
        _tests[key] = info;
    }

    std::shared_ptr<TestCase> TestRegistry::GetTestCase(const std::string& name) const
    {
        auto it = _tests.find(name);
        if (it != _tests.end())
            return it->second.factory();

        return nullptr;
    }

    const TestInfo* TestRegistry::GetTestInfo(const std::string& name) const
    {
        auto it = _tests.find(name);
        if (it != _tests.end())
            return &it->second;

        return nullptr;
    }

    std::vector<std::string> TestRegistry::GetAllTests() const
    {
        std::vector<std::string> names;
        names.reserve(_tests.size());
        for (const auto& [name, info] : _tests)
            names.push_back(name);

        return names;
    }

    std::vector<std::string> TestRegistry::GetPlatformTests() const
    {
        std::vector<std::string> names;
        names.reserve(_tests.size());

        for (const auto& [name, info] : _tests)
        {
            // Check platform compatibility
#if AM_PLATFORM_IOS
            if (!info.supportsIOS)
                continue;
#elif AM_PLATFORM_ANDROID
            if (!info.supportsAndroid)
                continue;
#else
            if (!info.supportsDesktop)
                continue;
#endif

            // Check feature requirements
            if (info.requiresPluginLoading && !SupportsPluginLoading())
                continue;

            names.push_back(name);
        }

        return names;
    }

    std::vector<std::string> TestRegistry::GetTestsByGroup(const std::string& group) const
    {
        std::vector<std::string> names;
        for (const auto& [name, info] : _tests)
            if (info.group == group)
                names.push_back(name);

        return names;
    }

    size_t TestRegistry::GetTestCount() const
    {
        return _tests.size();
    }

    bool TestRegistry::HasTest(const std::string& name) const
    {
        return _tests.find(name) != _tests.end();
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
