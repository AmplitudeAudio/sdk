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

#include <SparkyStudios/Audio/Amplitude/Sound/Schedulers/RandomScheduler.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include "sound_collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    void RandomScheduler::Init(const SoundCollectionDefinition* definition)
    {
        _definition = definition;

        flatbuffers::uoffset_t sample_count = definition->audio_sample_set() ? definition->audio_sample_set()->size() : 0;

        for (flatbuffers::uoffset_t i = 0; i < sample_count; ++i)
        {
            const AudioSampleSetEntry* entry = definition->audio_sample_set()->Get(i);
            const char* entry_filename = entry->audio_sample()->filename()->c_str();
            _probabilitiesSum += entry->playback_probability();
        }
    }

    Sound* RandomScheduler::Select(std::vector<Sound>& sounds)
    {
        float selection = std::rand() / static_cast<float>(RAND_MAX) * _probabilitiesSum;
        for (size_t i = 0; i < sounds.size(); ++i)
        {
            const AudioSampleSetEntry* entry = _definition->audio_sample_set()->Get(static_cast<flatbuffers::uoffset_t>(i));

            selection -= entry->playback_probability();

            if (selection <= 0)
            {
                return &sounds[i];
            }
        }

        return nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude