/*
 * Copyright (c) 2026-present Sparky Studios. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.amplitudeaudiosdk.tests

import android.content.res.AssetManager

/**
 * JNI bridge object for native Amplitude test functions.
 *
 * This object provides the Kotlin interface to the native C++ test runner.
 * All methods are static and delegate to native implementations.
 */
object NativeTestBridge {

    init {
        System.loadLibrary("amplitude_tests")
    }

    /**
     * Initialize the native Amplitude test environment.
     *
     * This must be called before running any tests. It sets up:
     * - The Android AssetManager for file access
     * - The Amplitude MemoryManager
     * - The Android logger
     *
     * @param assetManager The Android AssetManager for accessing bundled assets
     */
    @JvmStatic
    external fun initialize(assetManager: AssetManager)

    /**
     * Deinitialize the native Amplitude test environment.
     *
     * This should be called when the test runner is finished.
     * It cleans up all Amplitude resources.
     */
    @JvmStatic
    external fun deinitialize()

    /**
     * Run a single test by name.
     *
     * @param testName The name of the test to run
     * @return true if the test passed, false otherwise
     */
    @JvmStatic
    external fun runTest(testName: String): Boolean

    /**
     * Get the list of available tests for the current platform.
     *
     * @return Array of test names that can run on Android
     */
    @JvmStatic
    external fun getAvailableTests(): Array<String>

    /**
     * Get the total count of registered tests.
     *
     * @return The number of registered tests
     */
    @JvmStatic
    external fun getTestCount(): Int
}
