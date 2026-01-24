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

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <jni.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "PlatformTestCase.h"
#include "TestCase.h"
#include "TestRegistry.h"

#define LOG_TAG "AmplitudeTests"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace SparkyStudios::Audio::Amplitude;
using namespace SparkyStudios::Audio::Amplitude::Tests;

/**
 * @brief Android-specific logger that outputs to logcat.
 *
 * This logger implementation redirects all Amplitude log messages
 * to Android's logcat system for easy debugging.
 */
class AndroidLogger : public Logger
{
public:
    void Log(eLogMessageLevel level, const AmString& file, AmUInt32 line, const AmString& message) override
    {
        int androidLevel;
        switch (level)
        {
        case eLogMessageLevel_Debug:
            androidLevel = ANDROID_LOG_DEBUG;
            break;
        case eLogMessageLevel_Info:
            androidLevel = ANDROID_LOG_INFO;
            break;
        case eLogMessageLevel_Warning:
            androidLevel = ANDROID_LOG_WARN;
            break;
        case eLogMessageLevel_Error:
            androidLevel = ANDROID_LOG_ERROR;
            break;
        case eLogMessageLevel_Critical:
            androidLevel = ANDROID_LOG_FATAL;
            break;
        default:
            androidLevel = ANDROID_LOG_VERBOSE;
            break;
        }
        __android_log_print(androidLevel, LOG_TAG, "[%s:%u] %s", file.c_str(), line, message.c_str());
    }
};

// Static instances
static AndroidLogger s_androidLogger;
static AAssetManager* s_assetManager = nullptr;

extern "C" {
/**
 * @brief Initialize the Amplitude test environment.
 *
 * This must be called before running any tests. It sets up:
 * - The Android AssetManager for file access
 * - The Amplitude MemoryManager
 * - The Android logger
 *
 * @param env JNI environment
 * @param clazz The NativeTestBridge class
 * @param assetManager The Android AssetManager instance
 */
JNIEXPORT void JNICALL Java_com_amplitudeaudiosdk_tests_NativeTestBridge_initialize(JNIEnv* env, jclass clazz, jobject assetManager)
{
    LOGI("Initializing Amplitude test environment");

    // Store asset manager for file system access
    s_assetManager = AAssetManager_fromJava(env, assetManager);
    if (s_assetManager == nullptr)
    {
        LOGE("Failed to get AAssetManager from Java");
        return;
    }

    // Configure platform test settings
    g_platformTestConfig.assetManager = s_assetManager;
    g_platformTestConfig.assetsSubPath = "assets";

    // Initialize memory manager
    MemoryManager::Initialize();

    // Set up logger
    Logger::SetLogger(&s_androidLogger);

    LOGI("Amplitude test environment initialized successfully");
}

/**
 * @brief Deinitialize the Amplitude test environment.
 *
 * This should be called when the test runner activity is destroyed.
 * It cleans up all Amplitude resources.
 *
 * @param env JNI environment
 * @param clazz The NativeTestBridge class
 */
JNIEXPORT void JNICALL Java_com_amplitudeaudiosdk_tests_NativeTestBridge_deinitialize(JNIEnv* env, jclass clazz)
{
    LOGI("Deinitializing Amplitude test environment");

    // Clean up engine if initialized
    if (amEngine != nullptr)
    {
        amEngine->Deinitialize();
        amEngine->DestroyInstance();
    }

    // Deinitialize memory manager
    MemoryManager::Deinitialize();

    // Clear asset manager reference
    s_assetManager = nullptr;

    LOGI("Amplitude test environment deinitialized");
}

/**
 * @brief Run a single test by name.
 *
 * @param env JNI environment
 * @param clazz The NativeTestBridge class
 * @param testName The name of the test to run
 * @return JNI_TRUE if the test passed, JNI_FALSE otherwise
 */
JNIEXPORT jboolean JNICALL Java_com_amplitudeaudiosdk_tests_NativeTestBridge_runTest(JNIEnv* env, jclass clazz, jstring testName)
{
    const char* testNameStr = env->GetStringUTFChars(testName, nullptr);
    if (testNameStr == nullptr)
    {
        LOGE("Failed to get test name string");
        return JNI_FALSE;
    }

    LOGI("Running test: %s", testNameStr);

    jboolean result = JNI_FALSE;

    try
    {
        // Get test case from registry
        auto testCase = TestRegistry::Instance().GetTestCase(testNameStr);
        if (testCase)
        {
            testCase->SetUp();
            testCase->Run();
            testCase->TearDown();

            result = testCase->HasFailure() ? JNI_FALSE : JNI_TRUE;

            LOGI(
                "Test %s: %s (passed: %d, failed: %d)", testNameStr, result ? "PASSED" : "FAILED", testCase->PassedCount(),
                testCase->FailedCount());
        }
        else
        {
            LOGE("Test case not found: %s", testNameStr);
        }
    } catch (const std::exception& e)
    {
        LOGE("Exception in test %s: %s", testNameStr, e.what());
    } catch (...)
    {
        LOGE("Unknown exception in test %s", testNameStr);
    }

    env->ReleaseStringUTFChars(testName, testNameStr);
    return result;
}

/**
 * @brief Get the list of available tests for the current platform.
 *
 * This returns only tests that are compatible with Android.
 *
 * @param env JNI environment
 * @param clazz The NativeTestBridge class
 * @return Array of test names
 */
JNIEXPORT jobjectArray JNICALL Java_com_amplitudeaudiosdk_tests_NativeTestBridge_getAvailableTests(JNIEnv* env, jclass clazz)
{
    // Get platform-compatible tests from registry
    std::vector<std::string> tests = TestRegistry::Instance().GetPlatformTests();

    LOGI("Found %zu platform-compatible tests", tests.size());

    // Create Java String array
    jclass stringClass = env->FindClass("java/lang/String");
    if (stringClass == nullptr)
    {
        LOGE("Failed to find java/lang/String class");
        return nullptr;
    }

    jobjectArray result = env->NewObjectArray(static_cast<jsize>(tests.size()), stringClass, nullptr);
    if (result == nullptr)
    {
        LOGE("Failed to create String array");
        return nullptr;
    }

    for (size_t i = 0; i < tests.size(); i++)
    {
        jstring str = env->NewStringUTF(tests[i].c_str());
        if (str != nullptr)
        {
            env->SetObjectArrayElement(result, static_cast<jsize>(i), str);
            env->DeleteLocalRef(str);
        }
    }

    return result;
}

/**
 * @brief Get the total count of registered tests.
 *
 * @param env JNI environment
 * @param clazz The NativeTestBridge class
 * @return The number of registered tests
 */
JNIEXPORT jint JNICALL Java_com_amplitudeaudiosdk_tests_NativeTestBridge_getTestCount(JNIEnv* env, jclass clazz)
{
    return static_cast<jint>(TestRegistry::Instance().GetTestCount());
}

} // extern "C"
