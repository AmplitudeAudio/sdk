// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#ifndef _AM_TESTS_COMMON_TEST_REGISTRY_H
#define _AM_TESTS_COMMON_TEST_REGISTRY_H

#include "TestCase.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /**
     * @brief Test factory function type.
     *
     * A function that creates and returns a new instance of a test case.
     */
    using TestFactory = std::function<std::shared_ptr<TestCase>()>;

    /**
     * @brief Test metadata including platform compatibility.
     *
     * Contains all information needed to identify, create, and filter tests
     * based on platform capabilities.
     */
    struct TestInfo
    {
        /**
         * @brief Unique name of the test.
         */
        std::string name;

        /**
         * @brief Group/category the test belongs to.
         */
        std::string group;

        /**
         * @brief Factory function to create the test case instance.
         */
        TestFactory factory;

        /**
         * @brief Whether this test can run on desktop platforms (Windows, macOS, Linux).
         */
        bool supportsDesktop = true;

        /**
         * @brief Whether this test can run on iOS.
         */
        bool supportsIOS = true;

        /**
         * @brief Whether this test can run on Android.
         */
        bool supportsAndroid = true;

        /**
         * @brief Whether this test requires plugin loading capability.
         *
         * If true, the test will be excluded on platforms that don't support
         * dynamic library loading (iOS, Android).
         */
        bool requiresPluginLoading = false;

        /**
         * @brief Whether this test requires file system access.
         *
         * Most tests require this, but some pure unit tests may not.
         */
        bool requiresFileSystem = true;
    };

    /**
     * @brief Global test registry for platform-aware test management.
     *
     * This singleton class maintains a registry of all available tests and provides
     * methods to query tests based on platform compatibility. Tests register themselves
     * using the REGISTER_TEST macros at static initialization time.
     */
    class TestRegistry
    {
    public:
        /**
         * @brief Gets the singleton instance of the test registry.
         *
         * @return Reference to the global TestRegistry instance.
         */
        static TestRegistry& Instance();

        /**
         * @brief Registers a test with the registry.
         *
         * @param[in] info The test information including name, group, factory, and platform support.
         */
        void RegisterTest(const TestInfo& info);

        /**
         * @brief Creates a test case instance by name.
         *
         * @param[in] name The unique name of the test.
         *
         * @return A shared pointer to the created test case, or nullptr if not found.
         */
        [[nodiscard]] std::shared_ptr<TestCase> GetTestCase(const std::string& name) const;

        /**
         * @brief Gets the test info for a specific test.
         *
         * @param[in] name The unique name of the test.
         *
         * @return Pointer to the TestInfo, or nullptr if not found.
         */
        [[nodiscard]] const TestInfo* GetTestInfo(const std::string& name) const;

        /**
         * @brief Gets names of all registered tests.
         *
         * @return Vector of all test names regardless of platform compatibility.
         */
        [[nodiscard]] std::vector<std::string> GetAllTests() const;

        /**
         * @brief Gets names of tests compatible with the current platform.
         *
         * Filters tests based on the current platform (iOS, Android, or desktop)
         * and excludes tests that require unsupported features (e.g., plugin loading
         * on mobile platforms).
         *
         * @return Vector of test names that can run on the current platform.
         */
        [[nodiscard]] std::vector<std::string> GetPlatformTests() const;

        /**
         * @brief Gets names of tests in a specific group.
         *
         * @param[in] group The group name to filter by.
         *
         * @return Vector of test names in the specified group.
         */
        [[nodiscard]] std::vector<std::string> GetTestsByGroup(const std::string& group) const;

        /**
         * @brief Gets the total count of registered tests.
         *
         * @return The number of registered tests.
         */
        [[nodiscard]] size_t GetTestCount() const;

        /**
         * @brief Checks if a test is registered.
         *
         * @param[in] name The test name to check.
         *
         * @return true if the test exists, false otherwise.
         */
        [[nodiscard]] bool HasTest(const std::string& name) const;

    private:
        TestRegistry() = default;
        ~TestRegistry() = default;

        // Non-copyable
        TestRegistry(const TestRegistry&) = delete;
        TestRegistry& operator=(const TestRegistry&) = delete;

        std::map<std::string, TestInfo> _tests;
    };

    /**
     * @brief Helper macro for registering tests that work on all platforms.
     *
     * Usage:
     * @code
     * REGISTER_TEST(MyTestCase, "math", "vector_add");
     * @endcode
     */
