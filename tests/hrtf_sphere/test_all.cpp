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

#include <HRTF/HRIRSphere.h>
#include <Math/LinearAlgebra.h>

#include "PlatformTestCase.h"
#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, hrtf_sphere, all)
    {
    public:
        void Run() override
        {
            HRIRSphereImpl sphere;
            AM_EXPECT_NOT(sphere.IsLoaded());

            auto fs = CreatePlatformFileSystem();
            fs->SetBasePath(GetPlatformAssetsBasePath());

            sphere.SetResource(AM_OS_STRING("./data/baker_a_101_lp_512.amir"));
            sphere.Load(fs);
            AM_EXPECT_NOT(sphere.IsLoaded());

            sphere.SetResource(AM_OS_STRING("./data/throw_01.wav"));
            sphere.Load(fs);
            AM_EXPECT_NOT(sphere.IsLoaded());

            sphere.SetResource(AM_OS_STRING("./data/sadie_h12.amir"));
            sphere.Load(fs);

            AM_EXPECT(sphere.IsLoaded());
            AM_EXPECT(sphere.GetPath() == AM_OS_STRING("./data/sadie_h12.amir"));

            AM_EXPECT(sphere.GetVertices().size() == sphere.GetVertexCount());
            AM_EXPECT(sphere.GetVertexCount() == 2114);

            AM_EXPECT(sphere.GetFaces().size() == sphere.GetFaceCount());
            AM_EXPECT(sphere.GetFaceCount() == 4224);

            AM_EXPECT(sphere.GetIRLength() == 256);
            AM_EXPECT(sphere.GetSampleRate() == 48000);

            HRIRSphereVertex vertex = sphere.GetVertex(0);

            AmAlignedReal32Buffer l, r;
            l.Init(256);
            r.Init(256);

            AmVector3 direction = { 0.0f, 0.0f, -1.2f };

            sphere.SetSamplingMode(eHRIRSphereSamplingMode_Bilinear);
            AM_EXPECT(sphere.GetSamplingMode() == eHRIRSphereSamplingMode_Bilinear);
            sphere.Sample(direction, l.GetBuffer(), r.GetBuffer());
            AM_EXPECT(std::memcmp(l.GetBuffer(), vertex.m_LeftIR.data(), 256 * sizeof(AmReal32)) == 0);
            AM_EXPECT(std::memcmp(r.GetBuffer(), vertex.m_RightIR.data(), 256 * sizeof(AmReal32)) == 0);

            l.Clear();
            r.Clear();

            sphere.SetSamplingMode(eHRIRSphereSamplingMode_NearestNeighbor);
            AM_EXPECT(sphere.GetSamplingMode() == eHRIRSphereSamplingMode_NearestNeighbor);
            sphere.Sample(direction, l.GetBuffer(), r.GetBuffer());
            AM_EXPECT(std::memcmp(l.GetBuffer(), vertex.m_LeftIR.data(), 256 * sizeof(AmReal32)) == 0);
            AM_EXPECT(std::memcmp(r.GetBuffer(), vertex.m_RightIR.data(), 256 * sizeof(AmReal32)) == 0);

            direction.z = -1.0f;

            sphere.Sample(direction, l.GetBuffer(), r.GetBuffer());
            AM_EXPECT(std::memcmp(l.GetBuffer(), vertex.m_LeftIR.data(), 256 * sizeof(AmReal32)) == 0);
            AM_EXPECT(std::memcmp(r.GetBuffer(), vertex.m_RightIR.data(), 256 * sizeof(AmReal32)) == 0);

            AmMatrix4 rotation = Rotation(FromAxisAngle(kVector3UnitZ, 90.0f * AM_DegToRad));
            sphere.Transform(rotation);

            HRIRSphereVertex transformedVertex = sphere.GetVertex(0);
            AM_EXPECT(transformedVertex.m_Position == Transform(rotation, { .xyz = vertex.m_Position, ._pad2 = 1.0f }).xyz);
        }
    };

    AM_REGISTER_TEST(hrtf_sphere, all);
} // namespace SparkyStudios::Audio::Amplitude::Tests
