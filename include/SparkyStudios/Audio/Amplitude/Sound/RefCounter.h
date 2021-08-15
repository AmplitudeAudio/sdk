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

#ifndef SPARK_AUDIO_REF_COUNTER_H
#define SPARK_AUDIO_REF_COUNTER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Holds the number of references to an object.
     */
    class RefCounter
    {
    public:
        /**
         * @brief Constructs a new RefCounter.
         *
         * This initializes the internal counter to 0.
         */
        RefCounter()
            : _count(0)
        {}

        /**
         * @brief Updates the counter by adding one reference.
         *
         * @return The number of references.
         */
        AmInt32 Increment();

        /**
         * @brief Updates the counter by removing one reference.
         *
         * @return The number of references.
         */
        AmInt32 Decrement();

        /**
         * @brief Gets the current number of references.
         *
         * @return The current number of references.
         */
        [[nodiscard]] AmInt32 GetCount() const
        {
            return _count;
        }

    private:
        AmInt32 _count;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_REF_COUNTER_H
