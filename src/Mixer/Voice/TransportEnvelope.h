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

#ifndef _AM_IMPLEMENTATION_MIXER_VOICE_TRANSPORT_ENVELOPE_H
#define _AM_IMPLEMENTATION_MIXER_VOICE_TRANSPORT_ENVELOPE_H

#include <memory>

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

#include <Mixer/Voice/VoiceTypes.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Audio-rate transport gain: the de-click is a raised cosine evaluated on every frame; fader curves are evaluated every 32 frames and
     * joined by cubic Hermite interpolation.
     *
     * @c Initialize() runs on the game thread; the rest is audio-thread safe.
     */
    class TransportEnvelope
    {
    public:
        static constexpr AmUInt64 kSegmentFrames = 32;

        void Initialize(std::shared_ptr<FaderInstance> curve, AmUInt32 outputRate);

        /**
         * @brief Jumps to @p gain and cancels any fade.
         */
        void SetGain(AmReal32 gain);

        /**
         * @brief Starts a fade from the current gain. Shorter than @c kDeclickFade becomes a raised cosine of that length.
         *
         * @return The fade length in frames, at least 1.
         */
        AmUInt64 FadeTo(AmReal32 target, AmTime duration);

        [[nodiscard]] AmUInt64 GetFadeLength(AmTime duration) const;

        /**
         * @brief Writes one gain per frame into @p gains, from @p offset, advancing the fade.
         */
        void Render(AudioBufferChannel& gains, AmUInt64 offset, AmUInt64 frames);

        [[nodiscard]] AmReal32 GetGain() const
        {
            return _gain;
        }

        [[nodiscard]] bool IsFading() const
        {
            return _position < _length;
        }

    private:
        AmReal64 Evaluate(AmUInt64 position);
        AmReal64 Slope(AmUInt64 position);
        AmReal64 Interpolate(AmUInt64 position);

        std::shared_ptr<FaderInstance> _curve;
        AmUInt32 _rate = 48000;
        AmReal32 _gain = 1.0f;
        AmReal32 _from = 1.0f;
        AmReal32 _target = 1.0f;
        AmUInt64 _length = 0;
        AmUInt64 _position = 0;
        bool _declick = false;
        AmUInt64 _knotStart = std::numeric_limits<AmUInt64>::max();
        AmReal64 _knotFrom = 0.0;
        AmReal64 _knotTo = 0.0;
        AmReal64 _slopeFrom = 0.0;
        AmReal64 _slopeTo = 0.0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_VOICE_TRANSPORT_ENVELOPE_H
