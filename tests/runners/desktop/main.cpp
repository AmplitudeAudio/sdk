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

/**
 * @file main.cpp
 * @brief Desktop test runner for Amplitude Audio SDK
 *
 * This is the unified desktop test runner that uses the TestRegistry to discover
 * and run all registered tests. It supports filtering, grouping, and verbose output.
 *
 * Usage:
 *   amplitude_tests [options]
 *
 * Options:
 *   -h, --help          Show help message
 *   -l, --list          List available tests
 *   -g, --group GROUP   Run tests in specified group only
 *   -v, --verbose       Verbose output
 *   --filter PATTERN    Run tests matching pattern (substring match)
 */

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/IO/ConsoleLogger.h>

#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;
using namespace SparkyStudios::Audio::Amplitude::Tests;

namespace
{
    struct RunOptions
    {
        bool showHelp = false;
        bool listTests = false;
        bool verbose = false;
        std::string groupFilter;
        std::string nameFilter;
    };

    void PrintHelp(const char* programName)
    {
        std::cout << "Amplitude Audio SDK Test Runner\n";
        std::cout << "================================\n\n";
        std::cout << "Usage: " << programName << " [options]\n\n";
        std::cout << "Options:\n";
        std::cout << "  -h, --help          Show this help message\n";
        std::cout << "  -l, --list          List available tests\n";
        std::cout << "  -g, --group GROUP   Run tests in specified group only\n";
        std::cout << "  -v, --verbose       Verbose output\n";
        std::cout << "  --filter PATTERN    Run tests matching pattern (substring match)\n";
        std::cout << "\nExamples:\n";
        std::cout << "  " << programName << " -l                    List all tests\n";
        std::cout << "  " << programName << " -g core_engine        Run core_engine tests\n";
        std::cout << "  " << programName << " --filter memory       Run tests containing 'memory'\n";
        std::cout << "  " << programName << " -v                    Run all tests with verbose output\n";
    }

