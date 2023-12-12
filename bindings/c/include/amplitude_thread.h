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

#ifndef SS_AMPLITUDE_THREAD_H
#define SS_AMPLITUDE_THREAD_H

#include "amplitude_common.h"

typedef am_voidptr am_mutex_handle;
typedef am_voidptr am_thread_handle;
typedef am_uint64 am_thread_id;
typedef void (*am_thread_proc)(am_voidptr param);

struct am_thread_pool_task;
typedef struct am_thread_pool_task am_thread_pool_task;
typedef am_thread_pool_task* am_thread_pool_task_handle;
typedef void (*am_thread_pool_task_proc)(am_thread_pool_task_handle task, am_voidptr param);

struct am_thread_pool_task_awaitable;
typedef struct am_thread_pool_task_awaitable am_thread_pool_task_awaitable;
typedef am_thread_pool_task_awaitable* am_thread_pool_task_awaitable_handle;
typedef void (*am_thread_pool_task_awaitable_proc)(am_thread_pool_task_awaitable_handle task, am_voidptr param);

struct am_thread_pool;
typedef struct am_thread_pool am_thread_pool;
typedef am_thread_pool* am_thread_pool_handle;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a mutex object.
 *
 * A mutex is an object that a thread can acquire, preventing other
 * threads from acquiring it.
 *
 * To acquire the mutex ownership, you should use LockMutex() with
 * the mutex handle as parameter. To release the ownership, use UnlockMutex()
 * with the mutex handle as parameter.
 */
am_mutex_handle am_thread_mutex_create(am_uint64 spin_count);

/**
 * @brief Destroys a mutex object.
 *
 * @param handle The mutex object handle.
 */
void am_thread_mutex_destroy(am_mutex_handle handle);

/**
 * @brief Takes ownership of a mutex.
 *
 * @param handle The mutex object handle.
 */
void am_thread_mutex_lock(am_mutex_handle handle);

/**
 * @brief Releases ownership of a mutex.
 *
 * @param handle The mutex object handle.
 */
void am_thread_mutex_unlock(am_mutex_handle handle);

/**
 * @brief Creates a new thread.
 *
 * @param func The function to run in the thread.
 * @param param An optional shared data to pass to the thread
 */
am_thread_handle am_thread_create(am_thread_proc func, am_voidptr param);

/**
 * @brief Makes the calling thread sleep for the given amount of milliseconds.
 *
 * @param ms The amount of time the calling thread should sleep.
 */
void am_thread_sleep(am_int32 ms);

/**
 * @brief Waits for the given thread to stop.
 *
 * @param thread The handle of the thread to wait.
 */
void am_thread_wait(am_thread_handle thread);

/**
 * @brief Manually stops a thread execution.
 *
 * @param thread The handle of the thread to wait.
 */
void am_thread_release(am_thread_handle thread);

/**
 * @brief Gets the handle of the calling thread.
 */
am_thread_id am_thread_get_id();

/**
 * @brief Creates a pool task.
 *
 * @param func The function to execute in the task.
 * @param param An optional parameter to pass to the task function.
 *
 * @return The handle to the created pool task.
 */
am_thread_pool_task_handle am_thread_pool_task_create(am_thread_pool_task_proc func, am_voidptr param);

/**
 * @brief Creates a pool task that can be awaited.
 *
 * The task can be awaited after being added to a pool by calling @c am_thread_pool_task_awaitable_await()
 * or @c am_thread_pool_task_awaitable_await_for() functions.
 *
 * @param func The function to execute in the task.
 * @param param An optional parameter to pass to the task function.
 *
 * @return The handle to the created pool task.
 */
am_thread_pool_task_awaitable_handle am_thread_pool_task_awaitable_create(am_thread_pool_task_awaitable_proc func, am_voidptr param);

/**
 * @brief Destroys the pool task with the given handle.
 *
 * The pool task may not be destroyed if it was added to a pool and the pool
 * have not yet processed it.
 *
 * @param task The handle of the task to destroy.
 */
void am_thread_pool_task_destroy(am_thread_pool_task_handle task);

