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

#ifndef _AM_TESTS_MIXER_STREAMING_VOICE_SWAP_TEST_UTILS_H
#define _AM_TESTS_MIXER_STREAMING_VOICE_SWAP_TEST_UTILS_H

#include <memory>
#include <vector>

#include <Mixer/Streaming/StreamSource.h>
#include <Mixer/Voice/Voice.h>

#include "../mixer_voice/VoiceTestUtils.h"
#include "StreamTestDecoder.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    constexpr AmUInt64 kSwapBlock = 256;

    /// A stream over a `length`-frame test source, filled inline, planned to start at `position`.
    inline std::unique_ptr<StreamSource> MakeSwapStream(AmUInt64 length, AmUInt64 position, AmUInt64 preroll)
    {
        AmUInt64 pre = 0;
        auto decoder = std::make_shared<StreamTestDecoder>(length, 1);
        return std::make_unique<StreamSource>(
            decoder, PlanStream(0, length, false, 0, position, preroll, pre), 4096, 1, 512, 48000, true);
    }

    inline VoiceSettings MakeSwapSettings(StreamSource& stream, AmUInt64 length)
    {
        VoiceSettings settings;
        settings.source.stream = &stream;
        settings.source.length = length;
        settings.source.channels = 1;
        settings.source.sampleRate = 48000;
        settings.outputRate = 48000;
        settings.maxBlockFrames = kSwapBlock;
        settings.resamplerName = "linear";
        settings.layer = 1;
        settings.id = 1;
        return settings;
    }

    inline VoiceCommand MakeStreamSeek(AmUInt64 frame, AmUInt64 position, StreamSource* stream)
    {
        VoiceCommand command = MakeCommand(eVoiceCommandKind::Seek, frame, 0.0, position);
        command.stream = stream;
        return command;
    }

    inline std::vector<VoiceEvent> EventsOf(const VoiceRun& run, eVoiceEventKind kind)
    {
        std::vector<VoiceEvent> events;
        for (const auto& event : run.events)
            if (event.kind == kind)
                events.push_back(event);
        return events;
    }

    inline AmSize CountDropped(const VoiceRun& run, const StreamSource* stream)
    {
        AmSize count = 0;
        for (const auto& event : run.events)
            if (event.kind == eVoiceEventKind::StreamDropped && event.stream == stream)
                ++count;
        return count;
    }

    inline AmSize CountSwapped(const VoiceRun& run, const StreamSource* stream)
    {
        AmSize count = 0;
        for (const auto& event : run.events)
            if (event.kind == eVoiceEventKind::StreamSwapped && event.stream == stream)
                ++count;
        return count;
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests

#endif // _AM_TESTS_MIXER_STREAMING_VOICE_SWAP_TEST_UTILS_H
