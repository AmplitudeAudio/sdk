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

#include <SparkyStudios/Audio/Amplitude/Sound/Schedulers/SequenceScheduler.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool SequenceScheduler::Valid() const
    {
        return true;
    }

    void SequenceScheduler::Init(const SoundCollectionDefinition* definition)
    {
        // noop
    }

    Sound* SequenceScheduler::Select(std::vector<Sound>& sounds, const std::vector<const Sound*>& toSkip)
    {
        if (_lastIndex == sounds.size())
            _lastIndex = 0;

        Sound* sound = nullptr;
        for (; _lastIndex < sounds.size(); ++_lastIndex)
        {
            if (auto foundIt = std::find(toSkip.begin(), toSkip.end(), &sounds[_lastIndex]); foundIt != toSkip.end())
                // Try to pick the next sound, since this one needs to be skipped
                continue;

            sound = &sounds[_lastIndex];
            break;
        }
        _lastIndex++;

        return sound;
    }
} // namespace SparkyStudios::Audio::Amplitude