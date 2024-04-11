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

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include <Sound/Schedulers/SequenceScheduler.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    SequenceScheduler::SequenceScheduler()
        : SequenceScheduler(nullptr)
    {}

    SequenceScheduler::SequenceScheduler(const SequenceSoundSchedulerConfig* config)
        : _nextIndex(0)
        , _step(1)
        , _definition(nullptr)
        , _config(config)
    {}

    bool SequenceScheduler::Valid() const
    {
        return true;
    }

    void SequenceScheduler::Init(const CollectionDefinition* definition)
    {
        for (size_t i = 0; i < definition->sounds()->size(); ++i)
        {
            const auto* entry = definition->sounds()->GetAs<SequenceSchedulerCollectionEntry>(i);
            _sounds.push_back(amEngine->GetSoundHandle(entry->sound()));
        }
    }

    Sound* SequenceScheduler::Select(const std::vector<AmSoundID>& toSkip)
    {
        const auto count = static_cast<AmInt32>(_sounds.size());

        if (_nextIndex == count || _nextIndex == -1)
        {
            switch (_config != nullptr ? _config->end_behavior() : SequenceSoundSchedulerEndBehavior_Restart)
            {
            default:
            case SequenceSoundSchedulerEndBehavior_Restart:
                _nextIndex = 0;
                _step = 1;
                break;
            case SequenceSoundSchedulerEndBehavior_Reverse:
                if (_nextIndex == count)
                {
                    _nextIndex = count - 1;
                    _step = -1;
                }
                else
                {
                    _nextIndex = 0;
                    _step = 1;
                }
                break;
            }
        }

        Sound* sound = _sounds[_nextIndex];
        _nextIndex = AM_CLAMP(_nextIndex + _step, -1, count);

        return sound;
    }

    void SequenceScheduler::Reset()
    {
        _nextIndex = 0;
        _step = 1;
    }
} // namespace SparkyStudios::Audio::Amplitude