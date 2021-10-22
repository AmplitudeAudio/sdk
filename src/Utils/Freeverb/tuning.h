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
//
// Based on code written by Jezar at Dreampoint, June 2000 http://www.dreampoint.co.uk,
// which was placed in public domain.

#ifndef SS_AMPLITUDE_AUDIO_TUNING_H
#define SS_AMPLITUDE_AUDIO_TUNING_H

constexpr int kNumCombs = 8;
constexpr int kNumAllPasses = 4;
constexpr float kMuted = 0;
constexpr float kFixedGain = 0.015f;
constexpr float kScaleWet = 3;
constexpr float kScaleDry = 2;
constexpr float kScaleDamp = 0.4f;
constexpr float kScaleRoom = 0.28f;
constexpr float kOffsetRoom = 0.7f;
constexpr float kInitialRoom = 0.5f;
constexpr float kInitialDamp = 0.5f;
constexpr float kInitialWet = 1 / kScaleWet;
constexpr float kInitialDry = 0;
constexpr float kInitialWidth = 1;
constexpr float kInitialMode = 0;
constexpr float kFreezeMode = 0.5f;
constexpr int kStereoSpread = 23;

// These values assume 44.1KHz sample rate
// they will probably be OK for 48KHz sample rate
// but would need scaling for 96KHz (or other) sample rates.
// The values were obtained by listening tests.
constexpr int kCombTuningL1 = 1116;
constexpr int kCombTuningR1 = 1116 + kStereoSpread;
constexpr int kCombTuningL2 = 1188;
constexpr int kCombTuningR2 = 1188 + kStereoSpread;
constexpr int kCombTuningL3 = 1277;
constexpr int kCombTuningR3 = 1277 + kStereoSpread;
constexpr int kCombTuningL4 = 1356;
constexpr int kCombTuningR4 = 1356 + kStereoSpread;
constexpr int kCombTuningL5 = 1422;
constexpr int kCombTuningR5 = 1422 + kStereoSpread;
constexpr int kCombTuningL6 = 1491;
constexpr int kCombTuningR6 = 1491 + kStereoSpread;
constexpr int kCombTuningL7 = 1557;
constexpr int kCombTuningR7 = 1557 + kStereoSpread;
constexpr int kCombTuningL8 = 1617;
constexpr int kCombTuningR8 = 1617 + kStereoSpread;
constexpr int kAllPassTuningL1 = 556;
constexpr int kAllPassTuningR1 = 556 + kStereoSpread;
constexpr int kAllPassTuningL2 = 441;
constexpr int kAllPassTuningR2 = 441 + kStereoSpread;
constexpr int kAllPassTuningL3 = 341;
constexpr int kAllPassTuningR3 = 341 + kStereoSpread;
constexpr int kAllPassTuningL4 = 225;
constexpr int kAllPassTuningR4 = 225 + kStereoSpread;

#endif // SS_AMPLITUDE_AUDIO_TUNING_H
