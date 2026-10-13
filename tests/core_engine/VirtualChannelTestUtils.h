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

#pragma once

#include <atomic>
#include <deque>
#include <utility>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Core/Playback/ChannelInternalState.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /// Counts the events of one channel (fired on the engine thread, read from the test thread).
    struct EventCounter
    {
        std::atomic<int> ended{ 0 };
        std::atomic<int> stopped{ 0 };
        std::atomic<AmUInt64> endClock{ 0 }; ///< The audio clock when End fired.
    };

    /**
     * @brief Returns a counter that lives for the whole process: channel events can still fire while the engine tears
     * down, after the test body (and anything on its stack or heap) is gone.
     */
    inline EventCounter* NewCounter()
    {
        static std::deque<EventCounter> counters;
        return &counters.emplace_back();
    }

    inline void CountEnd(ChannelEventInfo info)
    {
        auto* counter = static_cast<EventCounter*>(info.m_userData);
        counter->endClock = amEngine->GetState()->mixer.GetAudioClock();
        ++counter->ended;
    }

    inline void CountStop(ChannelEventInfo info)
    {
        ++static_cast<EventCounter*>(info.m_userData)->stopped;
    }

    /// A channel that lost its real channel to a louder one, with the counters of every channel of the scenario.
    struct StolenScenario
    {
        std::vector<Channel> low;
        std::vector<EventCounter*> counters;
        Channel high;
        Channel* stolen = nullptr;
        EventCounter* stolenCounter = nullptr;
    };

    /**
     * @brief Plays @p holder on the first 49 real channels and @p tail on the 50th (the lowest in the priority list),
     * then a louder @p holder that takes a real channel from the tail: that one is left virtual (@c stolen).
     *
     * With a looping @p holder no real channel frees up, so the stolen channel stays virtual until its own sound ends.
     */
    template<typename Wait>
    inline bool StartStolenScenario(StolenScenario& scenario, SoundHandle holder, SoundHandle tail, Wait&& waitFrames)
    {
        for (int i = 0; i < 50; ++i)
        {
            scenario.counters.push_back(NewCounter());
            scenario.low.push_back(amEngine->Play(i == 49 ? tail : holder, AmVector3{ 0.0f, 0.0f, 0.0f }, 0.5f));
            scenario.low.back().On(eChannelEvent_End, &CountEnd, scenario.counters.back());
            scenario.low.back().On(eChannelEvent_Stop, &CountStop, scenario.counters.back());
        }

        waitFrames(2);

        scenario.high = amEngine->Play(holder, AmVector3{ 0.0f, 0.0f, 0.0f }, 1.0f);
        waitFrames(2);

        for (std::size_t i = 0; i < scenario.low.size(); ++i)
        {
            if (!scenario.low[i].GetState()->IsReal())
            {
                scenario.stolen = &scenario.low[i];
                scenario.stolenCounter = scenario.counters[i];
            }
        }

        return scenario.stolen != nullptr;
    }

    template<typename Wait>
    inline bool StartStolenScenario(StolenScenario& scenario, SoundHandle sound, Wait&& waitFrames)
    {
        return StartStolenScenario(scenario, sound, sound, std::forward<Wait>(waitFrames));
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
