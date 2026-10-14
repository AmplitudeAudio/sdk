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

#include <Utils/TripleBuffer.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        struct Payload
        {
            std::vector<AmUInt32> values;
        };

        void Publish(TripleBuffer<Payload>& buffer, AmUInt32 value, AmSize count)
        {
            buffer.Write().values.assign(count, value);
            buffer.Publish();
        }

        bool Holds(const Payload& payload, AmUInt32 value, AmSize count)
        {
            if (payload.values.size() != count)
                return false;

            for (const AmUInt32 v : payload.values)
                if (v != value)
                    return false;

            return true;
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, core_triple_buffer, reader_sees_the_default_value_before_the_first_publish)
    {
    public:
        void Run() override
        {
            TripleBuffer<Payload> buffer;

            AM_EXPECT(buffer.Read().values.empty());
        }
    };

    AM_TEST_CASE(ComponentTestCase, core_triple_buffer, reader_sees_only_the_latest_publish)
    {
    public:
        void Run() override
        {
            TripleBuffer<Payload> buffer;

            Publish(buffer, 1, 4);
            Publish(buffer, 2, 5);
            Publish(buffer, 3, 6);

            AM_EXPECT(Holds(buffer.Read(), 3, 6));

            // Nothing newer: the same value, from the same buffer.
            const Payload* first = &buffer.Read();
            AM_EXPECT(Holds(*first, 3, 6));
            AM_EXPECT(&buffer.Read() == first);
        }
    };

    // The property the double buffer lacked: however many snapshots are published while the reader holds one, the held
    // one is neither modified nor freed.
    AM_TEST_CASE(ComponentTestCase, core_triple_buffer, reader_keeps_its_buffer_across_any_number_of_publishes)
    {
    public:
        void Run() override
        {
            TripleBuffer<Payload> buffer;

            Publish(buffer, 7, 8);

            const Payload& held = buffer.Read();
            const AmUInt32* storage = held.values.data();
            AM_EXPECT(Holds(held, 7, 8));

            // Different sizes force the publisher's vectors to reallocate.
            for (AmUInt32 i = 0; i < 200; ++i)
            {
                Publish(buffer, 100 + i, 1 + (i * 13) % 61);

                AM_EXPECT(Holds(held, 7, 8));
                AM_EXPECT(held.values.data() == storage);
            }

            AM_EXPECT(Holds(buffer.Read(), 299, 1 + (199 * 13) % 61));
        }
    };

    AM_TEST_CASE(ComponentTestCase, core_triple_buffer, publisher_never_writes_the_buffer_the_reader_holds)
    {
    public:
        void Run() override
        {
            TripleBuffer<Payload> buffer;

            // Interleaves publishes and reads in a fixed pseudo-random order.
            AmUInt32 state = 12345;
            for (AmUInt32 step = 0; step < 2000; ++step)
            {
                state = state * 1664525u + 1013904223u;

                if ((state >> 24) % 3 == 0)
                {
                    const Payload* held = &buffer.Read();
                    AM_EXPECT(&buffer.Write() != held);
                }
                else
                {
                    buffer.Write().values.assign(1 + (state >> 20) % 16, step);
                    buffer.Publish();
                }
            }
        }
    };

    AM_REGISTER_TEST(core_triple_buffer, reader_sees_the_default_value_before_the_first_publish);
    AM_REGISTER_TEST(core_triple_buffer, reader_sees_only_the_latest_publish);
    AM_REGISTER_TEST(core_triple_buffer, reader_keeps_its_buffer_across_any_number_of_publishes);
    AM_REGISTER_TEST(core_triple_buffer, publisher_never_writes_the_buffer_the_reader_holds);
} // namespace SparkyStudios::Audio::Amplitude::Tests