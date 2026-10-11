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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Ambisonics/AmbisonicOrientationProcessor.h>
#include <Ambisonics/BFormat.h>
#include <Math/LinearAlgebra.h>
#include <SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, ambisonics_orientation_processor, room_orientation_composition)
    {
    public:
        void Run() override
        {
            // Room yawed +90 degrees about Z (engine frame): its front wall faces engine -X, its left side.
            CheckFrontWall(Orientation(AM_PI32 / 2.0f, 0.0f, 0.0f), 0.0f, -1.0f, 0.0f);

            // Room pitched up 90 degrees: its front wall faces engine +Z.
            CheckFrontWall(Orientation({ 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }), 0.0f, 0.0f, 1.0f);
        }

    private:
        // Rotates the room's front direction (+X in the field: X forward, Y right, Z up) with the composition used by
        // ReflectionsNode, and checks where the rotated room's front wall faces in the world.
        void CheckFrontWall(const Orientation& room, AmReal32 x, AmReal32 y, AmReal32 z)
        {
            constexpr AmUInt32 kSamples = 8;

            BFormat field;
            AM_EXPECT(field.Configure(1, true, kSamples));
            field.Reset();
            for (AmUInt32 i = 0; i < kSamples; ++i)
                field.GetBufferChannel(eBFormatChannel_X)[i] = 1.0f;

            using Axis = CartesianCoordinateSystem::Axis;
            const CartesianCoordinateSystem::Converter engineToField(
                CartesianCoordinateSystem::Default(), CartesianCoordinateSystem(Axis::PositiveY, Axis::PositiveX, Axis::PositiveZ));

            AmbisonicOrientationProcessor processor;
            AM_EXPECT(processor.Configure(1, true));
            processor.SetOrientation(Orientation(engineToField.Forward(Inverse(room.GetQuaternion()))));
            processor.Process(&field, kSamples);

            for (AmUInt32 i = 0; i < kSamples; ++i)
            {
                AM_EXPECT(std::abs(field.GetBufferChannel(eBFormatChannel_X)[i] - x) < 1e-4f);
                AM_EXPECT(std::abs(field.GetBufferChannel(eBFormatChannel_Y)[i] - y) < 1e-4f);
                AM_EXPECT(std::abs(field.GetBufferChannel(eBFormatChannel_Z)[i] - z) < 1e-4f);
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_orientation_processor, room_orientation_composition);
} // namespace SparkyStudios::Audio::Amplitude::Tests
