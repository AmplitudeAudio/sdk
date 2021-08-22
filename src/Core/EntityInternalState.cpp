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

namespace SparkyStudios::Audio::Amplitude
{
    void EntityInternalState::SetLocation(const hmm_vec3& location)
    {
        _location = location;
    }

    const hmm_vec3& EntityInternalState::GetLocation() const
    {
        return _location;
    }

    void EntityInternalState::SetOrientation(const hmm_vec3& direction, const hmm_vec3& up)
    {
        _direction = direction;
        _up = up;
    }

    void EntityInternalState::Update()
    {
        _inverseMatrix = AM_LookAt(_location, _location + _direction, _up);
    }
} // namespace SparkyStudios::Audio::Amplitude
