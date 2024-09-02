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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_DECODER_H
#define _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_DECODER_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Math/SphericalPosition.h>

#include <Ambisonics/AmbisonicComponent.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmbisonicSpeaker;
    class BFormat;

    class AmbisonicDecoder : public AmbisonicComponent
    {
    public:
        AmbisonicDecoder();
        ~AmbisonicDecoder() override;

        /**
         * @brief Configures the ambisonic component with the given parameters. Previous configurations will be discarded.
         *
         * @param order The order of the ambisonic component.
         * @param is3D Whether the ambisonic component is 3D or not (has height).
         * @param blockSize The block size for processing.
         * @param setUp The speaker setup for the ambisonic component. If @c SpeakerSetUp::eSpeakerSetUp_Custom is used,
         * then @ref speakerCount must be provided.
         * @param speakerCount The number of speakers in the speaker setup. Only used when @ref setUp is @c
         * SpeakerSetUp::eSpeakerSetUp_Custom.
         *
         * @return @c true if the configuration is successful, @c false otherwise.
         */
        bool Configure(AmUInt32 order, bool is3D, SpeakersPreset setUp, AmUInt32 speakerCount = 0);

        /**
         * @copydoc AmbisonicComponent::Reset
         */
        void Reset() override;

        /**
         * @copydoc AmbisonicComponent::Refresh
         */
        void Refresh() override;

        /**
         * @brief Decodes the B-Format input audio samples and outputs the speaker feed.
         *
         * @param input The input audio samples in B-format.
         * @param samples The number of audio samples to process.
         * @param output The output speaker feed as audio samples.
         */
        void Process(BFormat* input, AmUInt32 samples, AudioBuffer& output);

        /**
         * @brief Gets the speaker setup for the ambisonic decoder.
         *
         * @return The speaker setup for the ambisonic decoder.
         */
        [[nodiscard]] AM_INLINE SpeakersPreset GetSpeakerSetUp() const
        {
            return _speakersPreset;
        }

        [[nodiscard]] AM_INLINE AmUInt32 GetSpeakerCount() const
        {
            return _speakerCount;
        }

        void SetSpeakerPosition(AmUInt32 speaker, const SphericalPosition& position);

        [[nodiscard]] SphericalPosition GetSpeakerPosition(AmUInt32 speaker) const;

        void SetSpeakerOrderWeight(AmUInt32 speaker, AmUInt32 order, AmReal32 weight);

        [[nodiscard]] AmReal32 GetSpeakerOrderWeight(AmUInt32 speaker, AmUInt32 order) const;

        void SetSpeakerCoefficient(AmUInt32 speaker, AmUInt32 channel, AmReal32 coefficient);

        [[nodiscard]] AmReal32 GetSpeakerCoefficient(AmUInt32 speaker, AmUInt32 channel) const;

        [[nodiscard]] AM_INLINE bool IsLoaded() const
        {
            return _isLoaded;
        }

    private:
        void SetUpSpeakers(SpeakersPreset setUp, AmUInt32 speakerCount = 1);

        /**
         * @brief Detects the speaker set-up from the initial configuration. If the decoder found a configuration
         * matching a known layout, it loads the corresponding decoder preset.
         */
        void DetectSpeakersPreset();

        /**
         * @brief Loads the decoder preset if it's a known layout.
         */
        void LoadDecoderPreset();

        SpeakersPreset _speakersPreset;

        AmUInt32 _speakerCount;
        std::vector<AmbisonicSpeaker> _speakers;

        bool _isLoaded;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_DECODER_H
