// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "TestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void deviceCallback(eDeviceNotification notification, const DeviceDescription& device, Driver* driver)
{
    amLogDebug(
        "Device notification: %d, device: %s, driver: %s", static_cast<int>(notification), device.mDeviceName.c_str(),
        driver->GetName().c_str());
}

int main()
{
    ConsoleLogger logger;
    Logger::SetLogger(&logger);

    RegisterDeviceNotificationCallback(deviceCallback);

    bool success = true;
    {
        auto testCase = Tests::MakeTestCase();

        testCase->SetUp();
        testCase->Run();
        testCase->TearDown();

        success = !testCase->HasFailure();
    }

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