#define REGISTER_TEST(TestClass, TestGroup, TestName)                                                                                      \
    static bool _registered_##TestClass = []()                                                                                             \
    {                                                                                                                                      \
        TestInfo info;                                                                                                                     \
        info.name = TestName;                                                                                                              \
        info.group = TestGroup;                                                                                                            \
        info.factory = []()                                                                                                                \
        {                                                                                                                                  \
            return std::make_shared<TestClass>();                                                                                          \
        };                                                                                                                                 \
        TestRegistry::Instance().RegisterTest(info);                                                                                       \
        return true;                                                                                                                       \
    }()

    /**
     * @brief Helper macro for registering tests that only work on desktop platforms.
     *
     * Use this for tests that require features not available on mobile, such as
     * plugin loading or specific file system access patterns.
     *
     * Usage:
     * @code
     * REGISTER_TEST_DESKTOP_ONLY(PluginLoadTest, "plugins", "load_codec");
     * @endcode
     */
#define REGISTER_TEST_DESKTOP_ONLY(TestClass, TestGroup, TestName)                                                                         \
    static bool _registered_##TestClass = []()                                                                                             \
    {                                                                                                                                      \
        TestInfo info;                                                                                                                     \
        info.name = TestName;                                                                                                              \
        info.group = TestGroup;                                                                                                            \
        info.factory = []()                                                                                                                \
        {                                                                                                                                  \
            return std::make_shared<TestClass>();                                                                                          \
        };                                                                                                                                 \
        info.supportsIOS = false;                                                                                                          \
        info.supportsAndroid = false;                                                                                                      \
        TestRegistry::Instance().RegisterTest(info);                                                                                       \
        return true;                                                                                                                       \
    }()

    /**
     * @brief Helper macro for registering tests that require plugin loading.
     *
     * These tests will automatically be excluded on platforms that don't support
     * dynamic library loading (iOS, Android).
     *
     * Usage:
     * @code
     * REGISTER_TEST_REQUIRES_PLUGINS(SharedLibTest, "plugins", "load_shared_lib");
     * @endcode
     */
#define REGISTER_TEST_REQUIRES_PLUGINS(TestClass, TestGroup, TestName)                                                                     \
    static bool _registered_##TestClass = []()                                                                                             \
    {                                                                                                                                      \
        TestInfo info;                                                                                                                     \
        info.name = TestName;                                                                                                              \
        info.group = TestGroup;                                                                                                            \
        info.factory = []()                                                                                                                \
        {                                                                                                                                  \
            return std::make_shared<TestClass>();                                                                                          \
        };                                                                                                                                 \
        info.requiresPluginLoading = true;                                                                                                 \
        info.supportsIOS = false;                                                                                                          \
        info.supportsAndroid = false;                                                                                                      \
        TestRegistry::Instance().RegisterTest(info);                                                                                       \
        return true;                                                                                                                       \
    }()

    /**
     * @brief Helper macro for registering tests with custom platform flags.
     *
     * Usage:
     * @code
     * REGISTER_TEST_WITH_FLAGS(MyTest, "group", "name", true, false, true); // Desktop + Android only
     * @endcode
     */
#define REGISTER_TEST_WITH_FLAGS(TestClass, TestGroup, TestName, SupportsDesktop, SupportsIOS, SupportsAndroid)                            \
    static bool _registered_##TestClass = []()                                                                                             \
    {                                                                                                                                      \
        TestInfo info;                                                                                                                     \
        info.name = TestName;                                                                                                              \
        info.group = TestGroup;                                                                                                            \
        info.factory = []()                                                                                                                \
        {                                                                                                                                  \
            return std::make_shared<TestClass>();                                                                                          \
        };                                                                                                                                 \
        info.supportsDesktop = SupportsDesktop;                                                                                            \
        info.supportsIOS = SupportsIOS;                                                                                                    \
        info.supportsAndroid = SupportsAndroid;                                                                                            \
        TestRegistry::Instance().RegisterTest(info);                                                                                       \
        return true;                                                                                                                       \
    }()

    /**
     * @brief Convenience function to get a test case by name.
     *
     * @param[in] name The unique name of the test.
     *
     * @return A shared pointer to the created test case, or nullptr if not found.
     */
    inline std::shared_ptr<TestCase> GetTestCase(const char* name)
    {
        return TestRegistry::Instance().GetTestCase(name);
    }

    /**
     * @brief Convenience function to get a test case by name (std::string version).
     *
     * @param[in] name The unique name of the test.
     *
     * @return A shared pointer to the created test case, or nullptr if not found.
     */
    inline std::shared_ptr<TestCase> GetTestCase(const std::string& name)
    {
        return TestRegistry::Instance().GetTestCase(name);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_COMMON_TEST_REGISTRY_H
