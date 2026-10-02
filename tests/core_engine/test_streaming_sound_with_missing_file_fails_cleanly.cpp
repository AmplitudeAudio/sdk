// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Sound/Sound.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, streaming_sound_with_missing_file_fails_cleanly)
    {
    public:
        void Run() override
        {
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("test_sound_02"));
            AM_EXPECT(sound != nullptr);
            AM_EXPECT(sound->IsStream());

            const AmInt32 refCountBefore = sound->GetRefCounter()->GetCount();

            // Point the file system at a base path that holds no assets, so opening the stream fails
            // when the instance is created.
            const AmOsString basePath = _fileSystem->GetBasePath();
            _fileSystem->SetBasePath(AM_OS_STRING("this-path-does-not-exist"));

            SoundInstance* instance = sound->CreateInstance();

            _fileSystem->SetBasePath(basePath);

            AM_EXPECT(instance != nullptr);
            AM_EXPECT(instance->Valid());

            SoundImpl::DestroyInstance(instance);

            // Creating then destroying a failed instance must leave the sound's reference count unchanged.
            AM_EXPECT(sound->GetRefCounter()->GetCount() == refCountBefore);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(core_engine, streaming_sound_with_missing_file_fails_cleanly);
} // namespace SparkyStudios::Audio::Amplitude::Tests
