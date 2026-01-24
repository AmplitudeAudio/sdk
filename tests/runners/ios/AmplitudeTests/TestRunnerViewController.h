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

#pragma once

#import <UIKit/UIKit.h>

/**
 * @brief Main view controller for running Amplitude SDK tests on iOS.
 *
 * Provides a UI for:
 * - Starting test execution
 * - Displaying test progress
 * - Showing test results
 */
@interface TestRunnerViewController : UIViewController

/// Button to trigger test execution
@property (weak, nonatomic) IBOutlet UIButton *runTestsButton;

/// Text view for displaying test results and logs
@property (weak, nonatomic) IBOutlet UITextView *resultsTextView;

/// Label showing current test status
@property (weak, nonatomic) IBOutlet UILabel *statusLabel;

/// Progress bar showing overall test progress
@property (weak, nonatomic) IBOutlet UIProgressView *progressView;

/// Action method called when the run tests button is tapped
- (IBAction)runTestsButtonTapped:(id)sender;

@end
