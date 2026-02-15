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

#include <Core/Codecs/WAV/Codec.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, core_codec_wav, can_encode_and_decode_stereo_audio)
    {
    public:
        void Run() override
        {
            WAVCodec codec;

            constexpr AmSize numSamples = 2041;
            constexpr AmSize numChannels = 2;
            constexpr AmSize sampleRate = 44100;

            AudioBuffer inputBuffer(numSamples, numChannels);
            GenerateSineWave(inputBuffer, sampleRate);

            constexpr AmSize estimatedEncodedSize = numSamples * numChannels * sizeof(AmReal32) * 2;
            auto encodedFile = std::make_shared<MemoryFile>();
            encodedFile->Open(estimatedEncodedSize);

            auto encoder = codec.CreateEncoder();
            SoundFormat format;
            format.SetAll(sampleRate, numChannels, 16, numSamples, numChannels * sizeof(AmAudioSample), eAudioSampleFormat_Float32);
            encoder->SetFormat(format);

            AM_EXPECT(encoder->Open(encodedFile));
            AmUInt64 written = encoder->Write(&inputBuffer, 0, numSamples);
            AM_EXPECT_EQ(written, numSamples);
            encoder->Close();

            encodedFile->Seek(0, eFileSeekOrigin_Start);

            auto decoder = codec.CreateDecoder();
            AM_EXPECT(decoder->Open(encodedFile));

            const SoundFormat& decodedFormat = decoder->GetFormat();
            AM_EXPECT_EQ(sampleRate, decodedFormat.GetSampleRate());
            AM_EXPECT_EQ(numChannels, decodedFormat.GetNumChannels());

            AudioBuffer outputBuffer(numSamples, numChannels);
            AmUInt64 loaded = decoder->Load(&outputBuffer);
            AM_EXPECT_EQ(loaded, numSamples);

            decoder->Close();
        }
    };

    AM_REGISTER_TEST(core_codec_wav, can_encode_and_decode_stereo_audio);
} // namespace SparkyStudios::Audio::Amplitude::Tests
