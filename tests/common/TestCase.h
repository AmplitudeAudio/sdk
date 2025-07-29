#pragma once

#include <iostream>

class TestCase
{
public:
    TestCase()
        : passedCount_(0)
        , failedCount_(0)
    {}

    virtual ~TestCase() = default;

    // Set up resources before each test (optional)
    void SetUp();

    // Clean up resources after each test (optional)
    void TearDown();

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
            ++passedCount_;
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
            ++passedCount_;
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
            ++passedCount_;
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
            ++passedCount_;
        }
    }

    // Returns true if any expectation failed during the test
    bool HasFailure() const
    {
        return failedCount_ > 0;
    }

    // Returns the number of passed expectations
    int PassedCount() const
    {
        return passedCount_;
    }

    // Returns the number of failed expectations
    int FailedCount() const
    {
        return failedCount_;
    }

    // Returns the total number of expectations ran
    int TotalExpectations() const
    {
        return passedCount_ + failedCount_;
    }

    // Run the test case (must be overridden)
    void Run();

protected:
    virtual void ReportFailure(const char* failureType, const char* message)
    {
        ++failedCount_;
        std::cout << "[TEST FAILURE] " << failureType << ": " << message << std::endl;
    }

private:
    int passedCount_;
    int failedCount_;
};