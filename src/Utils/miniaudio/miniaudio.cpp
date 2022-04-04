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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio_utils.h"

ma_format ma_format_from_amplitude(SparkyStudios::Audio::Amplitude::PlaybackOutputFormat format)
{
    switch (format)
    {
    default:
        return ma_format_unknown;
    case SparkyStudios::Audio::Amplitude::PlaybackOutputFormat::UInt8:
        return ma_format_u8;
    case SparkyStudios::Audio::Amplitude::PlaybackOutputFormat::Int16:
        return ma_format_s16;
    case SparkyStudios::Audio::Amplitude::PlaybackOutputFormat::Int24:
        return ma_format_s24;
    case SparkyStudios::Audio::Amplitude::PlaybackOutputFormat::Int32:
        return ma_format_s32;
    case SparkyStudios::Audio::Amplitude::PlaybackOutputFormat::Float32:
        return ma_format_f32;
    }
}
