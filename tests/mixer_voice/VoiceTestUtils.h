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

#include <algorithm>
#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/SourceReader.h>
#include <Mixer/Voice/Voice.h>

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

    struct VoiceRun
    {
        std::vector<AmReal32> source; ///< Mono render before the envelope.
        std::vector<AmReal32> gains;  ///< Envelope gain per frame.
        std::vector<VoiceEvent> events;
    };

    inline VoiceSettings MakeVoiceSettings(const AudioBuffer& buffer, AmUInt32 sourceRate, AmUInt64 block, bool loop = false)
    {
        VoiceSettings settings;
        settings.source = MemorySource(buffer, sourceRate);
        settings.loop = loop;
        settings.outputRate = 48000;
        settings.maxBlockFrames = block;
        settings.layer = 1;
        settings.id = 1;
        return settings;
    }

    /// Renders `total` frames in blocks of `block`; `before(clock)` runs before each block (enqueue commands there).
    template<typename Before>
    VoiceRun RunVoice(Voice& voice, AmUInt64 total, AmUInt64 block, Before&& before, AmUInt64 clockStart = 0)
    {
        VoiceRun run;
        auto queue = std::make_unique<VoiceEventQueue>();
        AudioBuffer mono(block, 1);

        for (AmUInt64 done = 0; done < total; done += block)
        {
            const AmUInt64 n = std::min(block, total - done);
            before(clockStart + done);

            voice.BeginBlock(clockStart + done, n);
            mono.Clear();
            voice.RenderPrimary(mono);

            AudioBuffer ones(n, 1);
            for (AmUInt64 i = 0; i < n; ++i)
                ones[0][i] = 1.0f;
            voice.ApplyGain(ones);

            voice.EndBlock();
            voice.GetMailbox().Publish(*queue);

            run.source.insert(run.source.end(), mono[0].begin(), mono[0].begin() + n);
            run.gains.insert(run.gains.end(), ones[0].begin(), ones[0].begin() + n);

            VoiceEvent event;
            while (queue->TryDequeue(event))
                run.events.push_back(event);
        }

        return run;
    }

    inline VoiceRun RunVoice(Voice& voice, AmUInt64 total, AmUInt64 block)
    {
        return RunVoice(voice, total, block, [](AmUInt64) {});
    }

    inline const VoiceEvent* FindEvent(const VoiceRun& run, eVoiceEventKind kind)
    {
        for (const auto& event : run.events)
            if (event.kind == kind)
                return &event;
        return nullptr;
    }

    inline VoiceCommand MakeCommand(eVoiceCommandKind kind, AmUInt64 frame, AmTime duration = 0.0, AmUInt64 position = 0)
    {
        return VoiceCommand{ 1, 1, kind, frame, duration, position };
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
