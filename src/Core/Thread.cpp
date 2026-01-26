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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>

#if AM_PLATFORM_WIN
// clang-format off
#include <processthreadsapi.h>
// clang-format on
#else
#include <ctime>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <cmath>
#include <thread>

namespace SparkyStudios::Audio::Amplitude::Thread
{
    struct AmThreadData
    {
        AmThreadFunction mFunc;
        AmVoidPtr mParam;
    };

#if AM_PLATFORM_WIN
    struct AmThreadHandleData
    {
        HANDLE thread;
        AmThreadData* data;
    };

    static DWORD WINAPI ThreadFunc(LPVOID d)
    {
        auto* p = static_cast<AmThreadData*>(d);
        p->mFunc(p->mParam);
        return 0;
    }

    AmThreadHandle CreateThread(AmThreadFunction threadFunction, AmVoidPtr parameter)
    {
        auto* d = ampoolnew(eMemoryPoolKind_IO, AmThreadData);
        d->mFunc = threadFunction;
        d->mParam = parameter;

        HANDLE h = ::CreateThread(nullptr, 0, ThreadFunc, (LPVOID)d, 0, nullptr);

        if (nullptr == h)
            return nullptr;

        auto* threadHandle = ampoolnew(eMemoryPoolKind_IO, AmThreadHandleData);
        threadHandle->thread = h;
        threadHandle->data = d;

        return static_cast<AmThreadHandle>(threadHandle);
    }

    void Sleep(AmInt32 milliseconds)
    {
        ::Sleep(milliseconds);
    }

    void Wait(AmThreadHandle threadHandle)
    {
        auto* threadHandleData = static_cast<AmThreadHandleData*>(threadHandle);
        ::WaitForSingleObject(threadHandleData->thread, INFINITE);
    }

    void Release(AmThreadHandle& threadHandle)
    {
        auto* threadHandleData = static_cast<AmThreadHandleData*>(threadHandle);
        ::CloseHandle(threadHandleData->thread);
        ampooldelete(eMemoryPoolKind_IO, AmThreadData, threadHandleData->data);
        ampooldelete(eMemoryPoolKind_IO, AmThreadHandleData, threadHandleData);
        threadHandle = nullptr;
    }

    AmUInt64 GetTimeMillis()
    {
        return ::GetTickCount64();
    }

    AmThreadID GetCurrentThreadId()
    {
        return ::GetCurrentThreadId();
    }
#else // pthreads
    struct AmThreadHandleData
    {
        pthread_t thread;
        AmThreadData* data;
    };

    static AmVoidPtr ThreadFunc(AmVoidPtr d)
    {
        auto* p = static_cast<AmThreadData*>(d);
        p->mFunc(p->mParam);
        return nullptr;
    }

    AmThreadHandle CreateThread(AmThreadFunction threadFunction, AmVoidPtr parameter)
    {
        auto* d = ampoolnew(eMemoryPoolKind_IO, AmThreadData);
        d->mFunc = threadFunction;
        d->mParam = parameter;

        auto* threadHandle = ampoolnew(eMemoryPoolKind_IO, AmThreadHandleData);
        threadHandle->data = d;

        pthread_create(&threadHandle->thread, nullptr, ThreadFunc, (AmVoidPtr)threadHandle->data);

        return threadHandle;
    }

    void Sleep(AmInt32 milliseconds)
    {
        struct timespec req = { 0 };

        if (milliseconds < 1000)
        {
            req.tv_sec = 0;
            req.tv_nsec = milliseconds * 1000000L;
        }
        else
        {
            req.tv_sec = std::floor(milliseconds / 1000);
            req.tv_nsec = (milliseconds % 1000) * 1000000L;
        }

        nanosleep(&req, nullptr);
    }

    void Wait(AmThreadHandle& threadHandle)
    {
        if (!threadHandle)
            return;

        auto* threadHandleData = static_cast<AmThreadHandleData*>(threadHandle);
        pthread_join(threadHandleData->thread, nullptr);
        ampooldelete(eMemoryPoolKind_IO, AmThreadData, threadHandleData->data);
        ampooldelete(eMemoryPoolKind_IO, AmThreadHandleData, threadHandleData);
        threadHandle = nullptr;
    }

    void Release(AmThreadHandle& threadHandle)
    {
        if (!threadHandle)
            return;

        auto* threadHandleData = static_cast<AmThreadHandleData*>(threadHandle);
        pthread_detach(threadHandleData->thread);
        ampooldelete(eMemoryPoolKind_IO, AmThreadData, threadHandleData->data);
        ampooldelete(eMemoryPoolKind_IO, AmThreadHandleData, threadHandleData);
        threadHandle = nullptr;
    }

