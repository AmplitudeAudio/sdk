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

#ifndef _AM_SOUND_SOUND_H
#define _AM_SOUND_SOUND_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/IO/Resource.h>

#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Amplitude Sound Asset.
     *
     * A `Sound` is the most basic sound object in Amplitude. It can be used to directly play an audio file,
     * or can be contained in a `SwitchContainer` or a `Collection` for a fine-grained control.
     *
     * Effects can be attached to a `Sound`, which will be applied to all instances of the sound in the Amplimix pipeline.
     *
     * @see [SwitchContainer](../SwitchContainer/index.md), [Collection](../Collection/index.md),
     * [SoundObject](../../engine/SoundObject/index.md)
     *
     * @ingroup assets
     */
    class AM_API_PUBLIC Sound
        : public SoundObject
        , public Resource
        , public Asset<AmSoundID>
    {
    public:
        /**
         * @brief Checks streaming is enabled for this Sound.
         *
         * @return `true` if streaming is enabled, `false` otherwise.
         */
        [[nodiscard]] virtual bool IsStream() const = 0;

        /**
         * @brief Checks if looping is enabled for this Sound.
         *
         * @return `true` if looping is enabled, `false` otherwise.
         */
        [[nodiscard]] virtual bool IsLoop() const = 0;

        /**
         * @brief Gets the near field effect gain of the sound object.
         *
         * @return The sound object near field effect gain.
         */
        [[nodiscard]] virtual const RtpcValue& GetNearFieldGain() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_SOUND_H
