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

#ifndef _AM_DSP_REVERBS_DATTORO_REVERB_H
#define _AM_DSP_REVERBS_DATTORO_REVERB_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Reverb.h>

#include <memory>

namespace SparkyStudios::Audio::Amplitude
{
    namespace Internal
    {
        class DattoroReverb;
    }

    class DattoroReverb;

    /**
     * @brief Dattorro plate reverb algorithm instance.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC DattoroReverbInstance final : public ReverbInstance
    {
    public:
        /**
         * @brief Constructs a new @c DattoroReverbInstance object.
         *
         * @param[in] parent The parent @c DattoroReverb object that created this instance.
         */
        explicit DattoroReverbInstance(DattoroReverb* parent);

        /**
         * @brief Destroys the @c DattoroReverbInstance object.
         */
        ~DattoroReverbInstance() override;

        /**
         * @copydoc ReverbInstance::Initialize
         */
        void Initialize(AmUInt32 sampleRate) override;

        /**
         * @copydoc ReverbInstance::Reset
         */
        void Reset() override;

        /**
         * @copydoc ReverbInstance::SetRoomParameters
         */
        void SetRoomParameters(AmReal32 roomSize, AmReal32 absorption) override;

        /**
         * @copydoc ReverbInstance::Process
         */
        void Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate) override;

    private:
        std::unique_ptr<Internal::DattoroReverb> _reverb;
        AmUInt32 _sampleRate;
    };

    /**
     * @brief Dattorro plate reverb plugin.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC DattoroReverb final : public Reverb
    {
    public:
        /**
         * @brief Constructs a new @c DattoroReverb object.
         */
        DattoroReverb();

        /**
         * @brief Destroys the @c DattoroReverb object.
         */
        ~DattoroReverb() override = default;

        /**
         * @copydoc Reverb::CreateInstance
         */
        [[nodiscard]] std::shared_ptr<ReverbInstance> CreateInstance() override;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_REVERBS_DATTORO_REVERB_H
