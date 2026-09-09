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
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, hrtf_sphere, all)
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

            // Bilinear output is rotation-covariant: rotating the whole sphere by R must give
            // the same result at R*dir as the unrotated sphere gives at dir. Capture reference
            // samples before the transform to verify the face-BSP is rebuilt, not just the KD-tree.
            const AmVector3 bilinearDirs[] = {
                { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f },  { 0.0f, -1.0f, 0.0f }, { 0.577f, 0.577f, 0.577f },
            };
            constexpr AmUInt32 kNumBilinearDirs = sizeof(bilinearDirs) / sizeof(bilinearDirs[0]);

            AmAlignedReal32Buffer bilinearRefL[kNumBilinearDirs];
            AmAlignedReal32Buffer bilinearRefR[kNumBilinearDirs];
            for (AmUInt32 i = 0; i < kNumBilinearDirs; ++i)
            {
                bilinearRefL[i].Init(256);
                bilinearRefR[i].Init(256);
                l.Clear();
                r.Clear();
                sphere.Sample(bilinearDirs[i], l.GetBuffer(), r.GetBuffer());
                std::memcpy(bilinearRefL[i].GetBuffer(), l.GetBuffer(), 256 * sizeof(AmReal32));
                std::memcpy(bilinearRefR[i].GetBuffer(), r.GetBuffer(), 256 * sizeof(AmReal32));
            }

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

            // Post-transform bilinear covariance: sampling at R*dir must equal the pre-transform
            // sample at dir (the sphere rotated rigidly, IRs riding along with their vertices).
            // Without the face-BSP rebuild this fails — the tree would still locate pre-rotation
            // faces and bilinear would interpolate the wrong triangle.
            {
                sphere.SetSamplingMode(eHRIRSphereSamplingMode_Bilinear);

                const AmSize irLen = sphere.GetIRLength();
                AmReal32 maxAbsDiff = 0.0f;

                for (AmUInt32 i = 0; i < kNumBilinearDirs; ++i)
                {
                    // Rotate dir by Rz(90 deg): (x, y, z) -> (-y, x, z)
                    const AmVector3& d = bilinearDirs[i];
                    const AmVector3 rotatedDir = { -d.y, d.x, d.z };

                    l.Clear();
                    r.Clear();
                    sphere.Sample(rotatedDir, l.GetBuffer(), r.GetBuffer());

                    for (AmSize s = 0; s < irLen; ++s)
                    {
                        maxAbsDiff = AM_MAX(maxAbsDiff, std::fabs(l.GetBuffer()[s] - bilinearRefL[i].GetBuffer()[s]));
                        maxAbsDiff = AM_MAX(maxAbsDiff, std::fabs(r.GetBuffer()[s] - bilinearRefR[i].GetBuffer()[s]));
                    }
                }

                // 90-degree rotation about Z has cos(pi/2) ~= -4.4e-8 float error, so the rotated
                // vertices are not bit-exact; allow a small tolerance.
                AM_EXPECT(maxAbsDiff < 1e-4f);
            }

            // Post-transform: verify NearestNeighbor sampling matches brute-force over transformed vertices
            {
                const auto& verts = sphere.GetVertices();
                sphere.SetSamplingMode(eHRIRSphereSamplingMode_NearestNeighbor);

                auto bruteForceNearest = [&verts](const AmVector3& dir) -> AmUInt32 {
                    const AmVector3 nDir = Normalize(dir);
                    AmReal32 bestDot = -2.0f;
                    AmUInt32 bestIdx = 0;
                    for (AmUInt32 i = 0; i < static_cast<AmUInt32>(verts.size()); ++i)
                    {
                        const AmReal32 d = Dot(nDir, Normalize(verts[i].m_Position));
                        if (d > bestDot)
                        {
                            bestDot = d;
                            bestIdx = i;
                        }
                    }
                    return bestIdx;
                };

                const AmVector3 directions[] = {
                    { 1.0f, 0.0f, 0.0f },
                    { -1.0f, 0.0f, 0.0f },
                    { 0.0f, 1.0f, 0.0f },
                    { 0.0f, -1.0f, 0.0f },
                    { 0.0f, 0.0f, 1.0f },
                    { 0.0f, 0.0f, -1.0f },
                    { 0.577f, 0.577f, 0.577f },
                    { -0.577f, 0.577f, -0.577f },
                };

                const AmSize irLen = sphere.GetIRLength();

                for (const auto& dir : directions)
                {
                    const AmUInt32 expected = bruteForceNearest(dir);
                    const auto& expectedVertex = verts[expected];

                    l.Clear();
                    r.Clear();
                    sphere.Sample(dir, l.GetBuffer(), r.GetBuffer());

                    AM_EXPECT(std::memcmp(l.GetBuffer(), expectedVertex.m_LeftIR.data(), irLen * sizeof(AmReal32)) == 0);
                    AM_EXPECT(std::memcmp(r.GetBuffer(), expectedVertex.m_RightIR.data(), irLen * sizeof(AmReal32)) == 0);
                }
            }
        }
    };

    AM_REGISTER_TEST(hrtf_sphere, all);
} // namespace SparkyStudios::Audio::Amplitude::Tests
