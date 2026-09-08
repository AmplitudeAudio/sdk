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

#include <HRTF/HRIRSphere.h>
#include <Math/LinearAlgebra.h>

#include <cstring>
#include <vector>

#include "PlatformTestCase.h"
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, hrtf_sphere, nearest_neighbor_picks_closest_vertex)
    {
    public:
        void Run() override
        {
            HRIRSphereImpl sphere;

            auto fs = CreatePlatformFileSystem();
            fs->SetBasePath(GetPlatformAssetsBasePath());

            sphere.SetResource(AM_OS_STRING("./data/sadie_h12.amir"));
            sphere.Load(fs);
            AM_EXPECT(sphere.IsLoaded());

            sphere.SetSamplingMode(eHRIRSphereSamplingMode_NearestNeighbor);

            const AmUInt32 irLength = sphere.GetIRLength();
            std::vector<AmReal32> left(irLength), right(irLength);

            auto bruteForceNearest = [&](const AmVector3& dir) -> const HRIRSphereVertex*
            {
                const HRIRSphereVertex* best = nullptr;
                AmReal32 bestDot = -2.0f;

                for (const auto& vertex : sphere.GetVertices())
                {
                    const AmReal32 d = Dot(dir, Normalize(vertex.m_Position));
                    if (d > bestDot)
                    {
                        bestDot = d;
                        best = &vertex;
                    }
                }

                return best;
            };

            // Deterministic directions: axis extremes + a golden-spiral spread over the sphere.
            std::vector<AmVector3> directions = {
                { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
                { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f },
            };

            constexpr AmUInt32 kExtra = 200;
            constexpr AmReal32 kGoldenAngle = 2.39996322972865332f;
            for (AmUInt32 i = 0; i < kExtra; ++i)
            {
                const AmReal32 z = 1.0f - 2.0f * (static_cast<AmReal32>(i) + 0.5f) / static_cast<AmReal32>(kExtra);
                const AmReal32 r = std::sqrt(1.0f - z * z);
                const AmReal32 theta = kGoldenAngle * static_cast<AmReal32>(i);
                directions.push_back({ r * std::cos(theta), r * std::sin(theta), z });
            }

            for (const auto& dir : directions)
            {
                sphere.Sample(dir, left.data(), right.data());

                const auto* expected = bruteForceNearest(dir);
                AM_EXPECT(expected != nullptr);
                AM_EXPECT(std::memcmp(left.data(), expected->m_LeftIR.data(), irLength * sizeof(AmReal32)) == 0);
                AM_EXPECT(std::memcmp(right.data(), expected->m_RightIR.data(), irLength * sizeof(AmReal32)) == 0);
            }
        }
    };

    AM_REGISTER_TEST(hrtf_sphere, nearest_neighbor_picks_closest_vertex);
} // namespace SparkyStudios::Audio::Amplitude::Tests
