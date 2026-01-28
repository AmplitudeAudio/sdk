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

#import "AppDelegate.h"

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/IO/iOS/NSLogger.h>

using namespace SparkyStudios::Audio::Amplitude;

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Initialize Amplitude Memory Manager
    MemoryManager::Initialize();

    // Set up console logger for debugging
    static NSLogger logger("com.amplitudeaudiosdk.tests", "AmplitudeTests");
    Logger::SetLogger(&logger);

    NSLog(@"Amplitude Test Runner initialized");

    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Clean up Amplitude resources
    if (amEngine != nullptr)
    {
        amEngine->Deinitialize();
        amEngine->DestroyInstance();
    }

    MemoryManager::Deinitialize();

    NSLog(@"Amplitude Test Runner terminated");
}

@end
