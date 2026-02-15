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

#include <Core/Codecs/AMS/Codec.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, core_codec_ams, can_seek_in_decoded_stream)
    {
    public:
        void Run() override
        {
            AMSCodec codec;

            constexpr AmSize numSamples = 4096;
            constexpr AmSize numChannels = 1;
            constexpr AmSize sampleRate = 44100;

            AudioBuffer inputBuffer(numSamples, numChannels);
            GenerateSineWave(inputBuffer, sampleRate);

            constexpr AmSize estimatedEncodedSize = numSamples * numChannels * sizeof(AmInt16) * 2;
            auto encodedFile = std::make_shared<MemoryFile>();
            encodedFile->Open(estimatedEncodedSize);

            auto encoder = codec.CreateEncoder();
            SoundFormat format;
            format.SetAll(sampleRate, numChannels, 4, numSamples, numChannels * sizeof(AmInt16), eAudioSampleFormat_Int16);
            encoder->SetFormat(format);

            encoder->Open(encodedFile);
            encoder->Write(&inputBuffer, 0, numSamples);
            encoder->Close();

            encodedFile->Seek(0, eFileSeekOrigin_Start);

            auto decoder = codec.CreateDecoder();
            decoder->Open(encodedFile);

            AM_EXPECT(decoder->Seek(0));
            AM_EXPECT(decoder->Seek(500));
            AM_EXPECT(decoder->Seek(1000));

            decoder->Close();
        }
    };

    AM_REGISTER_TEST(core_codec_ams, can_seek_in_decoded_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
