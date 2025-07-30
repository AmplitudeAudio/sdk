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

#pragma once

#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AwaitableDummyPoolTask final : public Thread::AwaitablePoolTask
    {
    public:
        void AwaitableWork() override
        {
            Thread::Sleep(1000);
            _isExecuted = true;
        }

        [[nodiscard]] bool IsExecuted() const
        {
            return _isExecuted;
        }

    private:
        bool _isExecuted = false;
    };
} // namespace SparkyStudios::Audio::Amplitude
