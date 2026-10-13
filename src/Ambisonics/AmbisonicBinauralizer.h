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

#include <memory>

#include <Ambisonics/AmbisonicComponent.h>
#include <Ambisonics/AmbisonicShelfFilter.h>
#include <Ambisonics/BFormat.h>
#include <Ambisonics/BinauralDecodingFilters.h>
#include <DSP/SummingStereoConvolver.h>
#include <HRTF/HRIRSphere.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmbisonicBinauralizer final : public AmbisonicComponent
    {
    public:
        AmbisonicBinauralizer() = default;

        /**
         * @brief Configures the binauralizer, building its own decoding filters from the HRIR sphere.
         */
        bool Configure(AmUInt32 order, bool is3D, AmUInt32 maxBlockSize, AmUInt32 sampleRate, const HRIRSphere* hrirSphere);

        /**
         * @brief Configures the binauralizer on shared decoding filters. Only the per-voice state is allocated.
         *
         * @return false when the filters are null or do not match the order and 2D/3D setting.
         */
        bool Configure(
            AmUInt32 order, bool is3D, AmUInt32 maxBlockSize, AmUInt32 sampleRate, std::shared_ptr<const BinauralDecodingFilters> filters);

        void Reset() override;

        void Refresh() override;

        /**
         * @brief Decodes the B-Format input audio samples and outputs the speaker feed.
         *
         * @param input The input audio samples in B-format.
         * @param samples The number of audio samples to process.
         * @param output The output speaker feed as audio samples, added to.
         */
        void Process(BFormat* input, AmUInt32 samples, AudioBuffer& output);

        /**
         * @brief Gets the decoding filters in use, or nullptr before a successful Configure.
         */
        [[nodiscard]] const std::shared_ptr<const BinauralDecodingFilters>& GetFilters() const;

    private:
        AmbisonicShelfFilter _shelfFilter;
        std::shared_ptr<const BinauralDecodingFilters> _filters;
        SummingStereoConvolver _convolver;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_BINAURALIZER_H