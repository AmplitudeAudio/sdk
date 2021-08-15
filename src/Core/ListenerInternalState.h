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

#ifndef SPARK_AUDIO_LISTENER_INTERNAL_STATE_H
#define SPARK_AUDIO_LISTENER_INTERNAL_STATE_H

#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>

#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ListenerInternalState
    {
    public:
        ListenerInternalState()
            : _inverseMatrix(AM_Mat4d(1))
        {}

        void SetInverseMatrix(const hmm_mat4& matrix)
        {
            _inverseMatrix = matrix;
        }

        hmm_mat4& GetInverseMatrix()
        {
            return _inverseMatrix;
        }

        const hmm_mat4& GetInverseMatrix() const
        {
            return _inverseMatrix;
        }

        fplutil::intrusive_list_node node;

    private:
        hmm_mat4 _inverseMatrix;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LISTENER_INTERNAL_STATE_H
