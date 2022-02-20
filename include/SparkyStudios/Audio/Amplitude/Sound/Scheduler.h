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

#ifndef SS_AMPLITUDE_AUDIO_SOUND_COLLECTION_SCHEDULER_H
#define SS_AMPLITUDE_AUDIO_SOUND_COLLECTION_SCHEDULER_H

#include <vector>

namespace SparkyStudios::Audio::Amplitude
{
    class Sound;

    struct CollectionDefinition;

    /**
     * @brief An abstract class for Collection schedulers
     */
    class Scheduler
    {
    public:
        virtual ~Scheduler() = default;

        /**
         * @brief Returns whether this scheduler has a valid state.
         *
         * @return true if the scheduler has a valid state.
         */
        [[nodiscard]] virtual bool Valid() const = 0;

        /**
         * @brief Initializes the scheduler for the given sound collection.
         *
         * @param definition The CollectionDefinition to use for initialization.
         */
        virtual void Init(const CollectionDefinition* definition) = 0;

        /**
         * @brief Selects a sound from the list of available sounds.
         *
         * This assumes that the scheduler was already initialized.
         *
         * @param toSkip The list of sounds to skip from selection.
         *
         * The values of this list are pointers of available sounds.
         *
         * @return The selected sound.
         */
        virtual Sound* Select(const std::vector<AmSoundID>& toSkip) = 0;

        /**
         * @brief Resets the internal state of the scheduler.
         */
        virtual void Reset() = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUND_COLLECTION_SCHEDULER_H
