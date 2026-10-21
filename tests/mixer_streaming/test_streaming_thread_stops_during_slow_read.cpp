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

#include <atomic>
#include <chrono>
#include <future>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamingThread.h>

#include "ComponentTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        /// Counts Close() calls and the moment of the first one.
        class CountingDecoder final : public Codec::Decoder
        {
        public:
            CountingDecoder(std::atomic<int>& closeCount, std::atomic<bool>& stopReturned, std::atomic<bool>& closedBeforeStop)
                : Decoder(nullptr)
                , _closeCount(closeCount)
                , _stopReturned(stopReturned)
                , _closedBeforeStop(closedBeforeStop)
            {
                m_format.SetAll(48000, 2, 32, 48000 * 10, 2 * sizeof(AmReal32), eAudioSampleFormat_Float32);
            }

            bool Open(std::shared_ptr<File>) override
            {
                return true;
            }

            bool Close() override
            {
                if (!_stopReturned.load())
                    _closedBeforeStop.store(true);
                _closeCount.fetch_add(1);
                return true;
            }

            AmUInt64 Load(AudioBuffer*) override
            {
                return 0;
            }

            AmUInt64 Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64, AmUInt64 length) override
            {
                Thread::Sleep(200);
                for (AmUInt16 c = 0; c < 2; ++c)
                    for (AmUInt64 i = 0; i < length; ++i)
                        (*out)[c][bufferOffset + i] = 0.0f;
                return length;
            }

            bool Seek(AmUInt64) override
            {
                return true;
            }

        private:
            std::atomic<int>& _closeCount;
            std::atomic<bool>& _stopReturned;
            std::atomic<bool>& _closedBeforeStop;
        };
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_stops_during_slow_read)
    {
    public:
        void Run() override
        {
            std::atomic<int> closeCount{ 0 };
            std::atomic<bool> stopReturned{ false };
            std::atomic<bool> closedBeforeStop{ false };

            StreamingThread thread({ 100.0, 20.0, false, 256 });
            thread.Start();

            auto decoder = std::make_shared<CountingDecoder>(closeCount, stopReturned, closedBeforeStop);
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000 * 10;

            // The worker is already running here, so the first fill happens on the worker, inside the slow Stream() call.
            StreamSource* source = thread.Create(decoder, plan, 2, 48000);
            Thread::Sleep(20); // the worker is inside the 200 ms Stream() call

            thread.Retire(source);

            auto stopped = std::async(
                std::launch::async,
                [&]()
                {
                    thread.Stop();
                });
            AM_EXPECT(stopped.wait_for(std::chrono::seconds(2)) == std::future_status::ready);
            stopReturned.store(true);

            // The decoder must not close while the worker could still use it.
            AM_EXPECT(!closedBeforeStop.load());
            AM_EXPECT_EQ(0, closeCount.load());

            thread.CollectRetired();
            AM_EXPECT_EQ(1, closeCount.load());

            thread.CollectRetired();
            AM_EXPECT_EQ(1, closeCount.load());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_stops_during_slow_read);
} // namespace SparkyStudios::Audio::Amplitude::Tests
