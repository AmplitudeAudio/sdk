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

#include <cstdlib>

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include <Sound/Schedulers/RandomScheduler.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    RandomScheduler::RandomScheduler()
        : RandomScheduler(nullptr)
    {}

    RandomScheduler::RandomScheduler(const RandomSoundSchedulerConfig* config)
        : _config(config)
        , _probabilitiesSum(0.0f)
        , _definition(nullptr)
        , _avoidRepeatStack()
    {}

    bool RandomScheduler::Valid() const
    {
        return _definition != nullptr;
    }

    void RandomScheduler::Init(const CollectionDefinition* definition)
    {
        Engine* engine = Engine::GetInstance();

        _definition = definition;
        _probabilitiesSum = 0.0f;

        flatbuffers::uoffset_t sample_count = definition->sounds() ? definition->sounds()->size() : 0;

        for (flatbuffers::uoffset_t i = 0; i < sample_count; ++i)
        {
            const RandomSchedulerCollectionEntry* entry = definition->sounds()->GetAs<RandomSchedulerCollectionEntry>(i);
            _probabilitiesSum += entry->weight();
            _sounds.push_back(engine->GetSoundHandle(entry->sound()));
        }
    }

    Sound* RandomScheduler::Select(const std::vector<AmSoundID>& toSkip)
    {
    Pick:
        float selection = std::rand() / static_cast<float>(RAND_MAX) * _probabilitiesSum;
        for (flatbuffers::uoffset_t i = 0; i < _sounds.size(); ++i)
        {
            const RandomSchedulerCollectionEntry* entry = _definition->sounds()->GetAs<RandomSchedulerCollectionEntry>(i);
            selection -= entry->weight();

            if (selection <= 0)
            {
                if (auto foundIt = std::find(toSkip.begin(), toSkip.end(), _sounds[i]->GetId()); foundIt != toSkip.end())
                    // Try to pick the next sound, since this one needs to be skipped
                    goto Pick;

                if (_config->avoid_repeat())
                {
                    if (auto foundIt = std::find(_avoidRepeatStack.begin(), _avoidRepeatStack.end(), _sounds[i]);
                        foundIt != _avoidRepeatStack.end())
                        // Try to pick the next sound, since this one has already been played
                        goto Pick;
                }

                Sound* sound = _sounds[i];
                if (_config->avoid_repeat())
                {
                    if (!_avoidRepeatStack.empty() && _avoidRepeatStack.size() >= _config->repeat_count())
                        _avoidRepeatStack.erase(_avoidRepeatStack.begin());

                    _avoidRepeatStack.push_back(sound);
                }

                return sound;
            }
        }

        _avoidRepeatStack.clear();

        return nullptr;
    }

    void RandomScheduler::Reset()
    {
        // noop
    }
} // namespace SparkyStudios::Audio::Amplitude
