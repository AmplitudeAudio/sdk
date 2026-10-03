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

#include <Fidelity/Scenarios/Playback.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    void RegisterPlaybackScenarios(ScenarioRegistry& registry)
    {
        registry.Add(MakeResamplingQualityScenario());
        registry.Add(MakeStartStopScenario());
        registry.Add(MakeTransportScenario());
        registry.Add(MakeLoopSeamScenario());
        registry.Add(MakeEndOfSoundScenario());
        registry.Add(MakeStreamingEquivalenceScenario());
        registry.Add(MakeBlockSizeIndependenceScenario());
        registry.Add(MakeVariableCallbackScenario());
    }

    void RegisterAllScenarios(ScenarioRegistry& registry)
    {
        RegisterPlaybackScenarios(registry);
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
