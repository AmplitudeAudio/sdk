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

#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/SourceReader.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /// A source of `frames` frames where frame i holds i * step on every channel.
    inline AudioBuffer MakeRamp(AmUInt64 frames, AmReal32 step, AmUInt16 channels = 1)
    {
        AudioBuffer buffer(frames, channels);
        for (AmUInt16 c = 0; c < channels; ++c)
            for (AmUInt64 i = 0; i < frames; ++i)
                buffer[c][i] = static_cast<AmReal32>(i) * step;
        return buffer;
    }

    inline VoiceSource MemorySource(const AudioBuffer& buffer, AmUInt32 sampleRate)
    {
        VoiceSource source;
        source.buffer = &buffer;
        source.length = buffer.GetFrameCount();
        source.channels = static_cast<AmUInt16>(buffer.GetChannelCount());
        source.sampleRate = sampleRate;
        return source;
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
