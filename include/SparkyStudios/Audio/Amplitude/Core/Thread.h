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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_THREAD_H
#define SS_AMPLITUDE_AUDIO_THREAD_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude::Thread
{
    typedef void (*AmThreadFunction)(AmVoidPtr param);

    struct AmThreadHandleData;
    typedef AmThreadHandleData* AmThreadHandle;

    AmVoidPtr CreateMutexAm();
    void DestroyMutex(AmVoidPtr handle);
    void LockMutex(AmVoidPtr handle);
    void UnlockMutex(AmVoidPtr handle);

    AmThreadHandle CreateThread(AmThreadFunction threadFunction, AmVoidPtr parameter);

    void Sleep(int milliseconds);
    void Wait(AmThreadHandle threadHandle);
    void Release(AmThreadHandle threadHandle);
    int GetTimeMillis();

#define MAX_THREAD_POOL_TASKS 1024

    class PoolTask
    {
    public:
        virtual void Work() = 0;
    };

    class Pool
    {
    public:
        Pool();
        // Waits for the threads to finish. Work may be unfinished.
        ~Pool();

        // Initialize and run thread pool. For thread count 0, Work is done at AddWork call.
        void Init(int threadCount);
        // Add work to work list. Object is not automatically deleted when Work is done.
        void AddWork(PoolTask* task);
        // Called from worker thread to Get a new task. Returns null if no Work available.
        PoolTask* GetWork();

    public:
        int mThreadCount; // number of threads
        AmThreadHandle* mThread; // array of thread handles
        AmVoidPtr mWorkMutex; // mutex to protect task array/max task
        PoolTask* mTaskArray[MAX_THREAD_POOL_TASKS]{}; // pointers to tasks
        int mMaxTask; // how many tasks are pending
        int mRobin; // cyclic counter, used to pick jobs for threads
        volatile int mRunning; // running flag, used to flag threads to Stop
    };
} // namespace SparkyStudios::Audio::Amplitude::Thread

#endif // SS_AMPLITUDE_AUDIO_THREAD_H
