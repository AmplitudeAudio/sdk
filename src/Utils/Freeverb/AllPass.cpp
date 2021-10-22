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
//
// Based on code written by Jezar at Dreampoint, June 2000 http://www.dreampoint.co.uk,
// which was placed in public domain.

#include "AllPass.h"

namespace Freeverb
{
    AllPass::AllPass()
    {
        _bufferIndex = 0;
    }

    void AllPass::SetBuffer(AmReal32Buffer buf, AmInt32 size)
    {
        _buffer = buf;
        _bufferSize = size;
    }

    void AllPass::Mute()
    {
        for (AmInt32 i = 0; i < _bufferSize; i++)
            _buffer[i] = 0;
    }

    void AllPass::SetFeedback(AmReal32 val)
    {
        _feedback = val;
    }

    AmReal32 AllPass::GetFeedback()
    {
        return _feedback;
    }
} // namespace Freeverb
