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

#ifndef SPARK_AUDIO_SOUND_BANK_H
#define SPARK_AUDIO_SOUND_BANK_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Sound/RefCounter.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SoundBankDefinition;

    class Engine;

    class SoundBank
    {
    public:
        bool Initialize(const std::string& filename, Engine* audio_engine);

        void Deinitialize(Engine* audio_engine);

        RefCounter* GetRefCounter()
        {
            return &_refCounter;
        }

    private:
        RefCounter _refCounter;
        std::string _soundBankDefSource;
        const SoundBankDefinition* _soundBankDef;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_SOUND_BANK_H
