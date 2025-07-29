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

#include <Core/EntityInternalState.h>
#include <Core/ListenerInternalState.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    fplutil::intrusive_list listener_list(&ListenerInternalState::node);
    fplutil::intrusive_list entity_list(&EntityInternalState::node);

    Listener InitTestListener(ListenerInternalState* state)
    {
        state->SetId(1);

        listener_list.push_back(*state);

        Listener wrapper(state);

        return wrapper;
    }

    Entity InitTestEntity(EntityInternalState* state)
    {
        state->SetId(1);

        entity_list.push_back(*state);

        Entity wrapper(state);

        return wrapper;
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
