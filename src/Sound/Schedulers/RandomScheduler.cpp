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

#include <algorithm>
#include <cstdlib>

#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include <Core/Engine.h>
#include <Sound/Schedulers/RandomScheduler.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    RandomScheduler::RandomScheduler()
        : RandomScheduler(nullptr)
    {}

    RandomScheduler::RandomScheduler(const RandomSoundSchedulerConfig* config)
        : _probabilitiesSum(0.0f)
        , _definition(nullptr)
        , _config(config)
        , _avoidRepeatStack()
    {}

    bool RandomScheduler::Valid() const
    {
        return _definition != nullptr;
    }

    void RandomScheduler::Init(const CollectionDefinition* definition)
    {
        _definition = definition;
        _probabilitiesSum = 0.0f;

        const flatbuffers::uoffset_t sample_count = definition->sounds() ? definition->sounds()->size() : 0;

        for (flatbuffers::uoffset_t i = 0; i < sample_count; ++i)
        {
            const auto* entry = definition->sounds()->GetAs<RandomSchedulerCollectionEntry>(i);
            _probabilitiesSum += entry->weight();
            _sounds.push_back(amEngine->GetSoundHandle(entry->sound()));
        }
    }

    Sound* RandomScheduler::Select(const std::unordered_set<AmSoundID>& toSkip)
    {
        if (_definition == nullptr || _sounds.empty())
            return nullptr;

        const bool avoidRepeat = _config != nullptr && _config->avoid_repeat();

        const auto weightOf = [this](const size_t index) -> AmReal32
        {
            return _definition->sounds()->GetAs<RandomSchedulerCollectionEntry>(static_cast<flatbuffers::uoffset_t>(index))->weight();
        };

        // `excluded` is a sound that must not be picked (the one just played, when avoid-repeat is relaxed).
        const auto isEligible = [&](const size_t index, const bool honorAvoidRepeat, const Sound* excluded) -> bool
        {
            const Sound* sound = _sounds[index];
            if (sound == nullptr || sound == excluded || toSkip.contains(sound->GetId()))
                return false;

            return !(honorAvoidRepeat && std::ranges::find(_avoidRepeatStack, sound) != _avoidRepeatStack.end());
        };

        const auto eligibleWeight = [&](const bool honorAvoidRepeat, const Sound* excluded) -> AmReal32
        {
            AmReal32 total = 0.0f;
            for (size_t i = 0; i < _sounds.size(); ++i)
            {
                if (isEligible(i, honorAvoidRepeat, excluded))
                    total += weightOf(i);
            }
            return total;
        };

        const auto eligibleCount = [&](const bool honorAvoidRepeat) -> size_t
        {
            size_t count = 0;
            for (size_t i = 0; i < _sounds.size(); ++i)
                count += isEligible(i, honorAvoidRepeat, nullptr) ? 1 : 0;
            return count;
        };

        bool honorAvoidRepeat = avoidRepeat;
        const Sound* excluded = nullptr;
        AmReal32 total = eligibleWeight(honorAvoidRepeat, excluded);

        if (total <= 0.0f && honorAvoidRepeat)
        {
            // Every remaining sound was recently played: relax the avoid-repeat rule instead of failing, but still
            // avoid replaying the sound just played unless it is the only candidate left.
            honorAvoidRepeat = false;
            if (!_avoidRepeatStack.empty() && eligibleCount(false) > 1)
                excluded = _avoidRepeatStack.back();

            total = eligibleWeight(honorAvoidRepeat, excluded);
        }

        if (total <= 0.0f)
            return nullptr;

        AmReal32 selection = static_cast<AmReal32>(std::rand()) / static_cast<AmReal32>(RAND_MAX) * total;
        Sound* picked = nullptr;

        for (size_t i = 0; i < _sounds.size(); ++i)
        {
            if (!isEligible(i, honorAvoidRepeat, excluded))
                continue;

            // Remember the last eligible sound: it is the fallback if rounding walks past the end.
            picked = _sounds[i];

            selection -= weightOf(i);
            if (selection <= 0.0f)
                break;
        }

        if (avoidRepeat && picked != nullptr)
        {
            // After a relaxed pick the history restarts, so the stack never holds duplicates.
            if (!honorAvoidRepeat)
                _avoidRepeatStack.clear();

            if (!_avoidRepeatStack.empty() && _avoidRepeatStack.size() >= _config->repeat_count())
                _avoidRepeatStack.erase(_avoidRepeatStack.begin());

            _avoidRepeatStack.push_back(picked);
        }

        return picked;
    }

    void RandomScheduler::Reset()
    {
        // noop
    }
} // namespace SparkyStudios::Audio::Amplitude