    RunOptions ParseArgs(int argc, char* argv[])
    {
        RunOptions options;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg == "-h" || arg == "--help")
            {
                options.showHelp = true;
            }
            else if (arg == "-l" || arg == "--list")
            {
                options.listTests = true;
            }
            else if (arg == "-v" || arg == "--verbose")
            {
                options.verbose = true;
            }
            else if ((arg == "-g" || arg == "--group") && i + 1 < argc)
            {
                options.groupFilter = argv[++i];
            }
            else if (arg == "--filter" && i + 1 < argc)
            {
                options.nameFilter = argv[++i];
            }
        }

        return options;
    }

    void ListTests(const std::vector<std::string>& testNames, const RunOptions& options)
    {
        std::cout << "Available tests (" << testNames.size() << " total):\n";
        std::cout << "----------------------------------------\n";

        std::string currentGroup;
        for (const auto& name : testNames)
        {
            const TestInfo* info = TestRegistry::Instance().GetTestInfo(name);
            if (!info)
                continue;

            // Apply filters
            if (!options.groupFilter.empty() && info->group != options.groupFilter)
                continue;

            if (!options.nameFilter.empty() && name.find(options.nameFilter) == std::string::npos)
                continue;

            // Print group header if changed
            if (info->group != currentGroup)
            {
                currentGroup = info->group;
                std::cout << "\n[" << currentGroup << "]\n";
            }

            std::cout << "  " << name << "\n";
        }
        std::cout << "\n";
    }

    struct TestResult
    {
        std::string name;
        bool passed = false;
        int passedExpectations = 0;
        int failedExpectations = 0;
        double durationMs = 0.0;
        std::string errorMessage;
    };

    TestResult RunSingleTest(const std::string& name, bool verbose)
    {
        TestResult result;
        result.name = name;

        auto testCase = TestRegistry::Instance().GetTestCase(name);
        if (!testCase)
        {
            result.passed = false;
            result.errorMessage = "Failed to create test case";
            return result;
        }

        if (verbose)
            std::cout << "  Running: " << name << " ... " << std::flush;

        auto startTime = std::chrono::high_resolution_clock::now();

        try
        {
            testCase->SetUp();
            testCase->Run();
            testCase->TearDown();

            result.passedExpectations = testCase->PassedCount();
            result.failedExpectations = testCase->FailedCount();
            result.passed = !testCase->HasFailure();
        } catch (const std::exception& e)
        {
            result.passed = false;
            result.errorMessage = std::string("Exception: ") + e.what();
        } catch (...)
        {
            result.passed = false;
            result.errorMessage = "Unknown exception";
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        if (verbose)
        {
            if (result.passed)
                std::cout << "PASSED (" << result.durationMs << " ms)\n";
            else
                std::cout << "FAILED (" << result.durationMs << " ms)\n";
        }

        return result;
    }

    int RunTests(const std::vector<std::string>& testNames, const RunOptions& options)
    {
        std::vector<std::string> testsToRun;

        // Apply filters
        for (const auto& name : testNames)
        {
            const TestInfo* info = TestRegistry::Instance().GetTestInfo(name);
            if (!info)
                continue;

            if (!options.groupFilter.empty() && info->group != options.groupFilter)
                continue;

            if (!options.nameFilter.empty() && name.find(options.nameFilter) == std::string::npos)
                continue;

            testsToRun.push_back(name);
        }

        if (testsToRun.empty())
        {
            std::cout << "No tests match the specified filters.\n";
            return 1;
        }

        std::cout << "Running " << testsToRun.size() << " tests...\n";
        if (!options.groupFilter.empty())
            std::cout << "  Group: " << options.groupFilter << "\n";
        if (!options.nameFilter.empty())
            std::cout << "  Filter: " << options.nameFilter << "\n";
        std::cout << "----------------------------------------\n\n";

        std::vector<TestResult> results;
        results.reserve(testsToRun.size());

        int passed = 0;
        int failed = 0;

        auto totalStartTime = std::chrono::high_resolution_clock::now();

        for (const auto& name : testsToRun)
        {
            TestResult result = RunSingleTest(name, options.verbose);
            results.push_back(result);

            if (result.passed)
                ++passed;
            else
                ++failed;

            // Print progress indicator if not verbose
            if (!options.verbose)
            {
                std::cout << (result.passed ? "." : "F") << std::flush;
                if ((passed + failed) % 50 == 0)
                    std::cout << " [" << (passed + failed) << "/" << testsToRun.size() << "]\n";
            }
        }

        auto totalEndTime = std::chrono::high_resolution_clock::now();
        double totalDurationMs = std::chrono::duration<double, std::milli>(totalEndTime - totalStartTime).count();

        // Print failed tests summary
        if (!options.verbose)
            std::cout << "\n";

        if (failed > 0)
        {
            std::cout << "\n----------------------------------------\n";
            std::cout << "Failed tests:\n";
            for (const auto& result : results)
            {
                if (!result.passed)
                {
                    std::cout << "  FAILED: " << result.name;
                    if (!result.errorMessage.empty())
                        std::cout << " - " << result.errorMessage;
                    std::cout << "\n";
                }
            }
        }

        // Print summary
        std::cout << "\n========== TEST SUMMARY ==========\n";
        std::cout << "Total:  " << testsToRun.size() << "\n";
        std::cout << "Passed: " << passed << "\n";
        std::cout << "Failed: " << failed << "\n";
        std::cout << "Time:   " << totalDurationMs << " ms\n";
        std::cout << "==================================\n";

        return (failed > 0) ? 1 : 0;
    }
} // namespace

int main(int argc, char* argv[])
{
    // Set up a console logger for test output
    static ConsoleLogger logger(false); // Don't display file and line for cleaner output
    Logger::SetLogger(&logger);

    RunOptions options = ParseArgs(argc, argv);

    if (options.showHelp)
    {
        PrintHelp(argv[0]);
        return 0;
    }

    // Get platform-compatible tests
    std::vector<std::string> testNames = TestRegistry::Instance().GetPlatformTests();

    // Sort tests for consistent ordering
    std::sort(testNames.begin(), testNames.end());

    if (options.listTests)
    {
        ListTests(testNames, options);
        return 0;
    }

    return RunTests(testNames, options);
}
