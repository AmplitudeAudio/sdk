// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#include <c/include/amplitude_thread.h>

#include "amplitude_internals.h"

class CPoolTask final : public Thread::PoolTask
{
public:
    explicit CPoolTask(am_thread_pool_task_proc func, am_voidptr param = nullptr)
        : _func(func)
        , _param(param)
    {}

    void Work() override
    {
        _func(reinterpret_cast<am_thread_pool_task_handle>(this), _param);
    }

    bool Ready() override
    {
        return _is_ready;
    }

    void SetReady()
    {
        _is_ready = true;
    }

private:
    am_thread_pool_task_proc _func;
    am_voidptr _param;

    bool _is_ready = false;
};

class CAwaitablePoolTask final : public Thread::AwaitablePoolTask
{
public:
    explicit CAwaitablePoolTask(am_thread_pool_task_awaitable_proc func, am_voidptr param = nullptr)
        : _func(func)
        , _param(param)
    {}

    void AwaitableWork() override
    {
        _func(reinterpret_cast<am_thread_pool_task_awaitable_handle>(this), _param);
    }

    bool Ready() override
    {
        return _is_ready;
    }

    void SetReady()
    {
        _is_ready = true;
    }

private:
    am_thread_pool_task_awaitable_proc _func;
    am_voidptr _param;

    bool _is_ready = false;
};

static std::map<CPoolTask*, std::shared_ptr<CPoolTask>> g_pool_tasks = {};
static std::map<CAwaitablePoolTask*, std::shared_ptr<CAwaitablePoolTask>> g_awaitable_pool_tasks = {};

extern "C" {
AM_API_PUBLIC am_mutex_handle am_thread_mutex_create(am_uint64 spin_count)
{
    return reinterpret_cast<am_mutex_handle>(Thread::CreateMutex(spin_count));
}

AM_API_PUBLIC void am_thread_mutex_destroy(am_mutex_handle handle)
{
    Thread::DestroyMutex(handle);
}

AM_API_PUBLIC void am_thread_mutex_lock(am_mutex_handle handle)
{
    Thread::LockMutex(handle);
}

AM_API_PUBLIC void am_thread_mutex_unlock(am_mutex_handle handle)
{
    Thread::UnlockMutex(handle);
}

AM_API_PUBLIC am_thread_handle am_thread_create(am_thread_proc func, am_voidptr param)
{
    return reinterpret_cast<am_thread_handle>(Thread::CreateThread(func, param));
}

AM_API_PUBLIC void am_thread_sleep(am_int32 ms)
{
    Thread::Sleep(ms);
}

AM_API_PUBLIC void am_thread_wait(am_thread_handle thread)
{
    Thread::Wait(thread);
}

AM_API_PUBLIC void am_thread_release(am_thread_handle thread)
{
    Thread::Release(thread);
}

AM_API_PUBLIC am_thread_id am_thread_get_id()
{
    return Thread::GetCurrentThreadId();
}

AM_API_PUBLIC am_thread_pool_task_handle am_thread_pool_task_create(am_thread_pool_task_proc func, am_voidptr param)
{
    auto* task = amnew(CPoolTask, func, param);
    g_pool_tasks.emplace(task, std::shared_ptr<CPoolTask>(task, am_delete<MemoryPoolKind::Default, CPoolTask>{}));

    return reinterpret_cast<am_thread_pool_task_handle>(task);
}

AM_API_PUBLIC am_thread_pool_task_awaitable_handle
am_thread_pool_task_awaitable_create(am_thread_pool_task_awaitable_proc func, am_voidptr param)
{
    auto* task = amnew(CAwaitablePoolTask, func, param);
    g_awaitable_pool_tasks.emplace(
        task, std::shared_ptr<CAwaitablePoolTask>(task, am_delete<MemoryPoolKind::Default, CAwaitablePoolTask>{}));

    return reinterpret_cast<am_thread_pool_task_awaitable_handle>(task);
}

AM_API_PUBLIC void am_thread_pool_task_destroy(am_thread_pool_task_handle task)
{
    auto* t = reinterpret_cast<CPoolTask*>(task);
    g_pool_tasks.erase(t);
}

AM_API_PUBLIC void am_thread_pool_task_awaitable_destroy(am_thread_pool_task_awaitable_handle task)
{
    auto* t = reinterpret_cast<CAwaitablePoolTask*>(task);
    g_awaitable_pool_tasks.erase(t);
}

AM_API_PUBLIC am_bool am_thread_pool_task_get_ready(am_thread_pool_task_handle task)
{
    return BOOL_TO_AM_BOOL(reinterpret_cast<CPoolTask*>(task)->Ready());
}

AM_API_PUBLIC am_bool am_thread_pool_task_awaitable_get_ready(am_thread_pool_task_awaitable_handle task)
{
    return BOOL_TO_AM_BOOL(reinterpret_cast<CAwaitablePoolTask*>(task)->Ready());
}

AM_API_PUBLIC void am_thread_pool_task_set_ready(am_thread_pool_task_handle task)
{
    reinterpret_cast<CPoolTask*>(task)->SetReady();
}

AM_API_PUBLIC void am_thread_pool_task_awaitable_set_ready(am_thread_pool_task_awaitable_handle task)
{
    reinterpret_cast<CAwaitablePoolTask*>(task)->SetReady();
}

AM_API_PUBLIC void am_thread_pool_task_awaitable_await(am_thread_pool_task_awaitable_handle task)
{
    reinterpret_cast<CAwaitablePoolTask*>(task)->Await();
}

AM_API_PUBLIC void am_thread_pool_task_awaitable_await_for(am_thread_pool_task_awaitable_handle task, am_uint64 ms)
{
    reinterpret_cast<CAwaitablePoolTask*>(task)->Await(ms);
}

AM_API_PUBLIC am_thread_pool_handle am_thread_pool_create(am_uint32 thread_count)
{
    auto* pool = amnew(Thread::Pool);
    pool->Init(thread_count);

    return reinterpret_cast<am_thread_pool_handle>(pool);
}

AM_API_PUBLIC void am_thread_pool_destroy(am_thread_pool_handle pool)
{
    amdelete(Pool, (Thread::Pool*)pool);
}

AM_API_PUBLIC void am_thread_pool_add_task(am_thread_pool_handle pool, am_thread_pool_task_handle task)
{
    auto* t = reinterpret_cast<CPoolTask*>(task);
    if (!g_pool_tasks.contains(t))
        return;

    reinterpret_cast<Thread::Pool*>(pool)->AddTask(g_pool_tasks.at(t));
}

AM_API_PUBLIC void am_thread_pool_add_task_awaitable(am_thread_pool_handle pool, am_thread_pool_task_awaitable_handle task)
{
    auto* t = reinterpret_cast<CAwaitablePoolTask*>(task);
    if (!g_awaitable_pool_tasks.contains(t))
        return;

    reinterpret_cast<Thread::Pool*>(pool)->AddTask(g_awaitable_pool_tasks.at(t));
}

AM_API_PUBLIC am_uint32 am_thread_pool_get_thread_count(am_thread_pool_handle pool)
{
    return reinterpret_cast<Thread::Pool*>(pool)->GetThreadCount();
}

AM_API_PUBLIC am_uint32 am_thread_pool_is_running(am_thread_pool_handle pool)
{
    return reinterpret_cast<Thread::Pool*>(pool)->IsRunning();
}

AM_API_PUBLIC am_uint32 am_thread_pool_has_tasks(am_thread_pool_handle pool)
{
    return reinterpret_cast<Thread::Pool*>(pool)->HasTasks();
}
}