    AmUInt64 GetTimeMillis()
    {
        struct timespec spec = { 0 };
        clock_gettime(CLOCK_REALTIME, &spec);
        return spec.tv_sec * 1000 + (spec.tv_nsec / 1.0e6);
    }

    AmThreadID GetCurrentThreadId()
    {
#if AM_PLATFORM_APPLE
        AmThreadID tid = 0;
        pthread_threadid_np(pthread_self(), &tid);
        return tid;
#else
        pid_t tid = syscall(__NR_gettid);
        return static_cast<AmThreadID>(tid);
#endif
    }
#endif

    AmUInt32 GetCPUCount()
    {
        // Since `hardware_concurrency()` may return 0 in edge cases,
        // we return at least 1 for the number of CPUs.
        return std::max(1u, std::thread::hardware_concurrency());
    }

    static void PoolWorker(AmVoidPtr param)
    {
        auto* pPool = static_cast<Pool*>(param);

        while (pPool->IsRunning())
        {
            if (std::shared_ptr<PoolTask> t = pPool->GetWork(); t == nullptr)
                Sleep(1);
            else
                t->Work();
        }
    }

    bool PoolTask::Ready()
    {
        return true;
    }

    AwaitablePoolTask::AwaitablePoolTask()
        : _condition()
        , _mutex()
    {}

    void AwaitablePoolTask::Work()
    {
        AwaitableWork();
        _condition.notify_all();
    }

    void AwaitablePoolTask::Await()
    {
        std::unique_lock lock(_mutex);
        _condition.wait(lock);
    }

    bool AwaitablePoolTask::Await(AmUInt64 duration)
    {
        std::unique_lock lock(_mutex);
        return _condition.wait_for(lock, std::chrono::milliseconds(duration)) == std::cv_status::no_timeout;
    }

    Pool::Pool()
        : _threadCount(0)
        , _thread(nullptr)
        , _workMutex()
        , _taskCount(0)
        , _robin(0)
        , _running(false)
    {
        for (auto& i : _taskArray)
            i = nullptr;
    }

    Pool::~Pool()
    {
        _running = false;

        for (AmUInt32 i = 0; i < _threadCount; i++)
            Wait(_thread[i]);

        ampoolfree(eMemoryPoolKind_IO, _thread);
    }

    void Pool::Init(AmUInt32 threadCount)
    {
        if (_running || threadCount == 0)
            return;

        _taskCount = 0;
        _running = true;
        _threadCount = threadCount;
        _thread = static_cast<AmThreadHandle*>(ampoolmalloc(eMemoryPoolKind_IO, sizeof(void*) * threadCount));

        for (AmUInt32 i = 0; i < _threadCount; i++)
            _thread[i] = CreateThread(PoolWorker, this);
    }

    void Pool::AddTask(std::shared_ptr<PoolTask> task)
    {
        if (_threadCount == 0)
        {
            if (task->Ready())
                task->Work();
        }
        else
        {
            _workMutex.lock();

            if (_taskCount == AM_MAX_THREAD_POOL_TASKS)
            {
                // If we're at max tasks, do the task on calling thread
                // (we're in trouble anyway, might as well slow down adding more tasks)
                _workMutex.unlock();

                if (task->Ready())
                    task->Work();
            }
            else
            {
                _taskArray[_taskCount] = task;
                _taskCount++;

                _workMutex.unlock();
            }
        }
    }

    std::shared_ptr<PoolTask> Pool::GetWork()
    {
        std::shared_ptr<PoolTask> t = nullptr;
        std::lock_guard lock(_workMutex);

        if (_taskCount > 0)
        {
            AmInt32 c = 0;

            do
            {
                const AmInt32 r = _robin % _taskCount;
                _robin++;
                t = _taskArray[r];

                if (t->Ready())
                {
                    _taskArray[r] = _taskArray[_taskCount - 1];
                    _taskCount--;

                    break;
                }

                t = nullptr;
                c++;
            } while (c < _taskCount);
        }

        return t;
    }

    AmUInt32 Pool::GetThreadCount() const
    {
        return _threadCount;
    }

    bool Pool::IsRunning() const
    {
        return _running;
    }

    bool Pool::HasTasks() const
    {
        return _taskCount > 0;
    }

    AmInt32 Pool::GetTaskCount() const
    {
        return _taskCount;
    }
} // namespace SparkyStudios::Audio::Amplitude::Thread
