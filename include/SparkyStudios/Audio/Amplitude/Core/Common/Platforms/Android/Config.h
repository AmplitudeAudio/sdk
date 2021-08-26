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

#ifndef SS_AMPLITUDE_AUDIO_ANDROID_CONFIG_H
#define SS_AMPLITUDE_AUDIO_ANDROID_CONFIG_H

// Enable Android Compilation
#define AM_ANDROID_VERSION

// Detect the platform CPU type
#if defined(__LP64__) || defined(_LP64)
#ifdef __aarch64__
#define AM_CPU_ARM_64
#define AM_CPU_ARM_NEON
#else
#define AM_CPU_X86_64
#endif
#else
#ifdef __arm__
#define AM_CPU_ARM
#define AM_CPU_ARM_NEON
#else
#define AM_CPU_X86
#endif
#endif

#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/UNIX/Config.h>

#endif // SS_AMPLITUDE_AUDIO_ANDROID_CONFIG_H
