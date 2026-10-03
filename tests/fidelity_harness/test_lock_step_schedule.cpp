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

#include <Fidelity/OfflineDriver.h>
#include <Fidelity/RenderSession.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, lock_step_schedule)
    {
    public:
        void Run() override
        {
            // 60 fps at 48 kHz = a frame every 800 samples; blocks of 1024; the frame wins the tie at 0.
            {
                RenderSettings settings;
                LockStepSchedule schedule(settings, 48000);

                std::vector<ScheduleEvent> events;
                for (std::size_t i = 0; i < 9; ++i)
                    events.push_back(schedule.Next());

                AM_EXPECT(events[0].kind == MarkerKind::Frame && events[0].sample == 0.0);
                AM_EXPECT(std::abs(events[0].deltaMs - 1000.0 / 60.0) < 1e-9);
                AM_EXPECT(events[1].kind == MarkerKind::Block && events[1].sample == 0.0 && events[1].frames == 1024);
                AM_EXPECT(events[2].kind == MarkerKind::Frame && events[2].sample == 800.0);
                AM_EXPECT(events[3].kind == MarkerKind::Block && events[3].sample == 1024.0);
                AM_EXPECT(events[4].kind == MarkerKind::Frame && events[4].sample == 1600.0);
                AM_EXPECT(events[5].kind == MarkerKind::Block && events[5].sample == 2048.0);
                AM_EXPECT(events[6].kind == MarkerKind::Frame && events[6].sample == 2400.0);
                AM_EXPECT(events[7].kind == MarkerKind::Block && events[7].sample == 3072.0);
                AM_EXPECT(events[8].kind == MarkerKind::Frame && events[8].sample == 3200.0);
            }

            // Jitter varies the periods but keeps the mean rate exact over pairs.
            {
                RenderSettings settings;
                settings.jitter = true;
                settings.blockSize = 1u << 30; // keep blocks out of the way
                LockStepSchedule schedule(settings, 48000);

                double minPeriod = 1e9;
                double maxPeriod = 0.0;
                double previous = schedule.Next().sample; // frame 0
                AM_EXPECT_EQ(schedule.Next().kind, MarkerKind::Block);
                double last = previous;
                for (std::size_t i = 0; i < 1000; ++i)
                {
                    const ScheduleEvent frame = schedule.Next();
                    AM_EXPECT_EQ(frame.kind, MarkerKind::Frame);
                    minPeriod = std::min(minPeriod, frame.sample - previous);
                    maxPeriod = std::max(maxPeriod, frame.sample - previous);
                    previous = frame.sample;
                    last = frame.sample;
                }

                AM_EXPECT(std::abs(last - 1000.0 * 800.0) < 1e-6);
                AM_EXPECT(minPeriod < 0.9 * 800.0);
                AM_EXPECT(maxPeriod > 1.1 * 800.0);
            }

            // A block sequence cycles through its frame counts.
            {
                RenderSettings settings;
                settings.fps = 1e-3; // one frame at 0, then none for a long time
                settings.blockSequence = { 256, 512 };
                LockStepSchedule schedule(settings, 48000);
                AM_EXPECT_EQ(schedule.Next().kind, MarkerKind::Frame);
                const std::vector<double> starts = { 0.0, 256.0, 768.0, 1024.0, 1536.0 };
                for (const double start : starts)
                {
                    const ScheduleEvent block = schedule.Next();
                    AM_EXPECT(block.kind == MarkerKind::Block && block.sample == start);
                }
            }

            OfflineDriver driver;
            AM_EXPECT(driver.GetName() == "offline");
            AM_EXPECT(!driver.IsOpen());
            std::vector<DeviceDescription> devices;
            AM_EXPECT(driver.EnumerateDevices(devices));
        }
    };

    AM_REGISTER_TEST(fidelity_harness, lock_step_schedule);
} // namespace SparkyStudios::Audio::Amplitude::Tests
