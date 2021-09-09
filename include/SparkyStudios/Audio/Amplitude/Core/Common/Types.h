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

#ifndef SS_AMPLITUDE_AUDIO_TYPES_H
#define SS_AMPLITUDE_AUDIO_TYPES_H

#if defined(AM_WINDOWS_VERSION)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Windows/Types.h>
#elif defined(AM_ANDROID_VERSION)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Android/Types.h>
#elif defined(AM_LINUX_VERSION)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Linux/Types.h>
#endif

namespace SparkyStudios::Audio::Amplitude
{
    typedef AmUInt64            AmObjectID;

    typedef AmObjectID          AmBankID;
    typedef AmObjectID          AmBusID;
    typedef AmObjectID          AmCollectionID;
    typedef AmObjectID          AmSoundID;
    typedef AmObjectID          AmEntityID;
    typedef AmObjectID          AmListenerID;
    typedef AmObjectID          AmChannelID;
    typedef AmObjectID          AmEventID;
    typedef AmObjectID          AmAttenuationID;

    typedef AmUInt8             AmResult;
    typedef AmUInt64            AmHandle;
    typedef AmReal64            AmTime;
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_TYPES_H
