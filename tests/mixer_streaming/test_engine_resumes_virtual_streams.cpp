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

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Sound/Sound.h>

#include "RecordingCodec.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        /// Boots on the configuration with a single real channel, so a louder sound demotes the one playing.
        class SingleChannelStreamingTestCase : public StreamingEngineTestCase
        {
        protected:
            [[nodiscard]] AmOsString GetConfigPath() const override
            {
                return AM_OS_STRING("tests.single_channel.config.amconfig");
            }
        };
    } // namespace

    // A streamed sound demoted to a virtual channel gives its streams back, and when it is promoted again it resumes at
    // the cursor it would have reached, on streams planned there.
    AM_TEST_CASE(SingleChannelStreamingTestCase, mixer_streaming, engine_resumes_virtual_streams)
    {
    public:
        void Run() override
        {
            auto& mixer = amEngine->GetState()->mixer;
            const AmReal64 outputRate = mixer.GetDeviceDescription().mRequestedOutputSampleRate;

            auto* forest = static_cast<SoundImpl*>(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(forest != nullptr);
            const SoundFormat& format = forest->GetFormat();
            const AmTime lengthMs = static_cast<AmTime>(format.GetFramesCount()) * kAmSecond / static_cast<AmTime>(format.GetSampleRate());
            AM_EXPECT(lengthMs > 10000.0);

            EventCount began;
            Channel low = amEngine->Play(forest, AmVector3{ 0.0f, 0.0f, 0.0f }, 0.5f);
            AM_EXPECT(low.Valid());
            low.On(eChannelEvent_Begin, &CountEvent, &began);

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return began.count.load() > 0 && low.GetPlaybackPosition() > 1000.0;
                }));
            AM_EXPECT(low.GetState()->IsReal());

            // A louder sound takes the only real channel: the forest goes virtual and gives its streams back. The last
            // position seen while real anchors the virtual cursor (the demotion happens within a frame of it).
            AmTime anchorPosition = low.GetPlaybackPosition();
            AmUInt64 anchorClock = mixer.GetAudioClock();

            Channel high = amEngine->Play(amEngine->GetSoundHandle("test_sound_06"), AmVector3{ 0.0f, 0.0f, 0.0f }, 1.0f);
            AM_EXPECT(high.Valid());

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    if (low.GetState()->IsReal())
                    {
                        anchorPosition = low.GetPlaybackPosition();
                        anchorClock = mixer.GetAudioClock();
                    }

                    return !low.GetState()->IsReal();
                }));
            AM_EXPECT(low.Playing());
            AM_EXPECT(anchorPosition > 1000.0);

            // The old streams are retired: every decoder opened so far is closed.
            const auto demotedDecoders = RecordingState::Get().GetDecoders();
            AM_EXPECT(!demotedDecoders.empty());
            AM_EXPECT(FramesUntil(
                []()
                {
                    return RecordingState::Get().AllClosed();
                }));

            // The virtual cursor keeps running with the audio clock.
            for (AmUInt32 i = 0; i < 60; ++i)
                Frame();

            AM_EXPECT(!low.GetState()->IsReal());

            // Freeing the real channel promotes it: a new stream is planned at the cursor it would have reached.
            high.Stop(0.0);

            AmTime promotedPosition = 0.0;
            AmUInt64 promotedClock = 0;
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    if (!low.GetState()->IsReal())
                        return false;

                    promotedPosition = low.GetPlaybackPosition();
                    promotedClock = mixer.GetAudioClock();
                    return promotedPosition > anchorPosition;
                }));

            // At its first rendered position it sits where the virtual cursor stands (restarting would read near zero;
            // the margin is the polling skew of this test, a few frames).
            const AmTime expected = anchorPosition + static_cast<AmTime>(promotedClock - anchorClock) * kAmSecond / outputRate;
            AM_EXPECT(std::abs(promotedPosition - expected) < 250.0);
            AM_EXPECT(promotedPosition - anchorPosition > 1000.0);

            // It plays on from there, with new streams, without an underrun.
            const AmUInt64 decodersBefore = RecordingState::Get().GetDecoders().size();
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return low.GetPlaybackPosition() > promotedPosition + 500.0;
                }));
            AM_EXPECT_EQ(0ULL, GetUnderruns());
            AM_EXPECT(RecordingState::Get().GetDecoders().size() >= decodersBefore);

            low.Stop(0.0);
            AM_EXPECT(FramesUntil(
                []()
                {
                    return RecordingState::Get().AllClosed();
                }));

            for (AmUInt32 i = 0; i < 10; ++i)
                Frame();

            AM_EXPECT(RecordingState::Get().AllClosedOnce());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_resumes_virtual_streams);
} // namespace SparkyStudios::Audio::Amplitude::Tests
