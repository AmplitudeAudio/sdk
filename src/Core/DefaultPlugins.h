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

#pragma region Default Sound Processors

#include <Mixer/SoundProcessors/ClipProcessor.h>
#include <Mixer/SoundProcessors/EffectProcessor.h>
#include <Mixer/SoundProcessors/EnvironmentProcessor.h>
#include <Mixer/SoundProcessors/ObstructionProcessor.h>
#include <Mixer/SoundProcessors/OcclusionProcessor.h>
#include <Mixer/SoundProcessors/PassThroughProcessor.h>
#include <Mixer/SoundProcessors/SilenceProcessor.h>

#pragma endregion

#pragma region Default Resamplers

#include <Mixer/Resamplers/LibsamplerateResampler.h>
#include <Mixer/Resamplers/R8BrainResampler.h>

#pragma endregion

#pragma region Default Faders

#include <Sound/Faders/ConstantFader.h>
#include <Sound/Faders/EaseFader.h>
#include <Sound/Faders/EaseInFader.h>
#include <Sound/Faders/EaseInOutFader.h>
#include <Sound/Faders/EaseOutFader.h>
#include <Sound/Faders/ExponentialFader.h>
#include <Sound/Faders/LinearFader.h>
#include <Sound/Faders/SCurveFader.h>

#pragma endregion

#endif // AMPLITUDE_DEFAULTPLUGINS_H
