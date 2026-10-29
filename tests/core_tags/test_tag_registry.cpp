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

#include <Core/TagRegistry.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        AmTagID ConstantHash(std::string_view)
        {
            return 7;
        }

        AmTagID ZeroHash(std::string_view)
        {
            return 0;
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, core_tags, tag_registry)
    {
    public:
        void Run() override
        {
            {
                TagRegistry registry;
                AmTagID first = 0, second = 0;
                AM_EXPECT(registry.Register("a", first));
                AM_EXPECT(registry.Register("a", second));
                AM_EXPECT_EQ(first, second);
                AM_EXPECT_EQ(first, CreateTagId("a"));
                AM_EXPECT_EQ(registry.Size(), static_cast<AmSize>(1));

                AmTagID empty = 0;
                AM_EXPECT_NOT(registry.Register("", empty));
            }

            {
                TagRegistry registry(&ConstantHash);
                AmTagID id = 0;
                AM_EXPECT(registry.Register("x", id));
                AM_EXPECT_EQ(id, static_cast<AmTagID>(7));
                AM_EXPECT_NOT(registry.Register("y", id));

                registry.Clear();
                AM_EXPECT_EQ(registry.Size(), static_cast<AmSize>(0));
                AM_EXPECT(registry.Register("y", id));
            }

            {
                TagRegistry registry(&ZeroHash);
                AmTagID id = 0;
                AM_EXPECT_NOT(registry.Register("x", id));
            }
        }
    };

    AM_REGISTER_TEST(core_tags, tag_registry);
} // namespace SparkyStudios::Audio::Amplitude::Tests
