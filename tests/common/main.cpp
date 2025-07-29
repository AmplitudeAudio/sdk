#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "TestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

int main()
{
    ConsoleLogger logger;
    Logger::SetLogger(&logger);

    // RegisterDeviceNotificationCallback(deviceCallback);

    MemoryManager::Initialize();

    TestCase testCase{};

    testCase.SetUp();
    testCase.Run();
    testCase.TearDown();

    MemoryManager::Deinitialize();

    return testCase.HasFailure() ? EXIT_FAILURE : EXIT_SUCCESS;
}