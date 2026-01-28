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

// Android Native Test Runner
//
// This is a NativeActivity-based test runner for the Amplitude Audio SDK.
// It uses android_native_app_glue to run tests without any Java/Kotlin code.
//
// Tests run automatically on app start and results are logged to logcat.
// Monitor with: adb logcat -s AmplitudeTests:V

#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/IO/Android/LogcatLogger.h>

#include "PlatformTestCase.h"
#include "TestRegistry.h"

#define LOG_TAG "AmplitudeTests"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global logger instance
static SparkyStudios::Audio::Amplitude::LogcatLogger* gLogger = nullptr;

using namespace SparkyStudios::Audio::Amplitude;
using namespace SparkyStudios::Audio::Amplitude::Tests;

// Forward declaration
static void RunAllTests(struct android_app* app);

/**
 * Process the next input event.
 */
static int32_t HandleInput(struct android_app* app, AInputEvent* event)
{
    // We don't need to handle input for test runner
    return 0;
}

/**
 * Process the next main command.
 */
static void HandleCommand(struct android_app* app, int32_t cmd)
{
    switch (cmd)
    {
    case APP_CMD_INIT_WINDOW:
        // Window is ready - we can run tests now
        if (app->window != nullptr)
        {
            LOGI("Window initialized, starting tests...");
            RunAllTests(app);
        }
        break;

    case APP_CMD_TERM_WINDOW:
        // Window is being destroyed
        LOGI("Window terminated");
        break;

    case APP_CMD_GAINED_FOCUS:
        LOGI("App gained focus");
        break;

    case APP_CMD_LOST_FOCUS:
        LOGI("App lost focus");
        break;

    default:
        break;
    }
}

/**
 * Initialize Amplitude test environment.
 */
static bool InitializeTestEnvironment(struct android_app* app)
{
    LOGI("Initializing Amplitude test environment...");

    // Configure platform test settings
    g_platformTestConfig.assetManager = app->activity->assetManager;
    g_platformTestConfig.assetsSubPath = AM_OS_STRING("amplitude_assets");

    // Initialize Amplitude memory manager
    MemoryManager::Initialize();
    LOGI("Memory manager initialized");

    // Set up LogcatLogger for Amplitude logging
    gLogger = new LogcatLogger(LOG_TAG);
    Logger::SetLogger(gLogger);
    LOGI("Logger registered");

    return true;
}

/**
 * Deinitialize Amplitude test environment.
 */
static void DeinitializeTestEnvironment()
{
    LOGI("Deinitializing Amplitude test environment...");

    Logger::SetLogger(nullptr);
    delete gLogger;
    gLogger = nullptr;

    MemoryManager::Deinitialize();

    LOGI("Test environment deinitialized");
}

/**
 * Run all registered tests and report results.
 */
static void RunAllTests(struct android_app* app)
{
    if (!InitializeTestEnvironment(app))
    {
        LOGE("Failed to initialize test environment!");
        ANativeActivity_finish(app->activity);
        return;
    }

    LOGI("========================================");
    LOGI("  Amplitude Audio SDK Test Runner");
    LOGI("  Platform: Android");
    LOGI("========================================");

    // Get platform-compatible tests
    auto& registry = TestRegistry::Instance();
    auto tests = registry.GetPlatformTests();
    const size_t totalTests = tests.size();

    LOGI("Found %zu tests to run", totalTests);
    LOGI("");

    size_t passedCount = 0;
    size_t failedCount = 0;

    for (size_t i = 0; i < tests.size(); ++i)
    {
        const auto& testName = tests[i];
        LOGI("[%zu/%zu] Running: %s", i + 1, totalTests, testName.c_str());

        auto testCase = registry.GetTestCase(testName);
        if (!testCase)
        {
            LOGE("  ERROR: Could not create test case");
            failedCount++;
            continue;
        }

        try
        {
            testCase->SetUp();
            testCase->Run();
            testCase->TearDown();

            if (!testCase->HasFailure())
            {
                LOGI("  PASSED");
                passedCount++;
            }
            else
            {
                LOGE("  FAILED");
                failedCount++;
            }
        } catch (const std::exception& e)
        {
            LOGE("  EXCEPTION: %s", e.what());
            failedCount++;

            try
            {
                testCase->TearDown();
            } catch (...)
            {
                // Ignore cleanup errors
            }
        } catch (...)
        {
            LOGE("  EXCEPTION: Unknown");
            failedCount++;

            try
            {
                testCase->TearDown();
            } catch (...)
            {
                // Ignore cleanup errors
            }
        }
    }

    // Print summary
    LOGI("");
    LOGI("========================================");
    LOGI("  TEST SUMMARY");
    LOGI("========================================");
    LOGI("  Total:  %zu", totalTests);
    LOGI("  Passed: %zu", passedCount);
    LOGI("  Failed: %zu", failedCount);
    LOGI("========================================");

    if (failedCount == 0 && totalTests > 0)
    {
        LOGI("  ALL TESTS PASSED!");
    }
    else if (totalTests == 0)
    {
        LOGI("  NO TESTS RUN");
    }
    else
    {
        LOGE("  SOME TESTS FAILED");
    }

    LOGI("========================================");

    DeinitializeTestEnvironment();

    // Exit the app after tests complete
    LOGI("Tests complete, exiting...");
    ANativeActivity_finish(app->activity);
}

/**
 * This is the main entry point of a native application using android_native_app_glue.
 * It runs in its own thread, with its own event loop for receiving input events.
 */
void android_main(struct android_app* app)
{
    LOGI("android_main started");

    // Set up callbacks
    app->onAppCmd = HandleCommand;
    app->onInputEvent = HandleInput;

    // Event loop
    while (true)
    {
        int events;
        struct android_poll_source* source;

        // Poll for events. Use -1 timeout to block when no window is available.
        while (ALooper_pollOnce(app->window ? 0 : -1, nullptr, &events, (void**)&source) >= 0)
        {
            // Process this event
            if (source != nullptr)
            {
                source->process(app, source);
            }

            // Check if we are exiting
            if (app->destroyRequested)
            {
                LOGI("App destruction requested, exiting event loop");
                return;
            }
        }

        // Check if we should exit
        if (app->destroyRequested)
        {
            LOGI("App destruction requested, exiting event loop");
            return;
        }
    }
}
