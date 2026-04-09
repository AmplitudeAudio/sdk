#!/bin/bash
# Copyright (c) 2026-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Android Test Runner Script
# This script installs and runs the Amplitude Audio SDK tests on an Android emulator.

set -e

echo "Installing test app..."
adb install -r build/android/x86_64/debug/tests/AmplitudeTests_Android.apk

# Clear and start capturing logcat
adb logcat -c
adb logcat -v time AmplitudeTests:V *:S > android_test_output.log 2>&1 &
LOGCAT_PID=$!

echo "Launching test app..."
adb shell am start -n com.amplitudeaudiosdk.tests/android.app.NativeActivity

# Wait for tests to complete (timeout after 10 minutes)
TIMEOUT=1200
ELAPSED=0

while [ "$ELAPSED" -lt "$TIMEOUT" ]; do
    if grep -q "TEST SUMMARY" android_test_output.log 2>/dev/null; then
        echo "Tests completed!"
        sleep 2
        break
    fi
    sleep 5
    ELAPSED=$((ELAPSED + 5))
    echo "Waiting for tests... ($ELAPSED seconds)"
done

# Stop logcat capture
kill "$LOGCAT_PID" 2>/dev/null || true

# Display results
echo ""
echo "=== Test Output ==="
cat android_test_output.log || true
echo "==================="
echo ""

# Parse results
if grep -q "Failed: 0" android_test_output.log && grep -q "Total:" android_test_output.log; then
    TOTAL=$(grep "Total:" android_test_output.log | tail -1 | awk '{print $2}')
    if [ -n "$TOTAL" ] && [ "$TOTAL" != "0" ]; then
        echo "All tests passed!"
        exit 0
    fi
fi

if grep -q "Failed:" android_test_output.log; then
    FAILED=$(grep "Failed:" android_test_output.log | tail -1 | awk '{print $2}')
    if [ -n "$FAILED" ] && [ "$FAILED" != "0" ]; then
        echo "Some tests failed!"
        exit 1
    fi
fi

echo "Could not determine test results"
exit 1
