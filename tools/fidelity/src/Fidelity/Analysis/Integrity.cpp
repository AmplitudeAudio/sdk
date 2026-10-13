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

#include <Fidelity/Analysis/Integrity.h>

#include <algorithm>
#include <cmath>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    IntegrityResult AnalyzeIntegrity(std::span<const float> x, const IntegrityOptions& options)
    {
        IntegrityResult result;
        const bool hasRegion = options.signalEnd > options.signalBegin;

        double sum = 0.0;
        std::size_t counted = 0;
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            const float v = x[i];
            switch (std::fpclassify(v))
            {
            case FP_NAN:
                ++result.nanCount;
                continue;
            case FP_INFINITE:
                ++result.infCount;
                continue;
            case FP_SUBNORMAL:
                ++result.denormalCount;
                break;
            default:
                break;
            }

            if (std::abs(v) >= options.clipThreshold)
                ++result.clippedCount;

            if (!hasRegion || (i >= options.signalBegin && i < options.signalEnd))
            {
                sum += v;
                ++counted;
            }
        }

        result.dcDbfs = counted > 0 ? DbFromAmplitude(sum / static_cast<double>(counted)) : DbFromAmplitude(0.0);

        if (!hasRegion)
            return result;

        const std::size_t end = std::min(options.signalEnd, x.size());
        std::size_t run = 0;
        for (std::size_t i = options.signalBegin; i <= end; ++i)
        {
            if (i < end && x[i] == 0.0f)
            {
                ++run;
                continue;
            }

            if (run >= options.dropoutMinRun)
            {
                ++result.dropoutCount;
                result.dropoutStarts.push_back(i - run);
                result.longestDropout = std::max(result.longestDropout, run);
            }

            run = 0;
        }

        return result;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
