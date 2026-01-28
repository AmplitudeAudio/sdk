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

#import "TestRunnerViewController.h"

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "PlatformTestCase.h"
#include "TestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;
using namespace SparkyStudios::Audio::Amplitude::Tests;

@interface TestRunnerViewController ()

/// Flag indicating whether tests are currently running
@property (nonatomic) BOOL isRunning;

/// Flag for auto-run mode (CI/automation)
@property (nonatomic) BOOL autoRun;

/// Flag to exit after tests complete (CI/automation)
@property (nonatomic) BOOL exitOnComplete;

@end

@implementation TestRunnerViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.isRunning = NO;
    self.resultsTextView.text = @"";
    self.statusLabel.text = @"Ready";
    self.progressView.progress = 0.0;

    // Make the results text view read-only but allow selection for copy
    self.resultsTextView.editable = NO;

    // Check for CI/automation flags in launch arguments or environment
    [self parseAutomationFlags];

    // Configure platform test settings
    [self configurePlatformSettings];

    if (self.autoRun)
    {
        [self logMessage:@"Auto-run mode enabled, starting tests..."];
        // Delay slightly to allow UI to render
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self runTestsButtonTapped:nil];
        });
    }
    else
    {
        [self logMessage:@"Press 'Run Tests' to start test execution."];
    }
}

- (void)parseAutomationFlags
{
    // Check launch arguments
    NSArray *arguments = [[NSProcessInfo processInfo] arguments];
    self.autoRun = [arguments containsObject:@"--autorun"] || [arguments containsObject:@"-autorun"];
    self.exitOnComplete = [arguments containsObject:@"--exit-on-complete"] || [arguments containsObject:@"-exit-on-complete"];

    // Also check environment variables
    NSDictionary *environment = [[NSProcessInfo processInfo] environment];
    if (environment[@"AMPLITUDE_AUTORUN"] != nil)
    {
        self.autoRun = [environment[@"AMPLITUDE_AUTORUN"] boolValue] ||
                       [environment[@"AMPLITUDE_AUTORUN"] isEqualToString:@"true"] ||
                       [environment[@"AMPLITUDE_AUTORUN"] isEqualToString:@"1"];
    }
    if (environment[@"AMPLITUDE_EXIT_ON_COMPLETE"] != nil)
    {
        self.exitOnComplete = [environment[@"AMPLITUDE_EXIT_ON_COMPLETE"] boolValue] ||
                              [environment[@"AMPLITUDE_EXIT_ON_COMPLETE"] isEqualToString:@"true"] ||
                              [environment[@"AMPLITUDE_EXIT_ON_COMPLETE"] isEqualToString:@"1"];
    }

    NSLog(@"[AmplitudeTests] Automation flags: autoRun=%d, exitOnComplete=%d", self.autoRun, self.exitOnComplete);
}

- (void)configurePlatformSettings
{
    // Get the bundle path for assets
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *resourcePath = [bundle resourcePath];

    // Set the global platform config
    // Use static storage to ensure the string outlives the function
    static std::string bundlePathStr;
    bundlePathStr = [resourcePath UTF8String];
    g_platformTestConfig.bundlePath = bundlePathStr.c_str();
    g_platformTestConfig.assetsSubPath = "assets";

    [self logMessage:[NSString stringWithFormat:@"Bundle path: %@", resourcePath]];
    [self logMessage:[NSString stringWithFormat:@"Assets path: %@/assets", resourcePath]];
}

- (IBAction)runTestsButtonTapped:(id)sender
{
    if (self.isRunning)
    {
        return;
    }

    self.isRunning = YES;
    self.runTestsButton.enabled = NO;
    self.statusLabel.text = @"Running tests...";
    self.resultsTextView.text = @"";

    // Run tests on background thread to keep UI responsive
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self runAllTests];
    });
}

