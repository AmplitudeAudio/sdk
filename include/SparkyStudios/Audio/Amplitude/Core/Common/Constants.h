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

#ifndef SS_AMPLITUDE_AUDIO_CONSTANTS_H
#define SS_AMPLITUDE_AUDIO_CONSTANTS_H

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Invalid Amplitude object ID.
     */
    const AmUInt64 kAmInvalidObjectId = 0;

    /**
     * @brief Specifies the value of the "master" bus ID.
     */
    const AmBusID kAmMasterBusId = 1;
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_CONSTANTS_H
