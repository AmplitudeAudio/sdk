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

#include <stdexcept>

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // An action that throws must become a render error, and the engine must still be torn down so the next render
    // starts from a fresh engine (otherwise it silently reuses the previous one).
    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, render_survives_throwing_action)
    {
    public:
        void Run() override
        {
            const RenderSettings settings = IsolatedSettings(GridPoint{}, 9600);

            std::vector<TimedAction> throwing = {
                { 800, "boom",
                  []()
                  {
                      throw std::runtime_error("action failed");
                  } },
            };

            RenderOutcome failed;
            bool threw = false;
            try
            {
                failed = Render(kDefaultAssetsPath, settings, std::move(throwing));
            } catch (...)
            {
                threw = true;
            }

            AM_EXPECT(!threw);
            AM_EXPECT(failed.error.find("action failed") != std::string::npos);
            AM_EXPECT(Engine::GetInstance() == nullptr || !Engine::GetInstance()->IsInitialized());

            const RenderOutcome next = Render(kDefaultAssetsPath, settings, PlayOnly(SoundName(*FindStimulus("sine_997_48000"), false))());
            AM_EXPECT(next.error.empty());
            AM_EXPECT(!next.capture.channels.empty() && Peak(ToSignal(next.capture.channels[0])) > 0.1);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, render_survives_throwing_action);
} // namespace SparkyStudios::Audio::Amplitude::Tests
