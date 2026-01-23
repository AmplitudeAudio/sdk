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

import android.app.Activity
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.widget.Button
import android.widget.ProgressBar
import android.widget.ScrollView
import android.widget.TextView
import java.util.concurrent.Executors

/**
 * Main activity for the Amplitude Audio SDK test runner.
 *
 * Provides a UI for:
 * - Starting test execution
 * - Displaying test progress
 * - Showing test results with pass/fail indicators
 *
 * Supports CI/automation via intent extras:
 * - "autorun" (boolean): Automatically start tests on launch
 * - "exit_on_complete" (boolean): Exit app after tests complete
 *
 * Example: adb shell am start -n com.amplitudeaudiosdk.tests/.TestRunnerActivity --ez autorun true --ez exit_on_complete true
 */
class TestRunnerActivity : Activity() {

    companion object {
        private const val TAG = "AmplitudeTests"
        private const val EXTRA_AUTORUN = "autorun"
        private const val EXTRA_EXIT_ON_COMPLETE = "exit_on_complete"
    }

    private lateinit var runButton: Button
    private lateinit var resultsTextView: TextView
    private lateinit var statusTextView: TextView
    private lateinit var progressBar: ProgressBar
    private lateinit var scrollView: ScrollView

    private val executor = Executors.newSingleThreadExecutor()
    private val mainHandler = Handler(Looper.getMainLooper())

    @Volatile
    private var isRunning = false

    private var autoRun = false
    private var exitOnComplete = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_test_runner)

        // Parse intent extras for CI mode
        autoRun = intent.getBooleanExtra(EXTRA_AUTORUN, false)
        exitOnComplete = intent.getBooleanExtra(EXTRA_EXIT_ON_COMPLETE, false)

        Log.i(TAG, "TestRunnerActivity started (autoRun=$autoRun, exitOnComplete=$exitOnComplete)")

        // Find views
        runButton = findViewById(R.id.runButton)
        resultsTextView = findViewById(R.id.resultsTextView)
        statusTextView = findViewById(R.id.statusTextView)
        progressBar = findViewById(R.id.progressBar)
        scrollView = findViewById(R.id.scrollView)

        // Initialize native test environment
        try {
            NativeTestBridge.initialize(assets)
            logMessage("Native test environment initialized.")
            logMessage("Registered tests: ${NativeTestBridge.getTestCount()}\n")
        } catch (e: Exception) {
            logMessage("ERROR: Failed to initialize native environment: ${e.message}")
            runButton.isEnabled = false
            return
        }

        // Set up button click listener
        runButton.setOnClickListener { runTests() }

        if (autoRun) {
            logMessage("Auto-run mode enabled, starting tests...")
            // Delay slightly to allow UI to render
            mainHandler.postDelayed({ runTests() }, 500)
        } else {
            logMessage("Press 'Run Tests' to start test execution.")
        }
    }

    override fun onDestroy() {
        super.onDestroy()

        // Clean up native resources
        try {
            NativeTestBridge.deinitialize()
        } catch (e: Exception) {
            // Ignore errors during cleanup
        }

        // Shut down executor
        executor.shutdown()
    }

    /**
     * Run all available tests on a background thread.
     */
    private fun runTests() {
        if (isRunning) return

        isRunning = true
        runButton.isEnabled = false
        resultsTextView.text = ""
        statusTextView.text = getString(R.string.status_running)
        progressBar.progress = 0

        executor.execute {
            val tests: Array<String>
            try {
                tests = NativeTestBridge.getAvailableTests()
            } catch (e: Exception) {
                logMessageOnMain("ERROR: Failed to get test list: ${e.message}")
                finishTests(0, 0, 0)
                return@execute
            }

            if (tests.isEmpty()) {
                logMessageOnMain("No tests available to run.")
                finishTests(0, 0, 0)
                return@execute
            }

            val totalTests = tests.size
            var passedTests = 0
            var failedTests = 0

            logMessageOnMain("Running $totalTests tests...\n")

            tests.forEachIndexed { index, testName ->
                val progress = ((index + 1).toFloat() / totalTests * 100).toInt()

                // Update UI
                mainHandler.post {
                    statusTextView.text = "Running: $testName"
                    progressBar.progress = progress
                }

                // Run the test
                val success = try {
                    NativeTestBridge.runTest(testName)
                } catch (e: Exception) {
                    logMessageOnMain("EXCEPTION: $testName - ${e.message}")
                    failedTests++
                    return@forEachIndexed
                }

                if (success) {
                    passedTests++
                    logMessageOnMain("PASSED: $testName")
                } else {
                    failedTests++
                    logMessageOnMain("FAILED: $testName")
                }
            }

            finishTests(totalTests, passedTests, failedTests)
        }
    }

    /**
     * Display final test summary and re-enable the run button.
     */
    private fun finishTests(total: Int, passed: Int, failed: Int) {
        // Log summary to logcat for CI parsing
        Log.i(TAG, "========== TEST SUMMARY ==========")
        Log.i(TAG, "Total:  $total")
        Log.i(TAG, "Passed: $passed")
        Log.i(TAG, "Failed: $failed")
        Log.i(TAG, "==================================")

        mainHandler.post {
            isRunning = false
            runButton.isEnabled = true
            progressBar.progress = 100

            val summary = """
                |
                |========== TEST SUMMARY ==========
                |Total:  $total
                |Passed: $passed
                |Failed: $failed
                |==================================
                |
            """.trimMargin()

            appendLog(summary)

            statusTextView.text = when {
                failed == 0 && total > 0 -> getString(R.string.status_all_passed)
                total == 0 -> "No tests run"
                else -> "$failed test(s) failed"
            }

            // Exit if in CI mode
            if (exitOnComplete) {
                Log.i(TAG, "Exit on complete enabled, finishing activity...")
                val exitCode = if (failed == 0 && total > 0) 0 else 1
                finishAndRemoveTask()
                // Note: We can't set process exit code from Activity, but CI can parse logcat
            }
        }
    }

    /**
     * Log a message to both the UI and logcat.
     */
    private fun logMessage(message: String) {
        Log.i(TAG, message)
        appendLog("$message\n")
    }

    /**
     * Log a message from any thread.
     */
    private fun logMessageOnMain(message: String) {
        Log.i(TAG, message)
        appendLogOnMain("$message\n")
    }

    /**
     * Append a message to the results text view.
     * Must be called on the main thread.
     */
    private fun appendLog(message: String) {
        resultsTextView.append(message)
        // Scroll to bottom
        scrollView.post { scrollView.fullScroll(ScrollView.FOCUS_DOWN) }
    }

    /**
     * Append a message to the results text view from any thread.
     */
    private fun appendLogOnMain(message: String) {
        mainHandler.post { appendLog(message) }
    }
}
