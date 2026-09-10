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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "TestUtils.h"

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

        void Expect(bool condition, const char* message = "", const char* file = "", int line = 0)
        {
            if (!condition)
            {
                ReportFailure("Expectation failed", message, file, line);
            }
        }

        // Returns true if any expectation failed during the test
        [[nodiscard]] AM_INLINE bool HasFailure() const
        {
            return _failedCount > 0;
        }

        // Returns the number of passed expectations
        [[nodiscard]] AM_INLINE AmInt32 PassedCount() const
        {
            return _passedCount;
        }

        // Returns the number of failed expectations
        [[nodiscard]] AM_INLINE AmInt32 FailedCount() const
        {
            return _failedCount;
        }

        // Returns the total number of expectations ran
        [[nodiscard]] AM_INLINE AmInt32 TotalExpectations() const
        {
            return _passedCount + _failedCount;
        }

        // Run the test case (must be overridden)
        virtual void Run() = 0;

    protected:
        void ExpectFloatNear(AmReal32 expected, AmReal32 actual, AmReal32 tolerance = 0.0001f, const char* file = "", int line = 0)
        {
            Expect(std::abs(expected - actual) <= tolerance, "ExpectFloatNear", file, line);
        }

        void ExpectDoubleNear(AmReal64 expected, AmReal64 actual, AmReal64 tolerance = 0.0001, const char* file = "", int line = 0)
        {
            Expect(std::abs(expected - actual) <= tolerance, "ExpectDoubleNear", file, line);
        }

        virtual void ReportFailure(const char* failureType, const char* message, const char* file = "", int line = 0)
        {
            ++_failedCount;

            if (amLogger != nullptr)
            {
                constexpr size_t bufferLen = 4096;
                char buffer[bufferLen];
                int formatted = std::snprintf(buffer, bufferLen, "[TEST FAILURE] %s: %s", failureType, message);
                amLogger->Error(file, line, AmString(buffer).substr(0, formatted));
            }
        }

        virtual void ReportSuccess(const char* failureType, const char* message, const char* file = "", int line = 0)
        {
            ++_passedCount;

            if (amLogger != nullptr)
            {
                constexpr size_t bufferLen = 4096;
                char buffer[bufferLen];
                int formatted = std::snprintf(buffer, bufferLen, "%s: %s", failureType, message);
                amLogger->Success(file, line, AmString(buffer).substr(0, formatted));
            }
        }

    private:
        AmInt32 _passedCount;
        AmInt32 _failedCount;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests

using namespace SparkyStudios::Audio::Amplitude::Tests;
