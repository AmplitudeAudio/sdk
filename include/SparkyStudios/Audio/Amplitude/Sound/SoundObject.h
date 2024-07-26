// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#ifndef _AM_SOUND_SOUND_OBJECT_H
#define _AM_SOUND_SOUND_OBJECT_H

#include <SparkyStudios/Audio/Amplitude/Core/Playback/Bus.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The SoundObject class is the base class for all sound objects.
     */
    class AM_API_PUBLIC SoundObject
    {
    public:
        virtual ~SoundObject() = default;

        /**
         * @brief Gets the actual gain of the sound object.
         *
         * @return The sound object gain.
         */
        [[nodiscard]] virtual const RtpcValue& GetGain() const = 0;

        /**
         * @brief Gets the actual pitch of the sound object.
         *
         * @return The sound object pitch.
         */
        [[nodiscard]] virtual const RtpcValue& GetPitch() const = 0;

        /**
         * @brief Gets the actual priority of the sound object.
         *
         * @return The sound object priority.
         */
        [[nodiscard]] virtual const RtpcValue& GetPriority() const = 0;

        /**
         * @brief Get the Effect object associated with this sound object.
         *
         * @return The Effect object.
         */
        [[nodiscard]] virtual const Effect* GetEffect() const = 0;

        /**
         * @brief Get the Attenuation object associated with this sound object.
         *
         * @return The Attenuation object.
         */
        [[nodiscard]] virtual const Attenuation* GetAttenuation() const = 0;

        /**
         * @brief Return the bus this sound object will play on.
         *
         * @return The bus this sound object will play on.
         */
        [[nodiscard]] virtual Bus GetBus() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_SOUND_OBJECT_H
