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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_SPEAKER_H
#define _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_SPEAKER_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>

#include <Ambisonics/AmbisonicEntity.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BFormat;

    /**
     * @brief Ambisonic speaker class representing a physical speaker.
     *
     * It is used by the ambisonic decoder to render the B-Format encoded audio signals
     * using a set of virtual speakers at given positions.
     */
    class AmbisonicSpeaker : public AmbisonicEntity
    {
    public:
        AmbisonicSpeaker();
        ~AmbisonicSpeaker() override;

        /**
         * @copydoc AmbisonicEntity::Configure
         */
        bool Configure(AmUInt32 order, bool is3D) override;

        /**
         * @copydoc AmbisonicEntity::Refresh
         */
        void Refresh() override;

        /**
         * @brief Process the input B-Format audio signals and render them into the virtual speaker.
         *
         * @param input The input B-Format audio signals.
         * @param frameCount The number of samples to process.
         * @param output The output buffer to store the rendered virtual speaker signals.
         */
        void Process(BFormat* input, AmUInt32 frameCount, AudioBufferChannel& output);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif //_AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_SPEAKER_H
