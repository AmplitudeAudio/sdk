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

#ifndef SPARK_AUDIO_SOUND_COLLECTION_SCHEDULER_H
#define SPARK_AUDIO_SOUND_COLLECTION_SCHEDULER_H

#include <vector>

namespace SparkyStudios::Audio::Amplitude
{
    class Sound;

    struct SoundCollectionDefinition;

    /**
     * @brief An abstract class for SoundCollection schedulers
     */
    class SoundCollectionScheduler
    {
    public:
        /**
         * @brief Initializes the scheduler for the given sound collection.
         *
         * @param definition The SoundCollectionDefinition to use for initialization.
         */
        virtual void Init(const SoundCollectionDefinition* definition) = 0;

        /**
         * @brief Selects a sound from the list of available sounds.
         *
         * This assumes that the scheduler was already initialized.
         *
         * @param sounds The list of available sounds.
         * @return The selected sound.
         */
        virtual Sound* Select(std::vector<Sound>& sounds) = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_COLLECTION_SCHEDULER_H
