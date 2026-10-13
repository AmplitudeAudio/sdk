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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        // Fills every slot using Add, then checks Get and Remove still work on the full pool.
        template<typename Handle, typename AddFn, typename GetFn, typename RemoveFn>
        bool FillAndCheck(AddFn add, GetFn get, RemoveFn remove)
        {
            constexpr AmObjectID kMaxProbe = 100000;

            AmObjectID count = 0;
            for (AmObjectID id = 1; id <= kMaxProbe; ++id)
            {
                if (!add(id).Valid())
                    break;
                count++;
            }

            if (count == 0 || count == kMaxProbe)
                return false;

            // Pool is now full: Get must still find the last added object.
            if (!get(count).Valid())
                return false;

            // Remove must free the slot even though the free list is empty.
            remove(count);
            if (get(count).Valid())
                return false;

            // The freed slot is reusable.
            return add(count + 1).Valid();
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_engine, get_and_remove_work_when_all_slots_are_used)
    {
    public:
        void Run() override
        {
            AM_EXPECT(
                FillAndCheck<Listener>(
                    [](AmObjectID id)
                    {
                        return amEngine->AddListener(id);
                    },
                    [](AmObjectID id)
                    {
                        return amEngine->GetListener(id);
                    },
                    [](AmObjectID id)
                    {
                        amEngine->RemoveListener(static_cast<AmListenerID>(id));
                    }));

            AM_EXPECT(
                FillAndCheck<Environment>(
                    [](AmObjectID id)
                    {
                        return amEngine->AddEnvironment(id);
                    },
                    [](AmObjectID id)
                    {
                        return amEngine->GetEnvironment(id);
                    },
                    [](AmObjectID id)
                    {
                        amEngine->RemoveEnvironment(static_cast<AmEnvironmentID>(id));
                    }));

            AM_EXPECT(
                FillAndCheck<Room>(
                    [](AmObjectID id)
                    {
                        return amEngine->AddRoom(id);
                    },
                    [](AmObjectID id)
                    {
                        return amEngine->GetRoom(id);
                    },
                    [](AmObjectID id)
                    {
                        amEngine->RemoveRoom(static_cast<AmRoomID>(id));
                    }));
        }
    };

    AM_REGISTER_TEST(core_engine, get_and_remove_work_when_all_slots_are_used);
} // namespace SparkyStudios::Audio::Amplitude::Tests
