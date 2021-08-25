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

#ifndef SPARK_AUDIO_CONSECUTIVE_SCHEDULER_H
#define SPARK_AUDIO_CONSECUTIVE_SCHEDULER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Scheduler.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SequenceSoundSchedulerConfig;

    class SequenceScheduler : public Scheduler
    {
    public:
        SequenceScheduler();
        explicit SequenceScheduler(const SequenceSoundSchedulerConfig* config);

        [[nodiscard]] bool Valid() const override;
        void Init(const SoundCollectionDefinition* definition) override;
        Sound* Select(std::vector<Sound>& sounds, const std::vector<const Sound*>& toSkip) override;

    private:
        enum STEP_MODE: AmUInt8 {
            MODE_INCREMENT,
            MODE_DECREMENT
        };

        AmUInt32 _lastIndex;
        STEP_MODE _stepMode;
        const SoundCollectionDefinition* _definition;
        const SequenceSoundSchedulerConfig* _config;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_CONSECUTIVE_SCHEDULER_H
