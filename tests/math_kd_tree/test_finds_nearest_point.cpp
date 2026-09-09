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

#include <Math/KDTree.h>
#include <Math/LinearAlgebra.h>

#include <algorithm>
#include <cstdint>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, math_kd_tree, finds_nearest_point)
    {
    public:
        void Run() override
        {
            // Deterministic LCG so the test is reproducible.
            AmUInt64 seed = 12345;
            auto nextRand = [&seed]()
            {
                seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
                return static_cast<AmReal32>((seed >> 33) & 0xFFFFFFFFULL) / static_cast<AmReal32>(0xFFFFFFFFULL);
            };

            auto bruteForceNearest = [](const std::vector<AmVector3>& pts, const AmVector3& q) -> AmSize
            {
                AmSize bestIndex = KDTree::kInvalidIndex;
                AmReal32 bestDist = std::numeric_limits<AmReal32>::max();

                for (AmSize i = 0; i < pts.size(); ++i)
                {
                    const AmReal32 d = SquaredLength(Sub(q, pts[i]));
                    if (d < bestDist || (d == bestDist && i < bestIndex))
                    {
                        bestDist = d;
                        bestIndex = i;
                    }
                }

                return bestIndex;
            };

            // Empty tree.
            {
                KDTree tree;
                AM_EXPECT_EQ(tree.GetPointCount(), 0u);
                AM_EXPECT_EQ(tree.FindNearest({ 1.0f, 2.0f, 3.0f }), KDTree::kInvalidIndex);
            }

            // Single point.
            {
                KDTree tree;
                tree.Build({ { 1.0f, 2.0f, 3.0f } });
                AM_EXPECT_EQ(tree.GetPointCount(), 1u);
                AM_EXPECT_EQ(tree.FindNearest({ 1.0f, 2.0f, 3.0f }), 0u);
                AM_EXPECT_EQ(tree.FindNearest({ 100.0f, -5.0f, 0.5f }), 0u);
            }

            // Tie-break determinism: two identical points; the lower index wins.
            {
                KDTree tree;
                tree.Build({ { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
                AM_EXPECT_EQ(tree.FindNearest({ 0.0f, 0.0f, 0.0f }), 0u);
                AM_EXPECT_EQ(tree.FindNearest({ 1.0f, 1.0f, 1.0f }), 1u);
            }

            // All-identical points must not hang or return an out-of-range index.
            {
                KDTree tree;
                const std::vector<AmVector3> identical(64, AmVector3{ 2.0f, -1.0f, 0.5f });
                tree.Build(identical);
                AM_EXPECT_EQ(tree.FindNearest({ 2.0f, -1.0f, 0.5f }), 0u);
                AM_EXPECT_EQ(tree.FindNearest({ 0.0f, 0.0f, 0.0f }), 0u);
            }

            // Random cloud matches brute force exactly (lowest-index tie-break).
            {
                constexpr AmSize kN = 1000;
                constexpr AmSize kM = 500;

                std::vector<AmVector3> points;
                points.reserve(kN);
                for (AmSize i = 0; i < kN; ++i)
                    points.push_back({ nextRand() * 20.0f - 10.0f, nextRand() * 20.0f - 10.0f, nextRand() * 20.0f - 10.0f });

                KDTree tree;
                tree.Build(points);
                AM_EXPECT_EQ(tree.GetPointCount(), kN);

                for (AmSize i = 0; i < kM; ++i)
                {
                    // Mix exact points and random probes so both descent paths are exercised.
                    const AmVector3 q = (i % 3 == 0)
                        ? points[static_cast<AmSize>(nextRand() * kN) % kN]
                        : AmVector3{ nextRand() * 20.0f - 10.0f, nextRand() * 20.0f - 10.0f, nextRand() * 20.0f - 10.0f };
                    const AmSize expected = bruteForceNearest(points, q);
                    const AmSize actual = tree.FindNearest(q);
                    AM_EXPECT_EQ(actual, expected);
                }
            }
        }
    };

    AM_REGISTER_TEST(math_kd_tree, finds_nearest_point);
} // namespace SparkyStudios::Audio::Amplitude::Tests
