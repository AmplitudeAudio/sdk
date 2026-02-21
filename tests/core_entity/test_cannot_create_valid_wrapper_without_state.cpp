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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EntityInternalState.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_entity, cannot_create_valid_wrapper_without_state)
    {
    public:
        void Run() override
        {
            EntityInternalState state;
            state.SetId(1);

            fplutil::intrusive_list entity_list(&EntityInternalState::node);
            entity_list.push_back(state);

            Entity wrapper2(nullptr);
            AM_EXPECT_NOT(wrapper2.Valid());

            Entity wrapper3;
            AM_EXPECT_NOT(wrapper3.Valid());
        }
    };

    AM_REGISTER_TEST(core_entity, cannot_create_valid_wrapper_without_state);
} // namespace SparkyStudios::Audio::Amplitude::Tests
