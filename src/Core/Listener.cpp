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

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>

#include <Core/ListenerInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    void Listener::Clear()
    {
        _state = nullptr;
    }

    bool Listener::Valid() const
    {
        return _state != nullptr && _state->node.in_list();
    }

    void Listener::SetOrientation(const hmm_vec3& location, const hmm_vec3& direction, const hmm_vec3& up)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetInverseMatrix(AM_LookAt(location, location + direction, up));
    }

    hmm_vec3 Listener::GetLocation() const
    {
        hmm_mat4 matrix = _state->GetInverseMatrix();
        return AM_Vec3(-matrix.Elements[3][0], -matrix.Elements[3][1], -matrix.Elements[3][2]);
    }

    void Listener::SetLocation(const hmm_vec3& location)
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetInverseMatrix(AM_Translate(-location));
    }

    void Listener::SetMatrix(const hmm_mat4& matrix)
    {
        AMPLITUDE_ASSERT(Valid());
        // TODO: _state->SetInverseMatrix(AM_Translate(matrix));
    }

    hmm_mat4 Listener::GetMatrix() const
    {
        return AM_Mat4d(1.0f); // TODO
    }

} // namespace SparkyStudios::Audio::Amplitude
