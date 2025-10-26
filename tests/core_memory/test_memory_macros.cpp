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

using namespace SparkyStudios::Audio::Amplitude;

void MemoryTestCase::Run()
{
    // Initialize memory manager
    MemoryManager::Initialize(nullptr);
    {
        // Test pool-specific allocation macros
        constexpr AmSize allocSize = 1024;

        // Test ampoolmalloc macro
        AmVoidPtr ptr1 = ampoolmalloc(eMemoryPoolKind_Default, allocSize);
        AM_EXPECT(ptr1 != nullptr);
        AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, ptr1) >= allocSize);

        // Test writing to allocated memory
        auto* intPtr = static_cast<AmInt32*>(ptr1);
        *intPtr = 42;
        AM_EXPECT(*intPtr == 42);

        ampoolfree(eMemoryPoolKind_Default, ptr1);

        // Test ampoolmalign macro
        AmVoidPtr alignedPtr = ampoolmalign(eMemoryPoolKind_Engine, 512, 64);
        AM_EXPECT(alignedPtr != nullptr);
        AM_EXPECT(reinterpret_cast<uintptr_t>(alignedPtr) % 64 == 0);
        AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Engine, alignedPtr) >= 512);
        ampoolfree(eMemoryPoolKind_Engine, alignedPtr);

        // Test ampoolrealloc macro
        AmVoidPtr reallocPtr = ampoolmalloc(eMemoryPoolKind_SoundData, 256);
        AM_EXPECT(reallocPtr != nullptr);

        // Write test data
        auto* testData = static_cast<AmReal32*>(reallocPtr);
        testData[0] = 1.5f;
        testData[1] = 2.5f;

        AmVoidPtr newReallocPtr = ampoolrealloc(eMemoryPoolKind_SoundData, reallocPtr, 512);
        AM_EXPECT(newReallocPtr != nullptr);
        AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_SoundData, newReallocPtr) >= 512);

        // Verify data preservation
        auto* newTestData = static_cast<AmReal32*>(newReallocPtr);
        AM_EXPECT(newTestData[0] == 1.5f);
        AM_EXPECT(newTestData[1] == 2.5f);

        ampoolfree(eMemoryPoolKind_SoundData, newReallocPtr);

        // Test ampoolrealign macro
        AmVoidPtr realignPtr = ampoolmalign(eMemoryPoolKind_Filtering, 128, 32);
        AM_EXPECT(realignPtr != nullptr);

        auto* realignData = static_cast<AmReal64*>(realignPtr);
        *realignData = 3.14159;

        AmVoidPtr newRealignPtr = ampoolrealign(eMemoryPoolKind_Filtering, realignPtr, 256, 128);
        AM_EXPECT(newRealignPtr != nullptr);
        AM_EXPECT(reinterpret_cast<uintptr_t>(newRealignPtr) % 128 == 0);

        auto* newRealignData = static_cast<AmReal64*>(newRealignPtr);
        AM_EXPECT(*newRealignData == 3.14159);

        ampoolfree(eMemoryPoolKind_Filtering, newRealignPtr);

        // Test default pool macros
        AmVoidPtr defaultPtr = ammalloc(1024);
        AM_EXPECT(defaultPtr != nullptr);
        AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, defaultPtr) >= 1024);

        auto* defaultData = static_cast<AmInt32*>(defaultPtr);
        *defaultData = 0x12345678;
        AM_EXPECT(*defaultData == 0x12345678);

        amfree(defaultPtr);

        // Test ammalign macro
        AmVoidPtr defaultAlignedPtr = ammalign(512, 64);
        AM_EXPECT(defaultAlignedPtr != nullptr);
        AM_EXPECT(reinterpret_cast<uintptr_t>(defaultAlignedPtr) % 64 == 0);
        amfree(defaultAlignedPtr);

        // Test amrealloc macro
        AmVoidPtr defaultReallocPtr = ammalloc(128);
        AM_EXPECT(defaultReallocPtr != nullptr);

        auto* defaultReallocData = static_cast<AmUInt32*>(defaultReallocPtr);
        *defaultReallocData = 0xABCDEF01;

        AmVoidPtr newDefaultReallocPtr = amrealloc(defaultReallocPtr, 256);
        AM_EXPECT(newDefaultReallocPtr != nullptr);

        auto* newDefaultReallocData = static_cast<AmUInt32*>(newDefaultReallocPtr);
        AM_EXPECT(*newDefaultReallocData == 0xABCDEF01);

        amfree(newDefaultReallocPtr);

        // Test amrealign macro
        AmVoidPtr defaultRealignPtr = ammalign(64, 16);
        AM_EXPECT(defaultRealignPtr != nullptr);

        auto* defaultRealignData = static_cast<AmUInt64*>(defaultRealignPtr);
        *defaultRealignData = 0x123456789ABCDEF0ULL;

        AmVoidPtr newDefaultRealignPtr = amrealign(defaultRealignPtr, 128, 32);
        AM_EXPECT(newDefaultRealignPtr != nullptr);
        AM_EXPECT(reinterpret_cast<uintptr_t>(newDefaultRealignPtr) % 32 == 0);

        auto* newDefaultRealignData = static_cast<AmUInt64*>(newDefaultRealignPtr);
        AM_EXPECT(*newDefaultRealignData == 0x123456789ABCDEF0ULL);

        amfree(newDefaultRealignPtr);

        // Test C++ object allocation macros
        struct TestObject
        {
            AmInt32 value;
            AmReal32 floatValue;

            TestObject()
                : value(100)
                , floatValue(3.14f)
            {}
            TestObject(AmInt32 v, AmReal32 f)
                : value(v)
                , floatValue(f)
            {}
        };

        // Test ampoolnew macro
        TestObject* poolObj = ampoolnew(eMemoryPoolKind_Codec, TestObject);
        AM_EXPECT(poolObj != nullptr);
        AM_EXPECT(poolObj->value == 100);
        AM_EXPECT(poolObj->floatValue == 3.14f);

        poolObj->value = 200;
        poolObj->floatValue = 2.71f;
        AM_EXPECT(poolObj->value == 200);
        AM_EXPECT(poolObj->floatValue == 2.71f);

        ampooldelete(eMemoryPoolKind_Codec, TestObject, poolObj);

        // Test ampoolnew with constructor arguments
        TestObject* poolObjWithArgs = ampoolnew(eMemoryPoolKind_IO, TestObject, 42, 1.41f);
        AM_EXPECT(poolObjWithArgs != nullptr);
        AM_EXPECT(poolObjWithArgs->value == 42);
        AM_EXPECT(poolObjWithArgs->floatValue == 1.41f);

        ampooldelete(eMemoryPoolKind_IO, TestObject, poolObjWithArgs);

        // Test amnew macro (default pool)
        TestObject* defaultObj = amnew(TestObject);
        AM_EXPECT(defaultObj != nullptr);
        AM_EXPECT(defaultObj->value == 100);
        AM_EXPECT(defaultObj->floatValue == 3.14f);

        defaultObj->value = 300;
        AM_EXPECT(defaultObj->value == 300);

        amdelete(TestObject, defaultObj);

        // Test amnew with constructor arguments
        TestObject* defaultObjWithArgs = amnew(TestObject, 99, 9.99f);
        AM_EXPECT(defaultObjWithArgs != nullptr);
        AM_EXPECT(defaultObjWithArgs->value == 99);
        AM_EXPECT(defaultObjWithArgs->floatValue == 9.99f);

        amdelete(TestObject, defaultObjWithArgs);

        // Test smart pointer macros
        auto poolUnique = ampoolunique(eMemoryPoolKind_Amplimix, TestObject, 777, 7.77f);
        AM_EXPECT(poolUnique != nullptr);
        AM_EXPECT(poolUnique->value == 777);
        AM_EXPECT(poolUnique->floatValue == 7.77f);

        poolUnique->value = 888;
        AM_EXPECT(poolUnique->value == 888);

        // poolUnique should automatically clean up when it goes out of scope

        auto defaultUnique = amunique(TestObject, 555, 5.55f);
        AM_EXPECT(defaultUnique != nullptr);
        AM_EXPECT(defaultUnique->value == 555);
        AM_EXPECT(defaultUnique->floatValue == 5.55f);

        // Test shared pointer macros
        auto poolShared = ampoolshared(eMemoryPoolKind_Default, TestObject, 666, 6.66f);
        AM_EXPECT(poolShared != nullptr);
        AM_EXPECT(poolShared->value == 666);
        AM_EXPECT(poolShared->floatValue == 6.66f);
        AM_EXPECT(poolShared.use_count() == 1);

        auto poolSharedCopy = poolShared;
        AM_EXPECT(poolShared.use_count() == 2);
        AM_EXPECT(poolSharedCopy.use_count() == 2);

        auto defaultShared = amshared(TestObject, 444, 4.44f);
        AM_EXPECT(defaultShared != nullptr);
        AM_EXPECT(defaultShared->value == 444);
        AM_EXPECT(defaultShared->floatValue == 4.44f);

        // Test array allocations
        constexpr AmSize arraySize = 10;
        AmInt32* intArray = static_cast<AmInt32*>(ampoolmalloc(eMemoryPoolKind_Default, sizeof(AmInt32) * arraySize));
        AM_EXPECT(intArray != nullptr);

        for (AmSize i = 0; i < arraySize; ++i)
        {
            intArray[i] = static_cast<AmInt32>(i * i);
        }

        for (AmSize i = 0; i < arraySize; ++i)
        {
            AM_EXPECT(intArray[i] == static_cast<AmInt32>(i * i));
        }

        ampoolfree(eMemoryPoolKind_Default, intArray);

        // Test that macros work with null pointers gracefully
        amfree(nullptr); // Should not crash
        ampoolfree(eMemoryPoolKind_Default, nullptr); // Should not crash
    }
    MemoryManager::Deinitialize();
}