- (void)runAllTests
{
    // Get list of platform-compatible tests from the registry
    std::vector<std::string> testNames = TestRegistry::Instance().GetPlatformTests();
    NSInteger totalTests = static_cast<NSInteger>(testNames.size());
    NSInteger passedTests = 0;
    NSInteger failedTests = 0;

    [self logMessageOnMainThread:[NSString stringWithFormat:@"Found %ld tests to run\n", (long)totalTests]];

    for (NSInteger i = 0; i < totalTests; i++)
    {
        NSString *testName = [NSString stringWithUTF8String:testNames[i].c_str()];

        // Update UI on main thread
        dispatch_async(dispatch_get_main_queue(), ^{
            self.statusLabel.text = [NSString stringWithFormat:@"Running: %@", testName];
            self.progressView.progress = (float)(i + 1) / (float)totalTests;
        });

        BOOL success = [self runTest:testName];

        if (success)
        {
            passedTests++;
            [self logMessageOnMainThread:[NSString stringWithFormat:@"PASSED: %@", testName]];
        }
        else
        {
            failedTests++;
            [self logMessageOnMainThread:[NSString stringWithFormat:@"FAILED: %@", testName]];
        }
    }

    // Log summary to console for CI parsing
    NSLog(@"[AmplitudeTests] ========== TEST SUMMARY ==========");
    NSLog(@"[AmplitudeTests] Total:  %ld", (long)totalTests);
    NSLog(@"[AmplitudeTests] Passed: %ld", (long)passedTests);
    NSLog(@"[AmplitudeTests] Failed: %ld", (long)failedTests);
    NSLog(@"[AmplitudeTests] ==================================");

    // Display final summary on main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        self.isRunning = NO;
        self.runTestsButton.enabled = YES;

        NSString *summary = [NSString stringWithFormat:
            @"\n========== TEST SUMMARY ==========\n"
            @"Total:  %ld\n"
            @"Passed: %ld\n"
            @"Failed: %ld\n"
            @"==================================\n",
            (long)totalTests, (long)passedTests, (long)failedTests];

        [self appendLog:summary];

        if (failedTests == 0 && totalTests > 0)
        {
            self.statusLabel.text = @"All tests passed!";
        }
        else if (totalTests == 0)
        {
            self.statusLabel.text = @"No tests run";
        }
        else
        {
            self.statusLabel.text = [NSString stringWithFormat:@"%ld test(s) failed", (long)failedTests];
        }

        // Exit if in CI mode
        if (self.exitOnComplete)
        {
            NSLog(@"[AmplitudeTests] Exit on complete enabled, terminating...");
            // Use exit code based on test results
            int exitCode = (failedTests == 0 && totalTests > 0) ? 0 : 1;
            exit(exitCode);
        }
    });
}

- (BOOL)runTest:(NSString *)testName
{
    @try
    {
        // Get test case from registry
        auto testCase = TestRegistry::Instance().GetTestCase([testName UTF8String]);
        if (!testCase)
        {
            [self logMessageOnMainThread:[NSString stringWithFormat:@"  Test not found: %@", testName]];
            return NO;
        }

        // Run the test with C++ exception handling
        try
        {
            testCase->SetUp();
            testCase->Run();
            testCase->TearDown();
        }
        catch (const std::exception& e)
        {
            [self logMessageOnMainThread:[NSString stringWithFormat:@"  C++ Exception: %s", e.what()]];
            // Try to clean up
            try { testCase->TearDown(); } catch (...) {}
            return NO;
        }
        catch (...)
        {
            [self logMessageOnMainThread:@"  C++ Exception: Unknown"];
            // Try to clean up
            try { testCase->TearDown(); } catch (...) {}
            return NO;
        }

        // Log test details
        [self logMessageOnMainThread:[NSString stringWithFormat:@"  Expectations: %d passed, %d failed",
            testCase->PassedCount(), testCase->FailedCount()]];

        return !testCase->HasFailure();
    }
    @catch (NSException *exception)
    {
        [self logMessageOnMainThread:[NSString stringWithFormat:@"  ObjC Exception: %@", exception.reason]];
        return NO;
    }
}

- (void)logMessage:(NSString *)message
{
    NSLog(@"[AmplitudeTests] %@", message);
    [self appendLog:[message stringByAppendingString:@"\n"]];
}

- (void)logMessageOnMainThread:(NSString *)message
{
    NSLog(@"[AmplitudeTests] %@", message);
    [self appendLogOnMainThread:[message stringByAppendingString:@"\n"]];
}

- (void)appendLog:(NSString *)message
{
    self.resultsTextView.text = [self.resultsTextView.text stringByAppendingString:message];

    // Scroll to bottom to show latest results
    if (self.resultsTextView.text.length > 0)
    {
        NSRange range = NSMakeRange(self.resultsTextView.text.length - 1, 1);
        [self.resultsTextView scrollRangeToVisible:range];
    }
}

- (void)appendLogOnMainThread:(NSString *)message
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self appendLog:message];
    });
}

@end
