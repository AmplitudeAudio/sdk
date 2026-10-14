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

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/InstanceCursors.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        ChannelInstanceData MakeInstance(AmChannelInstanceID id, AmUInt64 cursor)
        {
            ChannelInstanceData data{};
            data.instanceId = id;
            data.cursor = cursor;

            return data;
        }

        std::vector<AmUInt64> Cursors(const std::vector<ChannelInstanceData>& instances)
        {
            std::vector<AmUInt64> result;
            for (const auto& instance : instances)
                result.push_back(instance.cursor);

            return result;
        }

        // What the audio thread did before: for each new instance, look it up in the previous ones.
        void CarryOverQuadratic(const std::vector<ChannelInstanceData>& previous, std::vector<ChannelInstanceData>& current)
        {
            for (auto& instance : current)
            {
                for (const auto& prev : previous)
                {
                    if (prev.instanceId == instance.instanceId)
                    {
                        instance.cursor = prev.cursor;
                        break;
                    }
                }
            }
        }
    } // namespace

    // The layer advances the cursors of its instances between snapshots; a new snapshot only knows their initial
    // value. Instances that were already there must keep the layer's cursor.
    AM_TEST_CASE(ComponentTestCase, mixer_instances, carry_over_instance_cursors)
    {
    public:
        void Run() override
        {
            // Same instances: every cursor is carried over.
            {
                const std::vector previous = { MakeInstance(1, 10), MakeInstance(2, 20), MakeInstance(3, 30) };
                std::vector current = { MakeInstance(1, 0), MakeInstance(2, 0), MakeInstance(3, 0) };

                CarryOverInstanceCursors(previous, current);
                AM_EXPECT(Cursors(current) == std::vector<AmUInt64>({ 10, 20, 30 }));
            }

            // An instance was removed from the middle: the ones after it keep their own cursor, not their neighbour's.
            {
                const std::vector previous = { MakeInstance(1, 10), MakeInstance(2, 20), MakeInstance(3, 30) };
                std::vector current = { MakeInstance(1, 0), MakeInstance(3, 0) };

                CarryOverInstanceCursors(previous, current);
                AM_EXPECT(Cursors(current) == std::vector<AmUInt64>({ 10, 30 }));
            }

            // An instance was appended: it keeps the initial cursor it came with.
            {
                const std::vector previous = { MakeInstance(1, 10), MakeInstance(2, 20) };
                std::vector current = { MakeInstance(1, 0), MakeInstance(2, 0), MakeInstance(7, 5) };

                CarryOverInstanceCursors(previous, current);
                AM_EXPECT(Cursors(current) == std::vector<AmUInt64>({ 10, 20, 5 }));
            }

            // Removals, then an addition, in the same snapshot.
            {
                const std::vector previous = { MakeInstance(1, 10), MakeInstance(2, 20), MakeInstance(3, 30), MakeInstance(4, 40) };
                std::vector current = { MakeInstance(2, 0), MakeInstance(4, 0), MakeInstance(5, 9) };

                CarryOverInstanceCursors(previous, current);
                AM_EXPECT(Cursors(current) == std::vector<AmUInt64>({ 20, 40, 9 }));
            }

            // Nothing before, nothing now, nothing in common.
            {
                std::vector current = { MakeInstance(1, 3), MakeInstance(2, 4) };
                CarryOverInstanceCursors({}, current);
                AM_EXPECT(Cursors(current) == std::vector<AmUInt64>({ 3, 4 }));

                std::vector<ChannelInstanceData> none;
                CarryOverInstanceCursors({ MakeInstance(1, 10) }, none);
                AM_EXPECT(none.empty());

                std::vector disjoint = { MakeInstance(8, 1), MakeInstance(9, 2) };
                CarryOverInstanceCursors({ MakeInstance(1, 10), MakeInstance(2, 20) }, disjoint);
                AM_EXPECT(Cursors(disjoint) == std::vector<AmUInt64>({ 1, 2 }));
            }

            // The previous list is only read.
            {
                const std::vector previous = { MakeInstance(1, 10), MakeInstance(2, 20) };
                std::vector current = { MakeInstance(2, 0) };

                CarryOverInstanceCursors(previous, current);
                AM_EXPECT(Cursors(previous) == std::vector<AmUInt64>({ 10, 20 }));
            }
        }
    };

    // Whatever the channel does to its instances, the linear merge must agree with the scan it replaces: both lists
    // are subsets of a growing id space, in ascending order.
    AM_TEST_CASE(ComponentTestCase, mixer_instances, carry_over_instance_cursors_matches_the_quadratic_scan)
    {
    public:
        void Run() override
        {
            AmUInt32 seed = 12345u;
            auto next = [&seed]()
            {
                seed = seed * 1664525u + 1013904223u;
                return seed >> 16;
            };

            for (AmUInt32 round = 0; round < 200; ++round)
            {
                std::vector<ChannelInstanceData> previous;
                std::vector<ChannelInstanceData> current;

                for (AmChannelInstanceID id = 1; id <= 64; ++id)
                {
                    if (next() % 3 != 0)
                        previous.push_back(MakeInstance(id, 1000 + id));

                    if (next() % 3 != 0)
                        current.push_back(MakeInstance(id, id));
                }

                std::vector<ChannelInstanceData> expected = current;
                CarryOverQuadratic(previous, expected);

                CarryOverInstanceCursors(previous, current);
                AM_EXPECT(Cursors(current) == Cursors(expected));
            }
        }
    };

    AM_REGISTER_TEST(mixer_instances, carry_over_instance_cursors);
    AM_REGISTER_TEST(mixer_instances, carry_over_instance_cursors_matches_the_quadratic_scan);
} // namespace SparkyStudios::Audio::Amplitude::Tests