/**
 * @brief Destroys the pool task with the given handle.
 *
 * The pool task may not be destroyed if it was added to a pool and the pool
 * have not yet processed it.
 *
 * @param task The handle of the task to destroy.
 */
void am_thread_pool_task_awaitable_destroy(am_thread_pool_task_awaitable_handle task);

/**
 * @brief Checks if the pool task is ready.
 *
 * @param task The handle of the task to check.
 *
 * @return @c AM_TRUE if the pool task is ready, @c AM_FALSE otherwise.
 */
am_bool am_thread_pool_task_get_ready(am_thread_pool_task_handle task);

/**
 * @brief Checks if the pool task is ready.
 *
 * @param task The handle of the task to check.
 *
 * @return @c AM_TRUE if the pool task is ready, @c AM_FALSE otherwise.
 */
am_bool am_thread_pool_task_awaitable_get_ready(am_thread_pool_task_awaitable_handle task);

/**
 * @brief Marks a pool task as ready.
 *
 * A ready pool task can be processed by the pool it is added in.
 *
 * @param task The handle of the task to mark as ready.
 */
void am_thread_pool_task_set_ready(am_thread_pool_task_handle task);

/**
 * @brief Marks a pool task as ready.
 *
 * A ready pool task can be processed by the pool it is added in.
 *
 * @param task The handle of the task to mark as ready.
 */
void am_thread_pool_task_awaitable_set_ready(am_thread_pool_task_awaitable_handle task);

/**
 * @brief Makes the calling thread wait for this task to finish.
 *
 * @param task The handle of the task to wait for.
 */
void am_thread_pool_task_awaitable_await(am_thread_pool_task_awaitable_handle task);

/**
 * @brief Makes the calling thread wait for this task to finish.
 *
 * @param task The handle of the task to wait for.
 * @param ms The maximum amount of time to wait in milliseconds.
 */
void am_thread_pool_task_awaitable_await_for(am_thread_pool_task_awaitable_handle task, am_uint64 ms);

/**
 * @brief Creates a new pool tasks scheduler.
 *
 * The Pool tasks scheduler can pick and run pool tasks on several multiple
 * threads. The number of threads is defined at initialization.
 *
 * The maximum number of tasks the pool can manage is defined by the @c AM_MAX_THREAD_POOL_TASKS
 * macro. The default value is 1024.
 *
 * @param thread_count The number of threads to run the pool tasks on.
 */
am_thread_pool_handle am_thread_pool_create(am_uint32 thread_count);

/**
 * @brief Destroys a pool and release all the associated threads.
 *
 * @param pool The pool to destroy.
 */
void am_thread_pool_destroy(am_thread_pool_handle pool);

/**
 * @brief Add a task in the given pool.
 *
 * @param pool The pool in which the task is to be added.
 * @param task The task to add. The task is not automatically deleted when the work is done.
 */
void am_thread_pool_add_task(am_thread_pool_handle pool, am_thread_pool_task_handle task);

/**
 * @brief Add a task in the given pool.
 *
 * @param pool The pool in which the task is to be added.
 * @param task The task to add. The task is not automatically deleted when the work is done.
 */
void am_thread_pool_add_task_awaitable(am_thread_pool_handle pool, am_thread_pool_task_awaitable_handle task);

/**
 * @brief Gets the number of threads this pool is using.
 *
 * @param pool The pool.
 *
 * @return The number of threads this pool is using.
 */
am_uint32 am_thread_pool_get_thread_count(am_thread_pool_handle pool);

/**
 * @brief Indicates that the pool is running.
 *
 * @param pool The pool.
 *
 * @return @c AM_TRUE if the pool is running, @c AM_FALSE otherwise.
 */
am_bool am_thread_pool_is_running(am_thread_pool_handle pool);

/**
 * @brief Indicates that the pool has tasks pending.
 *
 * @param pool The pool.
 *
 * @return @c AM_TRUE if the pool is running, @c AM_FALSE otherwise.
 */
am_bool am_thread_pool_has_tasks(am_thread_pool_handle pool);

#ifdef __cplusplus
}
#endif

#endif // SS_AMPLITUDE_THREAD_H
