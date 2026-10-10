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

#pragma once

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Nodes/SpeakerPanningNode.h>

#include "NodeTestCase.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /// The location @p azimuth degrees from straight ahead (positive to the left), @p distance metres from a listener at
    /// the origin facing -Z with +Y up.
    inline AmVector3 LocationAt(AmReal32 azimuth, AmReal32 distance = 10.0f)
    {
        const AmReal32 radians = azimuth * AM_DegToRad;
        return { -std::sin(radians) * distance, 0.0f, -std::cos(radians) * distance };
    }

    /// Independent uniform noise on every channel, from a fixed linear congruential generator.
    inline AudioBuffer MakeNoise(AmUInt64 frames, AmUInt16 channels, AmUInt32 seed = 1)
    {
        AudioBuffer buffer(frames, channels);
        AmUInt32 state = seed;
        for (AmUInt16 c = 0; c < channels; ++c)
        {
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                state = state * 1664525u + 1013904223u;
                buffer[c][i] = static_cast<AmReal32>(state >> 8) / static_cast<AmReal32>(1u << 24) - 0.5f;
            }
        }

        return buffer;
    }

    inline AmReal64 MeanSquare(const AudioBuffer& buffer, AmUInt16 channel)
    {
        AmReal64 sum = 0.0;
        for (AmUInt64 i = 0; i < buffer.GetFrameCount(); ++i)
            sum += static_cast<AmReal64>(buffer[channel][i]) * buffer[channel][i];

        return sum / static_cast<AmReal64>(buffer.GetFrameCount());
    }

    class SpeakerPanningTestCase : public NodeTestCase
    {
    protected:
        void SetUpListener()
        {
            GetMockLayer().SetSampleRate(48000);
            _listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            _listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            _listenerState.Update();
            GetMockLayer().SetListener(InitTestListener(&_listenerState));
        }

        /// Renders @p input through a fresh panner, whose gains snap on this first block, to the @p output layout.
        AudioBuffer Render(const AudioBuffer& input, eChannelLayout output)
        {
            SpeakerPanningNode node;
            auto instance = node.CreateInstance();
            instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
            instance->SetPipelineOutputLayout(output);
            instance->Configure(input.GetFrameCount(), static_cast<AmUInt16>(input.GetChannelCount()));

            const AudioBuffer* rendered = AsProcessor(instance)->Process(&input);
            AM_EXPECT_NOT(rendered == nullptr);

            AudioBuffer copy(input.GetFrameCount(), ChannelLayout::Get(output).GetChannelCount());
            if (rendered != nullptr)
                AudioBuffer::Copy(*rendered, 0, copy, 0, input.GetFrameCount());

            return copy;
        }

        static bool Near(const AudioBuffer& a, const AudioBuffer& b, AmReal32 tolerance)
        {
            if (a.GetChannelCount() != b.GetChannelCount() || a.GetFrameCount() != b.GetFrameCount())
                return false;

            for (AmSize c = 0; c < a.GetChannelCount(); ++c)
                for (AmUInt64 i = 0; i < a.GetFrameCount(); ++i)
                    if (!(std::abs(a[c][i] - b[c][i]) <= tolerance))
                        return false;

            return true;
        }

        ListenerInternalState _listenerState;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
