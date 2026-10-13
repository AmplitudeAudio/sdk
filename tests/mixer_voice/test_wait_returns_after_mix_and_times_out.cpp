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

#include <chrono>
#include <thread>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Amplimix::Wait() polls an atomic flag the audio thread clears without locking: it returns at once when no mix is
    // running, returns once the flag clears, and gives up after its timeout when it never does.
    AM_TEST_CASE(EngineTestCase, mixer_voice, wait_returns_after_mix_and_times_out)
    {
    public:
        void Run() override
        {
            using Clock = std::chrono::steady_clock;
            AmplimixImpl& mixer = amEngine->GetState()->mixer;

            // Let the engine mix a few blocks while waiting: Wait() must never get stuck on a live mixer.
            Channel channel = amEngine->Play("test_sound_06");
            amEngine->WaitUntilFrames(2);

            for (int i = 0; i < 20; ++i)
                AM_EXPECT(mixer.Wait(std::chrono::milliseconds(5000)));

            channel.Stop();
            amEngine->WaitUntilFrames(2);

            // A paused engine skips Mix(): the audio thread stops touching the flag, so the checks below own it.
            amEngine->Pause(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // No mix in progress: returns immediately.
            mixer.SetMixInProgressForTesting(false);
            auto start = Clock::now();
            AM_EXPECT(mixer.Wait(std::chrono::milliseconds(5000)));
            AM_EXPECT(Clock::now() - start < std::chrono::milliseconds(4000));

            // A mix that completes while waiting releases the waiter before the timeout.
            mixer.SetMixInProgressForTesting(true);
            std::thread finisher(
                [&mixer]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    mixer.SetMixInProgressForTesting(false);
                });
            start = Clock::now();
            AM_EXPECT(mixer.Wait(std::chrono::milliseconds(8000)));
            const auto waited = Clock::now() - start;
            finisher.join();
            AM_EXPECT(waited >= std::chrono::milliseconds(40));
            AM_EXPECT(waited < std::chrono::milliseconds(4000));

            // A mix that never completes: Wait() gives up after the timeout, neither earlier nor much later.
            mixer.SetMixInProgressForTesting(true);
            start = Clock::now();
            AM_EXPECT_NOT(mixer.Wait(std::chrono::milliseconds(150)));
            const auto timedOut = Clock::now() - start;
            AM_EXPECT(timedOut >= std::chrono::milliseconds(150));
            AM_EXPECT(timedOut < std::chrono::milliseconds(4000));

            mixer.SetMixInProgressForTesting(false);
            amEngine->Pause(false);
        }
    };

    AM_REGISTER_TEST(mixer_voice, wait_returns_after_mix_and_times_out);
} // namespace SparkyStudios::Audio::Amplitude::Tests
