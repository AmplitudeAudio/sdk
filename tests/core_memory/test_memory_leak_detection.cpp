// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#include "MemoryTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(MemoryTestCase, core_memory, memory_leak_detection)
    {
    public:
        void Run() override
        {
            // Initialize memory manager
            MemoryManager::Initialize(nullptr);

#if !defined(AM_NO_MEMORY_STATS)
            // Test that no memory leaks are detected initially
            AmString initialLeakReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(initialLeakReport == "No memory leaks detected");

            // Test leak detection with intentional memory leak
            AmVoidPtr leakedPtr1 = amMemory->Malloc(eMemoryPoolKind_Default, 1024, __FILE__, __LINE__);
            AM_EXPECT(leakedPtr1 != nullptr);

            AmVoidPtr leakedPtr2 = amMemory->Malloc(eMemoryPoolKind_Engine, 512, __FILE__, __LINE__);
            AM_EXPECT(leakedPtr2 != nullptr);

            AmVoidPtr leakedPtr3 = amMemory->Malign(eMemoryPoolKind_SoundData, 256, 32, __FILE__, __LINE__);
            AM_EXPECT(leakedPtr3 != nullptr);

            // Check that leaks are detected
            AmString leakReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(leakReport != "No memory leaks detected");
            AM_EXPECT(leakReport.find("Memory leaks detected") != AmString::npos);
            AM_EXPECT(leakReport.find("Default") != AmString::npos);
            AM_EXPECT(leakReport.find("Engine") != AmString::npos);
            AM_EXPECT(leakReport.find("SoundData") != AmString::npos);

            // Verify leak report contains file and line information
            AM_EXPECT(leakReport.find(__FILE__) != AmString::npos);
            AM_EXPECT(leakReport.find("Size:") != AmString::npos);
            AM_EXPECT(leakReport.find("Address:") != AmString::npos);

            // Test that freeing memory removes leaks from detection
            amMemory->Free(eMemoryPoolKind_Default, leakedPtr1);

            AmString partialLeakReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(partialLeakReport != "No memory leaks detected");
            AM_EXPECT(partialLeakReport.find("Engine") != AmString::npos);
            AM_EXPECT(partialLeakReport.find("SoundData") != AmString::npos);

            // Free another allocation
            amMemory->Free(eMemoryPoolKind_Engine, leakedPtr2);

            AmString remainingLeakReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(remainingLeakReport != "No memory leaks detected");
            AM_EXPECT(remainingLeakReport.find("SoundData") != AmString::npos);

            // Free the last allocation
            amMemory->Free(eMemoryPoolKind_SoundData, leakedPtr3);

            AmString finalLeakReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(finalLeakReport == "No memory leaks detected");

            // Test leak detection with reallocation
            AmVoidPtr reallocPtr = amMemory->Malloc(eMemoryPoolKind_Filtering, 128, __FILE__, __LINE__);
            AM_EXPECT(reallocPtr != nullptr);

            // Realloc should update the tracking
            AmVoidPtr newReallocPtr = amMemory->Realloc(eMemoryPoolKind_Filtering, reallocPtr, 256, __FILE__, __LINE__);
            AM_EXPECT(newReallocPtr != nullptr);

            AmString reallocLeakReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(reallocLeakReport != "No memory leaks detected");
            AM_EXPECT(reallocLeakReport.find("Filtering") != AmString::npos);

            amMemory->Free(eMemoryPoolKind_Filtering, newReallocPtr);

            AmString postReallocReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(postReallocReport == "No memory leaks detected");

            // Test leak detection across different memory pools simultaneously
            AmVoidPtr codecPtr = amMemory->Malloc(eMemoryPoolKind_Codec, 64, __FILE__, __LINE__);
            AmVoidPtr ioPtr = amMemory->Malign(eMemoryPoolKind_IO, 128, 16, __FILE__, __LINE__);
            AmVoidPtr amplimixPtr = amMemory->Malloc(eMemoryPoolKind_Amplimix, 2048, __FILE__, __LINE__);

            AM_EXPECT(codecPtr != nullptr);
            AM_EXPECT(ioPtr != nullptr);
            AM_EXPECT(amplimixPtr != nullptr);

            AmString multiPoolLeakReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(multiPoolLeakReport != "No memory leaks detected");
            AM_EXPECT(multiPoolLeakReport.find("Codec") != AmString::npos);
            AM_EXPECT(multiPoolLeakReport.find("IO") != AmString::npos);
            AM_EXPECT(multiPoolLeakReport.find("Amplimix") != AmString::npos);

            // Clean up all allocations
            amMemory->Free(eMemoryPoolKind_Codec, codecPtr);
            amMemory->Free(eMemoryPoolKind_IO, ioPtr);
            amMemory->Free(eMemoryPoolKind_Amplimix, amplimixPtr);

            // Verify all leaks are resolved
            AmString cleanReport = amMemory->InspectMemoryLeaks();
            AM_EXPECT(cleanReport == "No memory leaks detected");

            // Test leak detection with zero-size allocations
            AmVoidPtr zeroPtr = amMemory->Malloc(eMemoryPoolKind_Default, 0, __FILE__, __LINE__);
            if (zeroPtr != nullptr) // Some allocators return null for zero-size allocations
            {
                AmString zeroLeakReport = amMemory->InspectMemoryLeaks();
                AM_EXPECT(zeroLeakReport != "No memory leaks detected");
                amMemory->Free(eMemoryPoolKind_Default, zeroPtr);

                AmString postZeroReport = amMemory->InspectMemoryLeaks();
                AM_EXPECT(postZeroReport == "No memory leaks detected");
            }

#else
            // Memory stats are disabled, InspectMemoryLeaks is not available
            // Just verify the memory manager can initialize and deinitialize
            AM_EXPECT(MemoryManager::IsInitialized());
#endif

            MemoryManager::Deinitialize();
        }
    };

    AM_REGISTER_TEST(core_memory, memory_leak_detection);
} // namespace SparkyStudios::Audio::Amplitude::Tests
