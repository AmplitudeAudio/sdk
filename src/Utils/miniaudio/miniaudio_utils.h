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

#ifndef SS_AMPLITUDE_MINIAUDIO_UTILS_H
#define SS_AMPLITUDE_MINIAUDIO_UTILS_H

#include <SparkyStudios/Audio/Amplitude/Core/Device.h>

#include "miniaudio.h"

ma_format ma_format_from_amplitude(SparkyStudios::Audio::Amplitude::PlaybackOutputFormat format);

#endif /* SS_AMPLITUDE_MINIAUDIO_UTILS_H */
