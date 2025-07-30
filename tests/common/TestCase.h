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

#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class TestCase
    {
    public:
        TestCase()
            : _passedCount(0)
            , _failedCount(0)
        {}

        virtual ~TestCase() = default;

        // Set up resources before each test (optional)
        virtual void SetUp() = 0;

        // Clean up resources after each test (optional)
        virtual void TearDown() = 0;

        // Utility methods for common test expectations
        template<typename T>
        void ExpectEqual(const T& expected, const T& actual, const char* message = "")
        {
            if (!(expected == actual))
            {
                ReportFailure("ExpectEqual failed", message);
            }
            else
            {
                ++_passedCount;
            }
        }

        template<typename T>
        void ExpectNotEqual(const T& expected, const T& actual, const char* message = "")
        {
            if (expected == actual)
            {
                ReportFailure("ExpectNotEqual failed", message);
            }
            else
            {
                ++_passedCount;
            }
        }

        void ExpectTrue(bool condition, const char* message = "")
        {
            if (!condition)
            {
                ReportFailure("ExpectTrue failed", message);
            }
            else
            {
                ++_passedCount;
            }
        }

        void ExpectFalse(bool condition, const char* message = "")
        {
            if (condition)
            {
                ReportFailure("ExpectFalse failed", message);
            }
            else
            {
                ++_passedCount;
            }
        }

        // Returns true if any expectation failed during the test
        bool HasFailure() const
        {
            return _failedCount > 0;
        }

        // Returns the number of passed expectations
        int PassedCount() const
        {
            return _passedCount;
        }

        // Returns the number of failed expectations
        int FailedCount() const
        {
            return _failedCount;
        }

        // Returns the total number of expectations ran
        int TotalExpectations() const
        {
            return _passedCount + _failedCount;
        }

        // Run the test case (must be overridden)
        virtual void Run() = 0;

    protected:
        virtual void ReportFailure(const char* failureType, const char* message)
        {
            ++_failedCount;
            amLogError("[TEST FAILURE] %s: %s", failureType, message);
        }

    private:
        int _passedCount;
        int _failedCount;
    };

    std::shared_ptr<TestCase> MakeTestCase();
} // namespace SparkyStudios::Audio::Amplitude::Tests

using namespace SparkyStudios::Audio::Amplitude::Tests;
