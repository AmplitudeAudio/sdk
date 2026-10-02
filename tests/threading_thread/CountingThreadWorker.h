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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <atomic>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /**
     * @brief A thread worker that keeps doing observable work until it is told to stop.
     *
     * The counter makes it possible to tell whether a thread is still running after the handle has
     * been released. Thread::Sleep is used as the only wait primitive so that the worker also reaches
     * a cancellation point on POSIX, where thread termination is cooperative.
     */
    class CountingThreadWorker
    {
    public:
        static void Run(AmVoidPtr param)
        {
            auto* self = static_cast<CountingThreadWorker*>(param);

            while (!self->_stop.load())
            {
                self->_iterations.fetch_add(1);
                Thread::Sleep(1);
            }
        }

        /**
         * @brief Gets how many times the worker loop has run so far.
         */
        [[nodiscard]] AmInt64 GetIterations() const
        {
            return _iterations.load();
        }

        /**
         * @brief Asks the worker to return from its function on the next iteration.
         *
         * Only meaningful for a detached thread, which cannot be joined and therefore has to be given
         * a way to notice that it should stop.
         */
        void Stop()
        {
            _stop.store(true);
        }

    private:
        std::atomic<AmInt64> _iterations{0};
        std::atomic<bool> _stop{false};
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
