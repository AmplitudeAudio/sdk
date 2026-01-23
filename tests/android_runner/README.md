# Amplitude Audio SDK - Android Test Runner

This Android application runs the Amplitude Audio SDK test suite on Android devices and emulators.

## Prerequisites

1. **Android Studio** (or command-line Android SDK tools)
2. **Android NDK** (version 21 or later recommended)
3. **CMake** (version 3.22.1 or later)
4. **Pre-built Amplitude SDK** for Android

## Building the Amplitude SDK for Android

Before building the test runner, you need to build the Amplitude SDK for Android:

```bash
# From the SDK root directory
xmake f -p android -a arm64-v8a -m release
xmake b Amplitude::Static

# Build for other ABIs if needed
xmake f -p android -a armeabi-v7a -m release
xmake b Amplitude::Static
```

## Building the Test Runner

### Using Android Studio

1. Open `tests/android_runner/` in Android Studio
2. Sync Gradle files
3. Build and run on a device or emulator

### Using Command Line

```bash
cd tests/android_runner

# Build debug APK
./gradlew assembleDebug

# Build release APK
./gradlew assembleRelease

# Install on connected device
./gradlew installDebug
```

## Test Assets

Test assets must be copied to `app/src/main/assets/` before building:

```bash
# Build assets on desktop first
xmake f -p macosx -m debug --build_assets=y --build_tools=y
xmake b build_sample_project

# Copy to Android assets directory
cp -r build/samples/assets/* tests/android_runner/app/src/main/assets/
```

## Running Tests

1. Launch the "Amplitude Tests" app on your device
2. Press "Run Tests" to execute all platform-compatible tests
3. Results are displayed with pass/fail indicators
4. A summary shows total, passed, and failed test counts

## Troubleshooting

### Native Library Not Found

If you see "Failed to load native library", ensure:
- The Amplitude SDK is built for the correct ABI
- The library is in the expected location (`build/android/<ABI>/`)

### Assets Not Found

If tests fail due to missing assets:
- Verify assets are in `app/src/main/assets/`
- Rebuild the APK after copying assets

## Project Structure

```
android_runner/
├── app/
│   ├── src/main/
│   │   ├── java/com/amplitudeaudiosdk/tests/   # Kotlin source files
│   │   │   ├── NativeTestBridge.kt             # JNI bridge declarations
│   │   │   └── TestRunnerActivity.kt           # Main test runner UI
│   │   ├── cpp/                                 # Native C++ code (JNI bridge)
│   │   │   ├── CMakeLists.txt
│   │   │   └── native_test_bridge.cpp
│   │   ├── res/                                 # Android resources
│   │   │   ├── layout/activity_test_runner.xml
│   │   │   └── values/strings.xml
│   │   ├── assets/                              # Test assets (copy here)
│   │   └── AndroidManifest.xml
│   └── build.gradle.kts
├── build.gradle.kts                             # Root build file
├── settings.gradle.kts
├── gradle.properties
└── README.md
```

## Package Information

- **Package ID**: `com.amplitudeaudiosdk.tests`
- **Language**: Kotlin
- **Min SDK**: 24 (Android 7.0)
- **Target SDK**: 34 (Android 14)
