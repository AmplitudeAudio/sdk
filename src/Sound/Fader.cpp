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

#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    Fader::Fader()
    {
        m_current = m_from = m_to = m_delta = 0;
        m_time = m_startTime = m_endTime = 0;
        m_state = AM_FADER_STATE_DISABLED;
    }

    void Fader::Set(float from, float to, AmTime time)
    {
        Start(0.0);
        m_current = m_from;
        m_from = from;
        m_to = to;
        m_time = time;
        m_delta = to - from;
        m_state = AM_FADER_STATE_ACTIVE;
    }

    float Fader::Get(AmTime currentTime)
    {
        if (m_startTime > currentTime)
        {
            // Time rolled over.
            // Figure out where we were...
            float p = (m_current - m_from) / m_delta; // 0..1
            m_from = m_current;
            m_startTime = currentTime;
            m_time = m_time * (1 - p); // time left
            m_delta = m_to - m_from;
            m_endTime = m_startTime + m_time;
        }

        if (currentTime > m_endTime)
        {
            m_state = AM_FADER_STATE_STOPPED;
            return m_to;
        }

        m_current = (float)(m_from + m_delta * ((currentTime - m_startTime) / m_time));
        return m_current;
    }

    void Fader::Start(AmTime time)
    {
        m_startTime = time;
        m_endTime = m_startTime + m_time;
    }
} // namespace SparkyStudios::Audio::Amplitude