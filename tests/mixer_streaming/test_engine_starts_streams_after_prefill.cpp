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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Sound/Sound.h>

#include "RecordingCodec.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmInt32 kDecoderDelayMs = 300;
        constexpr AmUInt64 kReferenceFrames = 4096;

        /// The peak of the first frames of a file, decoded with the plain WAV codec.
        AmReal32 ReferencePeak(const AmOsString& path)
        {
            const auto file = amEngine->GetFileSystem()->OpenFile(path);
            auto decoder = Codec::Find("wav")->CreateDecoder();
            if (file == nullptr || decoder == nullptr || !decoder->Open(file))
                return 0.0f;

            const AmUInt16 channels = decoder->GetFormat().GetNumChannels();
            AudioBuffer buffer(kReferenceFrames, channels);
            const AmUInt64 frames = decoder->Stream(&buffer, 0, 0, kReferenceFrames);
            decoder->Close();

            AmReal32 peak = 0.0f;
            for (AmUInt16 c = 0; c < channels; ++c)
                for (AmUInt64 i = 0; i < frames; ++i)
                    peak = std::max(peak, std::abs(buffer.GetChannel(c)[i]));

            return peak;
        }
    } // namespace

    // A streamed voice is not mixed before its ring holds the prefill: with a decoder that takes 300 ms per call, the
    // channel does not begin and nothing is rendered until the first decode returned.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_starts_streams_after_prefill)
    {
    public:
        void Run() override
        {
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("test_sound_02"));
            AM_EXPECT(sound != nullptr);
            const AmReal32 referencePeak = ReferencePeak(sound->GetPath());

            RecordingState::Get().SetDelay(kDecoderDelayMs);

            EventCount began;
            Channel channel = amEngine->Play(sound);
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &began);

            bool sawGate = false;
            bool gateViolated = false;
            bool positionMoved = false;

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    // Read in this order: a begin or a mix seen first implies a decode that returned before.
                    const AmInt32 begins = began.count.load();
                    const AmUInt32 mixes = MixedBlocks().load();
                    const AmUInt64 decoded = RecordingState::Get().GetCompletedStreamCalls();

                    if (begins == 0 && mixes == 0 && decoded == 0)
                    {
                        sawGate = true;
                        if (channel.GetPlaybackPosition() != 0.0)
                            gateViolated = true;
                    }

                    if ((begins > 0 || mixes > 0) && decoded == 0)
                        gateViolated = true;

                    positionMoved = channel.GetPlaybackPosition() > 0.0;
                    return begins > 0 && positionMoved;
                }));

            AM_EXPECT(sawGate);
            AM_EXPECT(!gateViolated);
            AM_EXPECT(positionMoved);
            AM_EXPECT(began.count.load() >= 1);

            // The first decode started at the start of the file.
            if (referencePeak > 0.0f)
            {
                AM_EXPECT(FramesUntil(
                    []()
                    {
                        return AudibleBlocks().load() > 0;
                    }));
            }

            RecordingState::Get().SetDelay(0);
            channel.Stop(0);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_starts_streams_after_prefill);
} // namespace SparkyStudios::Audio::Amplitude::Tests
