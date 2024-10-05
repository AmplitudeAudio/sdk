// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_BINAURALIZER_H
#define _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_BINAURALIZER_H

#include <SparkyStudios/Audio/Amplitude/DSP/Convolver.h>

#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/BFormat.h>
#include <HRTF/HRIRSphere.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmbisonicBinauralizer final : public AmbisonicComponent
    {
    public:
        AmbisonicBinauralizer();

        bool Configure(AmUInt32 order, bool is3D, const HRIRSphere* hrirSphere);

        void Reset() override;

        void Refresh() override;

        /**
         * @brief Decodes the B-Format input audio samples and outputs the speaker feed.
         *
         * @param input The input audio samples in B-format.
         * @param samples The number of audio samples to process.
         * @param output The output speaker feed as audio samples.
         */
        void Process(BFormat* input, AmUInt32 samples, AudioBuffer& output);

    private:
        void SetUpSpeakers();

        AmbisonicDecoder _decoder;

        const HRIRSphere* _hrir;
        AudioBuffer _accumulatedHRIR[2];

        Convolver _convL[16];
        Convolver _convR[16];
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_BINAURALIZER_H