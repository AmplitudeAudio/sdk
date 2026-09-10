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
#include <SparkyStudios/Audio/Amplitude/DSP/Reverb.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, dsp_reverbs, default_reverb_plugins)
    {
    public:
        void Run() override
        {
            auto freeverb = Reverb::Find("Freeverb");
            auto dattorro = Reverb::Find("Dattorro");
            auto room = Reverb::Find("RoomReverb");

            AM_EXPECT_NOT(freeverb == nullptr);
            AM_EXPECT_NOT(dattorro == nullptr);
            AM_EXPECT_NOT(room == nullptr);

            constexpr AmUInt64 frameCount = 8192;
            constexpr AmUInt32 sampleRate = 48000;

            // Test Freeverb instance: stereo and mono out
            {
                auto inst = freeverb->CreateInstance();
                AM_EXPECT_NOT(inst == nullptr);
                inst->Initialize(sampleRate);
                inst->Configure(frameCount);
                inst->SetRoomParameters(0.8f, 0.2f);

                AudioBuffer inStereo(frameCount, 2);
                AudioBuffer outStereo(frameCount, 2);
                inStereo[0][0] = 1.0f;
                inStereo[1][0] = 1.0f;

                inst->Process(inStereo, outStereo, frameCount, sampleRate);

                AmReal32 energyStereo = 0.0f;
                for (AmUInt64 f = 0; f < frameCount; ++f)
                    energyStereo += outStereo[0][f] * outStereo[0][f] + outStereo[1][f] * outStereo[1][f];
                AM_EXPECT(energyStereo > 0.0f);

                // Test mono output (exercises scratch buffer path)
                AudioBuffer inMono(frameCount, 1);
                AudioBuffer outMono(frameCount, 1);
                inMono[0][0] = 1.0f;

                inst->Process(inMono, outMono, frameCount, sampleRate);

                AmReal32 energyMono = 0.0f;
                for (AmUInt64 f = 0; f < frameCount; ++f)
                    energyMono += outMono[0][f] * outMono[0][f];
                AM_EXPECT(energyMono > 0.0f);
            }

            // Test Dattorro instance
            {
                auto inst = dattorro->CreateInstance();
                AM_EXPECT_NOT(inst == nullptr);
                inst->Initialize(sampleRate);
                inst->Configure(frameCount);
                inst->Reset();

                AudioBuffer in(frameCount, 2);
                AudioBuffer out(frameCount, 2);
                in[0][0] = 1.0f;
                in[1][0] = 1.0f;

                inst->Process(in, out, frameCount, sampleRate);

                AmReal32 energy = 0.0f;
                for (AmUInt64 f = 0; f < frameCount; ++f)
                    energy += out[0][f] * out[0][f] + out[1][f] * out[1][f];
                AM_EXPECT(energy > 0.0f);
            }

            // Test RoomReverb instance
            {
                auto inst = room->CreateInstance();
                AM_EXPECT_NOT(inst == nullptr);
                inst->Initialize(sampleRate);
                inst->Configure(frameCount);
                inst->SetRoomParameters(0.5f, 0.5f);
                inst->Reset();

                AudioBuffer in(frameCount, 2);
                AudioBuffer out(frameCount, 2);
                in[0][0] = 1.0f;
                in[1][0] = 1.0f;

                inst->Process(in, out, frameCount, sampleRate);

                AmReal32 energy = 0.0f;
                for (AmUInt64 f = 0; f < frameCount; ++f)
                    energy += out[0][f] * out[0][f] + out[1][f] * out[1][f];
                AM_EXPECT(energy > 0.0f);
            }
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, default_reverb_plugins);
} // namespace SparkyStudios::Audio::Amplitude::Tests
