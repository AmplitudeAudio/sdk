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

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Schedulers/SequenceScheduler.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    SequenceScheduler::SequenceScheduler()
        : SequenceScheduler(nullptr)
    {}

    SequenceScheduler::SequenceScheduler(const SequenceSoundSchedulerConfig* config)
        : _config(config)
        , _lastIndex(0)
        , _stepMode(MODE_INCREMENT)
        , _definition(nullptr)
    {}

    bool SequenceScheduler::Valid() const
    {
        return true;
    }

    void SequenceScheduler::Init(const CollectionDefinition* definition)
    {
        // noop
    }

    Sound* SequenceScheduler::Select(std::vector<Sound>& sounds, const std::vector<const Sound*>& toSkip)
    {
        if (_lastIndex == sounds.size() || _lastIndex == 0)
        {
            SequenceSoundSchedulerEndBehavior onEnd =
                _config != nullptr ? _config->end_behavior() : SequenceSoundSchedulerEndBehavior_Restart;

            switch (onEnd)
            {
            default:
            case SequenceSoundSchedulerEndBehavior_Restart:
                _lastIndex = 0;
                _stepMode = MODE_INCREMENT;
                break;
            case SequenceSoundSchedulerEndBehavior_Reverse:
                if (_lastIndex == sounds.size())
                {
                    _lastIndex = sounds.size() - 2; // Do not play the last sound twice
                    _stepMode = MODE_DECREMENT;
                }
                else
                {
                    _lastIndex = 0;
                    _stepMode = MODE_INCREMENT;
                }
                break;
            }
        }

        Sound* sound = &sounds[_lastIndex];
        _lastIndex = AM_Clamp(0, _stepMode == MODE_INCREMENT ? _lastIndex + 1 : _lastIndex - 1, sounds.size());

        return sound;
    }
} // namespace SparkyStudios::Audio::Amplitude