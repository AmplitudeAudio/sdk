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

#ifndef AMPLITUDE_DEFAULTPLUGINS_H
#define AMPLITUDE_DEFAULTPLUGINS_H

#pragma region Default Codecs

// clang-format off
#include <Core/Codecs/WAV/Codec.h>
#include <Core/Codecs/OGG/Codec.h>
#include <Core/Codecs/MP3/Codec.h>
// clang-format on

#pragma endregion

#pragma region Default Drivers

#include <Core/Drivers/MiniAudio/Driver.h>

#pragma endregion

#pragma region Default Sound Processors

#include <Mixer/SoundProcessors/EffectProcessor.h>
#include <Mixer/SoundProcessors/EnvironmentProcessor.h>
#include <Mixer/SoundProcessors/ObstructionProcessor.h>
#include <Mixer/SoundProcessors/OcclusionProcessor.h>
#include <Mixer/SoundProcessors/PassThroughProcessor.h>

#pragma endregion

#pragma region Default Resamplers

#include <Mixer/Resamplers/LibsamplerateResampler.h>
#include <Mixer/Resamplers/R8BrainResampler.h>

#pragma endregion

#pragma region Default Filters

#include <Sound/Filters/BassBoostFilter.h>
#include <Sound/Filters/BiquadResonantFilter.h>
#include <Sound/Filters/DCRemovalFilter.h>
#include <Sound/Filters/DelayFilter.h>
#include <Sound/Filters/EqualizerFilter.h>
#include <Sound/Filters/FlangerFilter.h>
#include <Sound/Filters/FreeverbFilter.h>
#include <Sound/Filters/LofiFilter.h>
#include <Sound/Filters/RobotizeFilter.h>
#include <Sound/Filters/WaveShaperFilter.h>

#pragma endregion

#pragma region Default Faders

#include <Sound/Faders/ConstantFader.h>
#include <Sound/Faders/ExponentialFader.h>
#include <Sound/Faders/LinearFader.h>
#include <Sound/Faders/SCurveFader.h>

#pragma endregion

#endif // AMPLITUDE_DEFAULTPLUGINS_H
